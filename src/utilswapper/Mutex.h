/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef SRC_MUTEX_H_
#define SRC_MUTEX_H_


#include <pthread.h>
class Mutex
{
public:
//    Mutex();
    Mutex(bool recursive = true);
    virtual ~Mutex();

    int lock();
    int unlock();
    int tryLock();

    class Autolock
    {
    public:
        Autolock(Mutex& mutex);
        ~Autolock();

        inline operator bool () const {
            return true;
        }

    private:
        Mutex& mLock;
    };

private:
    pthread_mutex_t mMutex;
};

class RecursiveMutex : public Mutex
{
public:
    RecursiveMutex();
    ~RecursiveMutex();
};

#define synchronized(obj) if (Mutex::Autolock __AL__ = obj)

#endif /* SRC_MUTEX_H_ */
