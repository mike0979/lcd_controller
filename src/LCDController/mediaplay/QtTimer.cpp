/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#include "QtTimer.h"

#include "SystemClock.h"
#include "Log.h"

QtTimer::QtTimer(QObject *parent)
{
	mTimer = new QTimer(this);

	mRunning = false;
	mStarted = false;
	mRemains = 0;

	mGoStamp = 0;

	mValidDuration = true;

	mTimer->setSingleShot(true);
	connect(mTimer, SIGNAL(timeout()), this, SLOT(qtimerout()));
}

QtTimer::~QtTimer()
{
	if(mTimer != NULL)
	{
		if(mTimer->isActive())
			mTimer->stop();

		delete mTimer;
		mTimer = NULL;
	}
}

void QtTimer::start(int msec)
{
	if (msec > 0) {
		mRunning = true;
		mStarted = true;
		mRemains = msec;

		mGoStamp = SystemClock::UptimeMillis();

		mTimer->start(msec);
	}
}

void QtTimer::stop()
{
	mRunning = false;
	mStarted = false;

	if(mTimer->isActive())
		mTimer->stop();
}

void QtTimer::setRunning(bool running,uint64_t rmsec)
{
	if(!mValidDuration)
		return ;

	if (mStarted == true) {
		if (mRunning != running) {
			if (running == true) {
				mGoStamp = SystemClock::UptimeMillis();

				if(rmsec == 0)
				{
					mTimer->start(mRemains);
				}
				else
					mTimer->start(rmsec);
			}
			else {
				mRemains = mRemains  - (SystemClock::UptimeMillis() - mGoStamp);
				mTimer->stop();
			}

			mRunning = running;
		}
	}
	else {
		LogE("QtTimer is NOT started, call start() first.\n");
	}
}

void QtTimer::setTimeout()
{
	emit timeout();
}

void QtTimer::qtimerout()
{
	emit timeout();
}

const char *QtTimer::TAG = "QtTimer";


