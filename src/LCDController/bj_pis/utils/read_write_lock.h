
//reference  https://blog.csdn.net/qq_35865125/article/details/110149082

#pragma once
#include <iostream>
#include <thread>
#include <mutex>
#include <list>
#include <cstdlib>
#include <vector>
#include <condition_variable>
using namespace std;

class ReadWriteLock
{
public:
    ReadWriteLock(): mWaitReadThreadNum(0), mReadingThreadNum(0),
        mWaitWriteTrheadNum(0), mWritingThreadNum(0)
    {
    }

    ~ReadWriteLock() {};

    void ReadLock();
    void WriteLock();
    void ReadUnlock();
    void WriteUnlock();

private:
    int mWaitReadThreadNum, mReadingThreadNum;

    int mWaitWriteTrheadNum, mWritingThreadNum;

    mutex mMyMutex;
    condition_variable mReadThreadCV;//用于“读线程”的等待和唤醒。
    condition_variable mWriteThreadCV;//用于"写线程"的等待和唤醒
};