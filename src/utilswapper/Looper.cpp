#include "Looper.h"
#include "Handler.h"

#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include "SystemClock.h"

enum LooperPollResult {
	LOOPER_POLL_WAKE = 0,
	LOOPER_POLL_FDS,
	LOOPER_POLL_TIMEOUT,
	LOOPER_POLL_INTERRUPT,
	LOOPER_POLL_ERROR,
};

void FdRunnable::setFdEvent(int fd, int event)
{
	mFd = fd;
	mEvent = event;
}

Looper *Looper::CreateLooper(bool main)
{
	Looper *looper = NULL;

	if (main == true) {
		if (mMainLooper == NULL) {
			mMainLooper = new Looper();

			looper = mMainLooper;
		}
		else {
			looper = NULL;

			printf("MainLooper already created !\n");
		}
	}
	else {
		looper = new Looper();
	}

	if (looper != NULL) {
		mLooperTLS = looper;
	}

	return looper;
}

Looper::Looper()
{
	mQuit = false;
	mMessageQueue = new MessageQueue(this);

	mMsgQueueSyncer = new SyncerPipe();

	// Allocate the epoll instance and register the wake pipe.
	mEpollFd = epoll_create(EPOLL_SIZE_HINT);
	if (mEpollFd < 0) {
		printf("Could not create epoll instance.\n");
	}

    struct epoll_event eventItem;
    memset(&eventItem, 0, sizeof(epoll_event));
    eventItem.events = EPOLLIN;
    eventItem.data.fd = mMsgQueueSyncer->getSyncerFd();

    int ctlret = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, eventItem.data.fd, &eventItem);
    if (ctlret != 0) {
    	printf("Could not add wake read pipe to epoll instance.\n");
    }
}

Looper::~Looper()
{
	delete mMessageQueue;
	delete mMsgQueueSyncer;

	for (std::list<FdEvent *>::iterator i = mFds.begin(); i != mFds.end(); i++) {
		delete (*i);
	}
}

void Looper::wake()
{
	mMsgQueueSyncer->sendSyncerMsg("W");
}

void Looper::loop()
{
	int timeout = -1;

	while (true) {
		loopInner(timeout);
	}
}

void Looper::quit()
{
	mQuit = true;

	wake();
}

bool Looper::isQuit()
{
	return mQuit;
}

bool Looper::isMain()
{
	return this == mMainLooper;
}

bool Looper::addFd(int fd, int event, FdRunnable *runnable)
{
	if (fd < 0) {
		return false;
	}

	int epollEvents = 0;
	if (event & FD_EVENT_INPUT) {
		epollEvents |= EPOLLIN;
	}
	if (event & FD_EVENT_OUTPUT) {
		epollEvents |= EPOLLOUT;
	}

     struct epoll_event eventItem;
     memset(& eventItem, 0, sizeof(epoll_event));
     eventItem.events = epollEvents;
     eventItem.data.fd = fd;

     bool add = false;
     if (isFdAdded(fd)) {
    	 int ctlret = epoll_ctl(mEpollFd, EPOLL_CTL_MOD, fd, &eventItem);
    	 if (ctlret < 0) {
    		 printf("Error modifying epoll events for fd %d. %s\n", fd, strerror(errno));
    	 }
    	 else {
    		 add = true;
    	 }
     }
     else {
    	 int ctlret = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &eventItem);
    	 if (ctlret < 0) {
    		 printf("Error adding epoll events for fd %d. %s\n", fd, strerror(errno));
    	 }
    	 else {
    		 synchronized(mFdsMutex) {
    			 mFds.push_back(new FdEvent(fd, event, runnable));
    		 }

    		 add = true;
    	 }
     }

     return add;
}

bool Looper::removeFd(int fd)
{
	if (fd < 0) {
		return false;
	}

	bool remove = false;

	if (isFdAdded(fd)) {
		int ctlret = epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, NULL);
		if (ctlret < 0) {
			printf("Error removing epoll events for fd %d. %s\n", fd, strerror(errno));
		}
		else {
			synchronized(mFdsMutex) {
				for (std::list<FdEvent *>::iterator i = mFds.begin(); i != mFds.end(); i++) {
					if ((*i)->mFd == fd) {
						delete (*i);
						mFds.erase(i);

						break;
					}
				}
			}

			remove = true;
		}
	}
	else {
		printf("Removing a fd NOT added.\n");
		remove = true;
	}

	return remove;
}

MessageQueue *Looper::getMessageQueue()
{
	return mMessageQueue;
}

Looper *Looper::MainLooper()
{
	return mMainLooper;
}

Looper *Looper::CurrLooper()
{
	return mLooperTLS;
}

int Looper::loopInner(int &timeout)
{
	int result = LOOPER_POLL_WAKE;

	struct epoll_event eventItems[EPOLL_MAX_EVENTS];
	int evcnt = epoll_wait(mEpollFd, eventItems, EPOLL_MAX_EVENTS, timeout);
	uint64_t now = SystemClock::UptimeMillis();

	if (evcnt < 0) {
		if (errno == EINTR) {
			result = LOOPER_POLL_INTERRUPT;
		}
		else {
			printf("Poll failed with an unexpected error, errno=%d", errno);
			result = LOOPER_POLL_ERROR;
		}
	}
	else if (evcnt == 0) {
		result = LOOPER_POLL_TIMEOUT;
	}
	else {
		for (int i = 0; i < evcnt; i++) {
			int fd = eventItems[i].data.fd;
			int ev = eventItems[i].events;

			if (fd == mMsgQueueSyncer->getSyncerFd()) {
				if (ev & EPOLLIN) {
					char awoken;
					mMsgQueueSyncer->recvSyncerMsg(&awoken, 1);
				}
				else {
					printf("Ignoring unexpected epoll events 0x%x on wake read pipe.", ev);
				}
			}
			else {
				FdEvent *fdev = getFdEvent(fd);
				if (fdev != NULL) {
					int evflag = 0;
					if (ev & EPOLLIN) {
						evflag |= FD_EVENT_INPUT;
					}
					if (ev & EPOLLOUT) {
						evflag |= FD_EVENT_OUTPUT;
					}
					if (ev & EPOLLERR) {
						evflag |= FD_EVENT_ERROR;
					}
					if (ev & EPOLLHUP) {
						evflag |= FD_EVENT_HANGUP;
					}

					fdev->mRunnable->setFdEvent(fd, evflag);
					fdev->mRunnable->runnable();
				}
				else {
					printf("Ignoring unexpected epoll events 0x%x on fd %d that is no longer registered.", ev, fd);
				}
			}
		}

		result = LOOPER_POLL_FDS;
	}

	do {
		Message *msg = mMessageQueue->next(timeout);
		if (msg != NULL) {
			msg->mHandler->handle(msg);
		}
	} while (timeout == 0);

	return result;
}

bool Looper::isFdAdded(int fd)
{
	synchronized(mFdsMutex) {
		for (std::list<FdEvent *>::iterator i = mFds.begin(); i != mFds.end(); i++) {
			if ((*i)->mFd == fd) {
				return true;
			}
		}
	}

	return false;
}

Looper::FdEvent *Looper::getFdEvent(int fd)
{
	FdEvent *ev = NULL;

	synchronized(mFdsMutex) {
		for (std::list<FdEvent *>::iterator i = mFds.begin(); i != mFds.end(); i++) {
			if ((*i)->mFd == fd) {
				ev = (*i);
				break;
			}
		}
	}

	return ev;
}

Looper::FdEvent::FdEvent(int fd, int event, FdRunnable *runnable) : mFd(fd), mEvent(event), mRunnable(runnable)
{

}

Looper::FdEvent::~FdEvent()
{
//	delete mRunnable;
}

const char *Looper::TAG = "Looper";
Looper *Looper::mMainLooper = NULL;
__thread Looper *Looper::mLooperTLS = NULL;

const int Looper::FD_EVENT_INPUT = 1 << 0;
const int Looper::FD_EVENT_OUTPUT = 1 << 1;
const int Looper::FD_EVENT_ERROR = 1 << 2;
const int Looper::FD_EVENT_HANGUP = 1 << 3;

const int Looper::EPOLL_SIZE_HINT = 8;
const int Looper::EPOLL_MAX_EVENTS = 16;
