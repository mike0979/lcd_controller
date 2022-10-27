/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/

#include "Mutex.h"

//Mutex::Mutex()
//{
//    pthread_mutex_init(&mMutex, NULL);
//}

Mutex::Mutex(bool recursive)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    if (recursive) {
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    }
    pthread_mutex_init(&mMutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&mMutex);
}

int Mutex::lock()
{
    return -pthread_mutex_lock(&mMutex);
}

int Mutex::unlock()
{
    return pthread_mutex_unlock(&mMutex);
}

int Mutex::tryLock()
{
    return pthread_mutex_trylock(&mMutex);
}

Mutex::Autolock::Autolock(Mutex& mutex) : mLock(mutex)
{
	mLock.lock();
}

Mutex::Autolock::~Autolock()
{
	mLock.unlock();
}

RecursiveMutex::RecursiveMutex() : Mutex(true)
{

}

RecursiveMutex::~RecursiveMutex()
{

}


