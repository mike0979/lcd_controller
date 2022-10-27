#ifndef LOOPER_H_
#define LOOPER_H_

#include "MessageQueue.h"
#include "Runnable.h"
#include "SyncerPipe.h"

class FdRunnable : public Runnable {
public:
	void setFdEvent(int fd, int event);

protected:
	int mFd;
	int mEvent;
};

class Looper {
public:
	static const int FD_EVENT_INPUT;
	static const int FD_EVENT_OUTPUT;
	static const int FD_EVENT_ERROR;
	static const int FD_EVENT_HANGUP;

	static Looper *CreateLooper(bool main = false);
	~Looper();

	void wake();
	void loop();
	void quit();

	bool isQuit();
	bool isMain();

	bool addFd(int fd, int event, FdRunnable *runnable);
	bool removeFd(int fd);

	MessageQueue *getMessageQueue();
	static Looper *MainLooper();
	static Looper *CurrLooper();

private:
	Looper();

	class FdEvent {
	public:
		FdEvent(int fd, int event, FdRunnable *runnable);
		~FdEvent();

	private:
		int mFd;
		int mEvent;
		FdRunnable *mRunnable;

		friend class Looper;
	};

	int loopInner(int &timeout);

	bool isFdAdded(int fd);
	FdEvent *getFdEvent(int fd);
private:
	static const char *TAG;

	static Looper *mMainLooper;
	static __thread Looper *mLooperTLS;

	bool mQuit;
	MessageQueue *mMessageQueue;

	int mEpollFd;
	std::list<FdEvent *> mFds;
	Mutex mFdsMutex;

	SyncerPipe *mMsgQueueSyncer;

	static const int EPOLL_SIZE_HINT;
	static const int EPOLL_MAX_EVENTS;
};

#endif /* LOOPER_H_ */
