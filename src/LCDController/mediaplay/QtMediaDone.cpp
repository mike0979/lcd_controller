/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#include "QtMediaDone.h"
#include "QtSchedule.h"
#include "Log.h"
#include "QtTimer.h"

QtMediaDone::QtMediaDone()
{
	setDefaultValue();

	connect(&mTimerForLive,SIGNAL(timeout()),this,SLOT(slotTimeout()));
	connect(&mTimerForCheckBlack,SIGNAL(timeout()),this,SLOT(slotsingletimeshot()));
}

QtMediaDone::~QtMediaDone()
{
	if(mTimerForLive.isActive())
	{
		mTimerForLive.stop();
		disconnect(&mTimerForLive,SIGNAL(timeout()),this,SLOT(slotTimeout()));
	}

	if(mTimerForCheckBlack.isActive())
	{
		mTimerForCheckBlack.stop();
		disconnect(&mTimerForCheckBlack,SIGNAL(timeout()),this,SLOT(slotsingletimeshot()));
	}
}

void QtMediaDone::setDefaultValue()
{
	mQtSchedule = NULL;
	pMedias = NULL;
	pPartation = NULL;
	pContent = NULL;

	pVoid = NULL;

	mTimer = NULL;

	mMediaType = -1;

	bPlayStatus = false;

	mIsOpsFlag = false;

	mOPSPlaying = false;

	mLastPosition = -1;

	mLastPositionForTimer = -1;

	mSwitchmediaMsgSended = false;

	mVideostopCount = 0;
	mLastMSecsSinceEpoch = 0;

	mMediaDoneReleaseFlag = false;

	mCountTime = 0;
}
void QtMediaDone::setOPSPartation(bool flag)
{
	mIsOpsFlag = flag;
}

bool QtMediaDone::isOPSPartation()
{
	return mIsOpsFlag;
}

void QtMediaDone::onQtMediaDone(bool paused)
{
	if(mQtSchedule != NULL)
		mQtSchedule->notifyMediaDone(this);
}

void QtMediaDone::checkTimerActiveForLive()
{
	if(mSwitchmediaMsgSended)
	{
		LogD("##mSwitchmediaMsgSended==true ready to switch,don't start timer type:%d\n",mMediaType);
		return;
	}
	//after 10 seconds
	LogD("---- checkTimerActiveForLive, mediatype: %d\n",mMediaType);
	//connect(&mTimerForCheckBlack,SIGNAL(timeout()),this,SLOT(slotsingletimeshot()));
	mTimerForCheckBlack.setSingleShot(true);
	mTimerForCheckBlack.start(10000);
	//QTimer::singleShot(10000, this, SLOT(slotsingletimeshot()));
}
void QtMediaDone::slotTimeout()
{
	//LogD("QtMediaDone::slotTimeout  mLastPosition:%lld   mLastPositionForTimer: %lld\n",mLastPosition,mLastPositionForTimer);
	if(mMediaType == Json::MediaBasic::Live &&  !mQtSchedule->getFreezeStatus())
	{
		bool bequal = mLastPositionForTimer == mLastPosition;
		//bool blt = mLastPositionForTimer < -1;
		LogD("## mLastPosition:%lld,  mLastPositionForTimer: %lld, %d\n",
				mLastPosition,mLastPositionForTimer,bequal);
		if( bequal )
		{
			if(mCountTime >=5)
			{
				mCountTime = 1;

				mLastPosition = -1;

				mLastPositionForTimer = -1;

				if(mQtSchedule->hasMessage(QtSchedule::Msg_SwitchMedia) == 0
						&& !mSwitchmediaMsgSended)
				{
					mSwitchmediaMsgSended = true;

					if(mTimerForLive.isActive())
						mTimerForLive.stop();

					if(mTimerForCheckBlack.isActive())
						mTimerForCheckBlack.stop();

					/*if(blt)
					{
						LogD("set stream status true - 1\n");
						mQtSchedule->setStreamStatus(true);
					}
					else
					{*/
						LogD("set stream status false - 1\n");
						mQtSchedule->setStreamStatus(false);
					//}

					mQtSchedule->sendMessage(new Message(QtSchedule::Msg_SwitchMedia,this),100);
				}
				else
					LogD("MessageQueue had Msg_SwitchMedia message 2 \n");

			}
			else
			{
				mCountTime++;
			}

		}
		else
		{
			mCountTime = 1;
		}
	}

	mLastPositionForTimer = mLastPosition;
}

void QtMediaDone::OnCheckVideoStop()
{
	mVideostopCount++;
	LogD("videostop signal count - %d\n",mVideostopCount);

	qint64 msecs = QDateTime::currentMSecsSinceEpoch();
	if(msecs - mLastMSecsSinceEpoch > 1000*6)
	{
		mVideostopCount = 0;
		LogD("videostop signal count reset to 0 \n");
	}
	mLastMSecsSinceEpoch = msecs;

	if(mVideostopCount >= 3)
	{
		LogD("set stream status false - 6\n");

		if(mQtSchedule->hasMessage(QtSchedule::Msg_SwitchMedia) == 0
				&& !mSwitchmediaMsgSended)
		{
			mSwitchmediaMsgSended = true;

			if(mTimerForLive.isActive())
				mTimerForLive.stop();

			if(mTimerForCheckBlack.isActive())
				mTimerForCheckBlack.stop();

			mQtSchedule->setStreamStatus(false);
			mQtSchedule->sendMessage(new Message(QtSchedule::Msg_SwitchMedia,this),100);
		}
		else
			LogD("MessageQueue had Msg_SwitchMedia message 6 \n");

		mVideostopCount = 0;
	}
}
void QtMediaDone::slotsingletimeshot()
{
	if(mSwitchmediaMsgSended)
	{
		LogD("##mSwitchmediaMsgSended==true ready to switch,don't start timer type:%d\n",mMediaType);
		return;
	}

	LogD("---- slotsingletimeshot start, mediatype: %d\n",mMediaType);
	if(!mTimerForLive.isActive() && mMediaType == Json::MediaBasic::Live)
	{
		LogD("---- slotsingletimeshot timer do not start, start timer\n");
		mTimerForLive.start(1000);
	}
}
void QtMediaDone::onQtPositionChanged(qint64 position)
{
//	LogD("##position = %llu\n",position);
//	if( !mQtSchedule->getFreezeStatus())
//	{
//		if(mLastPosition == position)
//		{
//			mLastPosition = -1;
//
//			mQtSchedule->setStreamStatus(false);
//			//mQtSchedule->notifyMediaDone(this);
//			mQtSchedule->sendMessage(new Message(QtSchedule::Msg_SwitchMedia,this));
//
//	//		if(mTimerForLive.isActive())
//	//			mTimerForLive.stop();
//
//			return ;
//		}
//		else if(mLastPosition < -1)
//		{
//			mLastPosition = -1;
//
//			mQtSchedule->setStreamStatus(true);
//			mQtSchedule->sendMessage(new Message(QtSchedule::Msg_SwitchMedia,this));
//
//			return ;
//		}
//
//	}

	mLastPosition = position;

	if(mSwitchmediaMsgSended)
	{
		LogD("##mSwitchmediaMsgSended==true ready to switch %lld\n",position);
		return;
	}

	if(!mTimerForLive.isActive())
	{
		LogD("##start timer position = %lld\n",position);
		mTimerForLive.start(1000);
	}
}

void QtMediaDone::onmediaStatusChanged(QtAV::MediaStatus status)
{
	LogD("################# onmediaStatusChanged  status=%d\n",status);

	if(status == QtAV::MediaStatus::InvalidMedia ||
						status == QtAV::MediaStatus::UnknownMediaStatus ||
						status == QtAV::MediaStatus::NoMedia ||
						status == QtAV::MediaStatus::StalledMedia)
	{
		LogD("set stream status - 2\n");

		if(mQtSchedule->hasMessage(QtSchedule::Msg_SwitchMedia) == 0
				&& !mSwitchmediaMsgSended)
		{
			mSwitchmediaMsgSended = true;

			if(mTimerForLive.isActive())
				mTimerForLive.stop();

			if(mTimerForCheckBlack.isActive())
				mTimerForCheckBlack.stop();

			if(status == QtAV::MediaStatus::InvalidMedia ||
					status == QtAV::MediaStatus::NoMedia)
			{
				LogD("set stream status false - 2\n");
				mQtSchedule->setStreamStatus(false);
			}
			else
			{
				LogD("set stream status true - 2\n");
				mQtSchedule->setStreamStatus(true);
			}

			mQtSchedule->sendMessage(new Message(QtSchedule::Msg_SwitchMedia,this),100);
		}
		else
			LogD("MessageQueue had Msg_SwitchMedia message 3 \n");
	}
	else if(status == QtAV::MediaStatus::EndOfMedia)
	{
		LogD("set stream status true - EndOfMedia\n");

		if(mQtSchedule->hasMessage(QtSchedule::Msg_SwitchMedia) == 0
				&& !mSwitchmediaMsgSended)
		{
			mSwitchmediaMsgSended = true;

			if(mTimerForLive.isActive())
				mTimerForLive.stop();

			if(mTimerForCheckBlack.isActive())
				mTimerForCheckBlack.stop();

			mQtSchedule->setStreamStatus(true);
			mQtSchedule->sendMessage(new Message(QtSchedule::Msg_SwitchMedia,this),100);
		}
		else
			LogD("MessageQueue had Msg_SwitchMedia message 4 \n");


	}
	else if(status == QtAV::MediaStatus::BufferedMedia)
	{
		if(mQtSchedule->getBufferedCount() > 5)
		{
			LogD("set stream status true - bufferedcount > 5\n");


			if(mQtSchedule->hasMessage(QtSchedule::Msg_SwitchMedia) == 0
					&& !mSwitchmediaMsgSended)
			{
				mSwitchmediaMsgSended = true;

				if(mTimerForLive.isActive())
					mTimerForLive.stop();

				if(mTimerForCheckBlack.isActive())
					mTimerForCheckBlack.stop();

				mQtSchedule->setStreamStatus(true);
				mQtSchedule->sendMessage(new Message(QtSchedule::Msg_SwitchMedia,this),100);
			}
			else
				LogD("MessageQueue had Msg_SwitchMedia message 5 \n");

			return ;
		}

		mQtSchedule->setBufferedCountIncrease();
	}
}
void QtMediaDone::setSwitchmediaMsg(bool status)
{
	if(mSwitchmediaMsgSended != status)
		mSwitchmediaMsgSended = status;
}
bool QtMediaDone::getSwitchmediaMsg()
{
	return mSwitchmediaMsgSended;
}
const char *QtMediaDone::TAG = "QtMediaDone";
