#include "Thread.h"

//#include "Log.h"

#include <errno.h>

Thread::Thread()
{
	mRunnable = NULL;
	mAutoDel = false;

	mTid = 0;
	mIsDaemon = false;

}

Thread::Thread(std::string ThreadName)
{
	mRunnable = NULL;
	mAutoDel = false;

	mTid = 0;
	mTName = ThreadName;
	mIsDaemon = false;
}

Thread::Thread(Runnable *runnable)
{
	mRunnable = runnable;
	mAutoDel = false;

	mTid = 0;
	mIsDaemon = false;
}

Thread::Thread(Runnable *runnable, std::string ThreadName)
{
	mRunnable = runnable;
	mAutoDel = false;

	mTid = 0;
	mTName = ThreadName;
	mIsDaemon = false;
}

Thread::~Thread()
{
	cancel();

	if (mRunnable != NULL && mAutoDel == true) {
		delete mRunnable;
	}
}

struct ThreadProcData
{
	Thread *mThread;
	Runnable *mRunnable;
};

void Thread::start()
{
	if (mTid != 0) {
		//LogI("%s %d --- Thread is already started, tid = 0x%X.\n", __FILE__, __LINE__, (unsigned) mTid);
		return;
	}

	struct ThreadProcData *tpData = new struct ThreadProcData;
	tpData->mThread = this;
	tpData->mRunnable = mRunnable;

	int cpret = pthread_create(&mTid, NULL, threadProc, tpData);
	if (cpret == 0) {
		pthread_detach(mTid);
	} else {
		mTid = 0;

		delete tpData;
		//LogE("%s %d --- pThread_create failed : %d\n", __FILE__, __LINE__, cpret);
	}
}

void Thread::cancel()
{
	if (mTid != 0) {
		pthread_cancel(mTid);

		mTid = 0;
	}
}

void Thread::join()
{
	if (mTid != 0) {
		pthread_join(mTid, NULL);
	}
}

pthread_t Thread::getThreadId()
{
	return mTid;
}

void Thread::setName(std::string name)
{
	mTName = name;
}

void Thread::setDaemon(bool isDaemon)
{
	mIsDaemon = isDaemon;
}

void Thread::setRunnable(Runnable *runnable, bool autoDel)
{
	if (mTid == 0) {
		if (mRunnable != NULL && mAutoDel == true) {
			delete mRunnable;
		}

		mRunnable = runnable;
		mAutoDel = autoDel;
	}
	else {
		cancel();

		if (mRunnable != NULL && mAutoDel == true) {
			delete mRunnable;
		}

		mRunnable = runnable;
		mAutoDel = autoDel;

		start();
	}
}

pthread_t Thread::currentThread()
{
	return pthread_self();
}

void Thread::sleep(long long ms) {
	struct timespec t;
	t.tv_sec = ms / 1000;
	t.tv_nsec = (ms % 1000) * 1000000;

	int ret = nanosleep(&t, NULL);
	if (ret == -1 && errno == EINTR) {
		//LogW("%s %d --- Thread::sleep() is interrupted by a signal handler\n", __FILE__, __LINE__);
	}
}

void Thread::run()
{
}

void* Thread::threadProc(void *arg)
{
	pthread_cleanup_push(threadCleanup, arg);
	struct ThreadProcData *tpData = (struct ThreadProcData *) arg;

	if (tpData->mRunnable != NULL) {
		tpData->mRunnable->runnable();
	}
	else if (tpData->mThread != NULL) {
		tpData->mThread->run();
	}

	pthread_cleanup_pop(1);

	return NULL;
}

void Thread::threadCleanup(void *arg) {
	struct ThreadProcData *tpData = (struct ThreadProcData *) arg;

	delete tpData;
}

const char *Thread::TAG = "Thread";
