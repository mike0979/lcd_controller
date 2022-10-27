#include "read_write_lock.h"

void ReadWriteLock::ReadLock()
{
    unique_lock<mutex> uniLock(mMyMutex);
    if (mWritingThreadNum || mWaitWriteTrheadNum)   //写优先，只要有线程在等待写，则不能让读线程得到机会。
    {
        ++mWaitReadThreadNum;
        while (mWritingThreadNum || mWaitWriteTrheadNum)
        {
            mReadThreadCV.wait(uniLock);
        }
        --mWaitReadThreadNum;
    }
    ++mReadingThreadNum;
}

void ReadWriteLock::WriteLock()
{
    unique_lock<mutex> uniLock(mMyMutex);
    if (mWritingThreadNum || mReadingThreadNum)
    {
        ++mWaitWriteTrheadNum;
        while (mWritingThreadNum || mReadingThreadNum)
        {
            mWriteThreadCV.wait(uniLock);
        }
        --mWaitWriteTrheadNum;
    }
    ++mWritingThreadNum;
}

void ReadWriteLock::ReadUnlock()
{
	if(mReadingThreadNum==0)return;
    unique_lock<mutex> uniLock(mMyMutex);
    --mReadingThreadNum;
    if (mWaitWriteTrheadNum)  //有写线程在等待的话，直接尝试唤醒一个写线程，即使还有其他线程在读。写优先！
    {
        mWriteThreadCV.notify_one();
    }
}

void ReadWriteLock::WriteUnlock()
{
	if(mWritingThreadNum==0)return;
    unique_lock<mutex> uniLock(mMyMutex);
    --mWritingThreadNum;
    if (mWaitWriteTrheadNum)  //写优先
    {
        mWriteThreadCV.notify_one();
    }
    else
    {
        mReadThreadCV.notify_all();//通知所有被阻塞的read线程
    }
}