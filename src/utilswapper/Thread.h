#ifndef THREAD_H_
#define THREAD_H_

#include <string>
#include <pthread.h>

#include "Runnable.h"

class Thread
{
public:
	Thread();
	Thread(std::string tName);

	Thread(Runnable *runnable);
	Thread(Runnable *runnable, std::string tName);

    virtual ~Thread();

    void start();
    void cancel();
    void join();

    pthread_t getThreadId();

    void setName(std::string tName);
    void setDaemon(bool isDaemon);

    void setRunnable(Runnable *runnable, bool autoDel = true);

    static pthread_t currentThread();
    static void sleep(long long ms);

private:
    static const char *TAG;

    Runnable *mRunnable;
    bool mAutoDel;

    pthread_t mTid;
    std::string mTName;
    bool mIsDaemon;

    virtual void run();

    static void* threadProc(void *arg);
	static void threadCleanup(void *arg);
};
#endif /* THREAD_H_ */
