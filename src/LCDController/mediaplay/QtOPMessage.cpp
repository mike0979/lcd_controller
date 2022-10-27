/*
 * QtOPMessage.cpp
 *
 *  Created on: Apr 10, 2017
 *      Author: root
 */

#include <Log.h>
#include <mediaplay/QtOPMessage.h>
#include <QtCore/QDebug>
#include <QtCore/qdatetime.h>
#include <QtCore/qglobal.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qobjectdefs.h>
#include <QtCore/qrect.h>
#include <QtCore/qsize.h>
#include <QtCore/qstring.h>
#include <QtCore/qtimer.h>
#include <QtGui/qcolor.h>
#include <QtGui/qfont.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qlabel.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpalette.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qsizepolicy.h>
#include <QtGui/qwidget.h>
#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <list>
#include <string>
#include <vector>
#include <QtGui/QPaintEvent>
#include <CommonDef.h>

QtOPMessage::QtOPMessage(QWidget *parent) : QWidget(parent)
{
    mImageBGLabel = new QLabel(this);
    mImageBGLabel->setBackgroundRole(QPalette::Window);
    mImageBGLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    mImageBGLabel->setScaledContents(true);
    mImageBGLabel->hide();

    mImageFrontLabel_header1 = new QLabel(this);
    mImageFrontLabel_header1->setBackgroundRole(QPalette::Window);
    mImageFrontLabel_header1->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    mImageFrontLabel_header1->setScaledContents(true);
    mImageFrontLabel_header1->setAttribute(Qt::WA_TranslucentBackground,false);
    mImageFrontLabel_header1->hide();

    mImageFrontLabel_header2 = new QLabel(this);
    mImageFrontLabel_header2->setBackgroundRole(QPalette::Window);
    mImageFrontLabel_header2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    mImageFrontLabel_header2->setScaledContents(true);
    mImageFrontLabel_header2->setAttribute(Qt::WA_TranslucentBackground,false);
    mImageFrontLabel_header2->hide();

    mImageFrontLabel = new QLabel(this);
    mImageFrontLabel->setBackgroundRole(QPalette::Window);
    mImageFrontLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    mImageFrontLabel->setScaledContents(true);
    mImageFrontLabel->setAttribute(Qt::WA_TranslucentBackground,false);
    mImageFrontLabel->hide();

    mStartPlay = false;

	connect(&mTimer,SIGNAL(timeout()),this,SLOT(slotTimeout()));

	mTimerForMarquee = new QTimer(this);

	mSpace = 120;

	mRunning = false;

	mTextPos = 0;
	mPaintPos = 0;
	mTextWidth = 0;
	mTextHeight = 0;
	mTextTotalHeight = 0;

	mTimeTrack = 0;

	connect(mTimerForMarquee, SIGNAL(timeout()), this, SLOT(invalidate()));

	setBGColor(Qt::GlobalColor::blue);

	mShiftCount = 0;
	mPrevShiftCount = 0;
	mMapIndex = 0;

	mHaveNewOPSMsg = false;
	mCurrOPSMsg = NULL;
	mCurrOPSRegion = unknownregion;

	mCurrOPSMsgList.clear();

	mHaveBackImage = false;

	connect(this, SIGNAL(signalOPMsgPlay(bool, OPSMsgParam*,int)), parent,
		SLOT(onOPMsgPlay(bool, OPSMsgParam*,int)));
	connect(this, SIGNAL(signalOPMsgPlayReply(const int, const OPSMsgPlayStatus)), parent,
		SLOT(onOPMsgPlayReply(const int, const OPSMsgPlayStatus)));
}

QtOPMessage::~QtOPMessage()
{
	for (std::list<OPSMsgParam*>::iterator iter =
			mCurrOPSMsgList.begin(); iter != mCurrOPSMsgList.end(); iter++ )
	{
		if(*iter != NULL)
		{
			delete *iter;
			*iter = NULL;
		}
	}

	if(mTimerForMarquee != NULL)
	{
		if(mTimerForMarquee->isActive())
			mTimerForMarquee->stop();

		delete mTimerForMarquee;
		mTimerForMarquee = NULL;
	}

	if(mImageBGLabel != NULL)
	{
		mImageBGLabel->clear();
		mImageBGLabel->hide();
		delete mImageBGLabel;
	}

	if(mImageFrontLabel != NULL)
	{
		mImageFrontLabel->clear();
		mImageFrontLabel->hide();
		delete mImageFrontLabel;
	}

	if(mImageFrontLabel_header1 != NULL)
	{
		mImageFrontLabel_header1->clear();
		mImageFrontLabel_header1->hide();
		delete mImageFrontLabel_header1;
	}

	if(mImageFrontLabel_header2 != NULL)
	{
		mImageFrontLabel_header2->clear();
		mImageFrontLabel_header2->hide();
		delete mImageFrontLabel_header2;
	}

	mCurrOPSRegion = unknownregion;
}

void QtOPMessage::resizeEvent(QResizeEvent* event)
{
	QSize size = event->size();

	mImageBGLabel->resize(size);

//	mImageFrontLabel_header1->setGeometry(QRect(0,60,size.width(),100));
//	mImageFrontLabel_header2->setGeometry(QRect(0,180,size.width(),120));
//	mImageFrontLabel->setGeometry(QRect(0,350,size.width(),size.height() - 350));
}

QString QtOPMessage::textAutoWap(QString text, QFont mFont,int w)
{
	QFontMetrics fm(mFont);

	QString str = "";
	int wTotal = 0;
	for(int index = 0;index<text.length();++index)
	{
		wTotal+=fm.width(text.at(index));
		if(wTotal >= w)
		{
			str.append('\n').append(text.at(index));
			wTotal = fm.width(text.at(index));
		}
		else
		{
			str.append(text.at(index));
		}
	}

	return str;
}
#include "FileSysUtils.h"
void QtOPMessage::setRunning(bool bstart,bool canced)
{
	mShiftCount = 0;
	mTimeTrack = 0;
	if(bstart)
	{
		if(mCurrOPSMsg == NULL)
		{
			LogE("error: mCurrOPSMsg == NULL \n");
			return;
		}

		mCurrOPSMsg->mPlayStatus = StartPlaying;
		emit signalOPMsgPlayReply(mCurrOPSMsg->mID,StartPlaying);

		mRunning = true;
		if (mCurrOPSMsg->mHaveBackColor)
			setBGColor(mCurrOPSMsg->mBackColor);

		//setSpeed(mCurrOPSMsg->mSpeed);

		if(mCurrOPSMsg->mEffect == (int)LeftEffect)
			mPaintPos = this->width();
		else if(mCurrOPSMsg->mEffect == (int)RightEffect)
			mPaintPos = 0;
		else if(mCurrOPSMsg->mEffect == (int)UpEffect)
			mPaintPos = this->height();
		else
			mPaintPos = this->width();

		std::size_t pos_m_1 = mCurrOPSMsg->mText.toStdString().find(".jpg");
		std::size_t pos_m_2 = mCurrOPSMsg->mText.toStdString().find(".png");

		if(pos_m_1 != std::string::npos || pos_m_2 != std::string::npos)
		{
			std::string filepath = "/home/workspace/media/";
			filepath.append(mCurrOPSMsg->mText.toStdString());
			LogD("OPS image filename: %s\n",filepath.c_str());
			if(FileSysUtils::Accessible(filepath, FileSysUtils::FR_OK))
			{
				mImageBGLabel->setGeometry(QRect(0,0,this->width(),this->height()));
				QImage BGImage;
				BGImage.load(filepath.c_str());
				mImageBGLabel->clear();
				QPixmap pixImage = pixImage.fromImage(BGImage);
				mImageBGLabel->setPixmap(pixImage);
				mImageBGLabel->setAlignment(Qt::AlignCenter);
				mImageBGLabel->show();
				mImageBGLabel->raise();
			}
			else
			{
				mImageBGLabel->hide();
				LogD("Don't find OPS image filename in /home/workspace/media/: %s\n",filepath.c_str());
			}
			return ;
		}
		else
		{
			mImageBGLabel->hide();
		}

		mCurrOPSMsg->mEffect = UpEffect;
		if(mCurrOPSMsg->mEffect == StaticEffect)
		{
			LogD("static effect\n");

			QPalette PaletteText;

			if(mCurrOPSMsg->mPriority == OPS_PriorityEmergency)
			{
				mImageFrontLabel_header1->setGeometry(QRect(0,60,this->width(),100));
				mImageFrontLabel_header2->setGeometry(QRect(0,180,this->width(),120));
				mImageFrontLabel->setGeometry(QRect(0,350,this->width(),this->height() - 350));

				PaletteText.setColor(QPalette::WindowText, mCurrOPSMsg->mColor);
				mImageFrontLabel_header1->setPalette(PaletteText);
				mImageFrontLabel_header1->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
				mImageFrontLabel_header1->setWordWrap(true);
				mImageFrontLabel_header1->setFont(mCurrOPSMsg->mFont);
				mImageFrontLabel_header1->setText("紧急通知");
				mImageFrontLabel_header1->show();

				PaletteText.setColor(QPalette::WindowText, mCurrOPSMsg->mColor);
				mImageFrontLabel_header2->setPalette(PaletteText);
				mImageFrontLabel_header2->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
				mImageFrontLabel_header2->setWordWrap(true);
				mImageFrontLabel_header2->setFont(mCurrOPSMsg->mFont);
				mImageFrontLabel_header2->setText("Emergency");
				mImageFrontLabel_header2->show();
				mImageFrontLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
			}
			else
			{
				mImageFrontLabel_header1->hide();
				mImageFrontLabel_header2->hide();
				mImageFrontLabel->setGeometry(QRect(0,0,this->width(),this->height()));
				mImageFrontLabel->setAlignment(mCurrOPSMsg->mAlign | Qt::AlignVCenter);
			}
			PaletteText.setColor(QPalette::WindowText, mCurrOPSMsg->mColor);
			mImageFrontLabel->setPalette(PaletteText);

			//mImageFrontLabel->setWordWrap(true);
			mImageFrontLabel->setFont(mCurrOPSMsg->mFont);
			mImageFrontLabel->setText(textAutoWap(mCurrOPSMsg->mText,mCurrOPSMsg->mFont,mImageFrontLabel->width()));
			mImageFrontLabel->show();
			mImageFrontLabel->raise();
		}
		else if(mCurrOPSMsg->mEffect == LeftEffect ||
				mCurrOPSMsg->mEffect == RightEffect ||
				mCurrOPSMsg->mEffect == UpEffect)
		{
//			QPalette PaletteText;
//			if(mCurrOPSMsg->mPriority == OPS_PriorityEmergency && mCurrOPSMsg->mEffect != UpEffect)
//			{
//				mImageFrontLabel_header1->setGeometry(QRect(0,60,this->width(),100));
//				mImageFrontLabel_header2->setGeometry(QRect(0,180,this->width(),120));
//				mImageFrontLabel->setGeometry(QRect(0,350,this->width(),this->height() - 350));
//
//				PaletteText.setColor(QPalette::WindowText, mCurrOPSMsg->mColor);
//				mImageFrontLabel_header1->setPalette(PaletteText);
//				mImageFrontLabel_header1->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
//				mImageFrontLabel_header1->setWordWrap(true);
//				mImageFrontLabel_header1->setFont(mCurrOPSMsg->mFont);
//				mImageFrontLabel_header1->setText("紧急通知");
//				mImageFrontLabel_header1->show();
//
//				PaletteText.setColor(QPalette::WindowText, mCurrOPSMsg->mColor);
//				mImageFrontLabel_header2->setPalette(PaletteText);
//				mImageFrontLabel_header2->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
//				mImageFrontLabel_header2->setWordWrap(true);
//				mImageFrontLabel_header2->setFont(mCurrOPSMsg->mFont);
//				mImageFrontLabel_header2->setText("Emergency");
//				mImageFrontLabel_header2->show();
//			}

			if(mTimerForMarquee->isActive())
			{
				mTimerForMarquee->stop();
			}

			updateTextWidth();
			updateCache();
			mTextPos = 0;

			if (mCurrOPSMsg->mSpeed > FPS) {
				mTimerForMarquee->start(1000 / FPS);
			}
			else {
				mTimerForMarquee->start(1000 / mCurrOPSMsg->mSpeed);
			}
		}
		else
		{
			LogD("unknown effect\n");
		}
	}
	else
	{
		mRunning = false;

//		if(mCurrOPSMsg!= NULL)
//		{
//			mCurrOPSMsg->mPlayStatus = PlayEnd;
//			emit signalOPMsgPlayReply(mCurrOPSMsg->mID,PlayEnd);
//		}

		if(mTimerForMarquee->isActive())
		{
			mTimerForMarquee->stop();
		}

		if(mTimer.isActive() && (canced || mCurrOPSMsgList.size() == 0))
		{
			mTimer.stop();
		}

		mCurrOPSMsg = NULL;
		mStartPlay = false;

		mImageBGLabel->clear();
		mImageFrontLabel->clear();
		mImageFrontLabel_header1->clear();
		mImageFrontLabel_header2->clear();

		mImageBGLabel->hide();
		mImageFrontLabel->hide();
		mImageFrontLabel_header1->hide();
		mImageFrontLabel_header2->hide();
	}

}
void QtOPMessage::slotTimeout()
{
	//hava new ops message ready to play
	if((!mStartPlay ||  mCurrOPSMsg == NULL) && mCurrOPSMsgList.size() > 0)
	{
		switchLoopOPSMsg();
		mPrevShiftCount = 0;
	}

	if(mCurrOPSMsg == NULL || mCurrOPSMsgList.size() == 0)
	{
		//emit signalOPMsgPlayReply(0,PlayStoped);
		emit signalOPMsgPlay(false,NULL, mCurrOPSRegion);
		return ;
	}

	QDateTime currtime = QDateTime::currentDateTime();
	OPSMsgParam tempCurrOPSMsg;
	if(!mStartPlay && mCurrOPSMsg != NULL && currtime>=mCurrOPSMsg->mStartTime && currtime <=mCurrOPSMsg->mEndTime)
	{
		mStartPlay = true;
//		mCurrOPSMsg->mPlayStatus = StartPlaying;
//
//		emit signalOPMsgPlayReply(mCurrOPSMsg->mID,StartPlaying);
		emit signalOPMsgPlay(true,mCurrOPSMsg);
		return ;
	}

	//play ops message end
	if(mStartPlay && mCurrOPSMsg != NULL && currtime > mCurrOPSMsg->mEndTime)
	{
		mStartPlay = false;
		mCurrOPSMsg->mPlayStatus = PlayEnd;


		LogD("OPSMsg play end, endtime-%s, id-%d\n",mCurrOPSMsg->mEndTime.toString().toStdString().c_str(),mCurrOPSMsg->mID);
		tempCurrOPSMsg = *mCurrOPSMsg;

		std::list<int> cleanIdList;
		cleanIdList.clear();
		cleanOPSPlayList(cleanIdList);

		emit signalOPMsgPlay(false,&tempCurrOPSMsg);
		for(std::list<int>::iterator it = cleanIdList.begin();
				it!=cleanIdList.end();it++)
		{
			emit signalOPMsgPlayReply(*it,PlayEnd);
		}

		return ;
	}

	//
	if(mStartPlay && mCurrOPSMsg != NULL && mCurrOPSMsg->mPlayCount > 0 && mCurrOPSMsg->mPlayedCount >= mCurrOPSMsg->mPlayCount)
	{
		mShiftCount = 0;
		mStartPlay = false;
		mCurrOPSMsg->mPlayStatus = PlayEnd;

		LogD("OPSMsg play end,playcount-%d, id-%d\n",mCurrOPSMsg->mPlayedCount,mCurrOPSMsg->mID);
		tempCurrOPSMsg = *mCurrOPSMsg;

		std::list<int> cleanIdList;
		cleanIdList.clear();
		cleanOPSPlayList(cleanIdList);

		emit signalOPMsgPlay(false,&tempCurrOPSMsg);
		for(std::list<int>::iterator it = cleanIdList.begin();
				it!=cleanIdList.end();it++)
		{
			emit signalOPMsgPlayReply(*it,PlayEnd);
		}

		return ;
	}
}

bool QtOPMessage::setOPSMsg(void* msg, int status)
{
	if(status == (int) OPSUpdateStatus::OPS_deletestatus)
	{
		int* deleteid = (int*)msg;
		if(mCurrOPSMsg!= NULL && *deleteid == mCurrOPSMsg->mID)
		{
			mStartPlay = false;
		}

		for (std::list<OPSMsgParam*>::iterator iter =
				mCurrOPSMsgList.begin(); iter != mCurrOPSMsgList.end(); )
		{
			if(*deleteid == (*iter)->mID)
			{
				iter = mCurrOPSMsgList.erase(iter);
			}
			else
			{
				iter++;
			}
		}
//		DELETE_ALLOCEDRESOURCE(deleteid);
		return true;
	}

	Json::OPSMsgDetail* opsmsg = (Json::OPSMsgDetail*)msg;
	if(opsmsg != NULL)
	{
		LogD("ops msg id = %d\n",opsmsg->mBasic.mId);
		OPSMsgParam* param = NULL;
		setParam(opsmsg,param);
		if(param != NULL)
		{
			bool bupdate = false;
			for (std::list<OPSMsgParam*>::iterator iter =
					mCurrOPSMsgList.begin(); iter != mCurrOPSMsgList.end(); iter++)
			{
				if(param->mID == (*iter)->mID)
				{
					bupdate = true;
					break;
				}
			}

			if(bupdate && mCurrOPSMsg!= NULL && param->mID == mCurrOPSMsg->mID)
			{
				mStartPlay = false;
			}
			else
			{
				if((mCurrOPSMsg != NULL && param->mPriority > mCurrOPSMsg->mPriority) ||
						param->mPlaymode == OPSQueueFlag::OverWrite)
				{
					if(mTimerForMarquee->isActive())
					{
						mTimerForMarquee->stop();
					}

					if(mTimer.isActive())
					{
						mTimer.stop();
					}

					mCurrOPSMsg = NULL;
					//for (std::list<OPSMsgParam*>::iterator iter =
					//		mCurrOPSMsgList.begin(); iter != mCurrOPSMsgList.end(); )
					//{
					//	delete *iter;
					//	iter = mCurrOPSMsgList.erase(iter);
					//}

					mStartPlay = false;
					auto it = find_if(mCurrOPSMsgList.begin(), mCurrOPSMsgList.end(), [param](const OPSMsgParam* ops_param)
						{
							return ops_param->mID == param->mID;
						});
					if (it == mCurrOPSMsgList.end())
					{
						mCurrOPSMsgList.push_back(param);
					}
				}
				else if((mCurrOPSMsg != NULL && param->mPriority == mCurrOPSMsg->mPriority) &&
						param->mPlaymode == OPSQueueFlag::QueueLoop)
				{
					mCurrOPSMsgList.push_back(param);
				}
				else if(mCurrOPSMsg == NULL)
				{
					mCurrOPSMsgList.push_back(param);
				}

			}

		}
	}

	if(!mTimer.isActive())
	{
		mTimer.start(1000);
	}

	return true;
}

bool QtOPMessage::setParam(Json::OPSMsgDetail* opmsg,OPSMsgParam* &opsparam)
{

	if(opmsg != NULL)
	{
		mLastRemain = 0;
		OPSMsgParam* opsmsgparam = new OPSMsgParam();

		opsmsgparam->mID = opmsg->mBasic.mId;
		opsmsgparam->mTimeStamp = opmsg->mBasic.mUpdateTime;

		//opsmsgparam->mFormat = opmsg->mTextFormat->mStyle->mFormat;

		opsmsgparam->mSize = opmsg->mText.mFont.mSize;
		opsmsgparam->mFont = QFont(opmsg->mText.mFont.mName.c_str(),opsmsgparam->mSize);
		opsmsgparam->mFont.setItalic(opmsg->mText.mFont.mIsItalic);
		opsmsgparam->mFont.setBold(opmsg->mText.mFont.mIsBold);

		opsmsgparam->mColor = QColor(strtoul((opmsg->mText.mForeColor).c_str(), NULL, 16));

		if(opmsg->mText.mBackColor.size() > 0)
		{
			opsmsgparam->mBackColor = QColor(strtoul((opmsg->mText.mBackColor).c_str(), NULL, 16));
			opsmsgparam->mHaveBackColor = true;
		}
		else
		{
			opsmsgparam->mHaveBackColor = false;
		}

		//setBGColor(mBackColor);
		opsmsgparam->mEffect = opmsg->mText.mEffect;
		opsmsgparam->mSpeed = opmsg->mText.mSpeed;

		opsmsgparam->mPriority = opmsg->mPriority;
		opsmsgparam->mPlaymode = opmsg->mPlayMode;
		opsmsgparam->mPlayCount = opmsg->mBasic.mPlayCnt;
		opsmsgparam->mPlayedCount = 0;

		opsmsgparam->mDisplayRegion = opmsg->mDisplayRegion;

		opsmsgparam->mText = opmsg->mContent.c_str();
		opsmsgparam->mStartTime = QDateTime::fromString(opmsg->mBasic.mStartTime.c_str(),"yyyyMMdd hhmmss");
		opsmsgparam->mEndTime = QDateTime::fromString(opmsg->mBasic.mEndTime.c_str(),"yyyyMMdd hhmmss");

		opsmsgparam->mPlayStatus = ReadyPlay;

		// set back ground image.
		opsmsgparam->mBackImgName.clear();
		opsmsgparam->mBackImgName.append(opmsg->mText.mBackImageDir.c_str());
		opsmsgparam->mBackImgName.append(opmsg->mText.mBackImage.c_str());
		//opsmsgparam->mBackImgName.append("test.png");
		LogD("OPS BackImg path: %s\n",opsmsgparam->mBackImgName.toStdString().c_str());

		if(opmsg->mText.mAlign == opmsg->mText.Left)
			opsmsgparam->mAlign = Qt::AlignLeft;
		else if(opmsg->mText.mAlign == opmsg->mText.Center)
			opsmsgparam->mAlign = Qt::AlignCenter;
		else if(opmsg->mText.mAlign == opmsg->mText.Right)
			opsmsgparam->mAlign = Qt::AlignRight;
		else
			opsmsgparam->mAlign = Qt::AlignLeft;

		if(opsmsgparam->mPriority == 1)
			opsmsgparam->mOPSMsgType = EmergencyMsg;
		else if(opsmsgparam->mPriority == 2)
			opsmsgparam->mOPSMsgType = ImportanceMsg;
		else if(opsmsgparam->mPriority == 3)
			opsmsgparam->mOPSMsgType = NormalMsg;
		else if(opsmsgparam->mPriority == 4)
			opsmsgparam->mOPSMsgType = DefaultMsg;
		else
			opsmsgparam->mOPSMsgType = UnknownMsg;

		QDateTime currtime = QDateTime::currentDateTime();
		if(currtime >=opsmsgparam->mEndTime)
		{
			LogD("OPS message was out of data\n");
			return false;
		}

		opsparam = opsmsgparam;

		return true;
	}
	else
	{
		opsparam = NULL;
		LogE("OPSMsg detail == NULL\n");
	}

	return false;
}

OPSDisplayRegion QtOPMessage::getDisplayRegion()
{
	return mCurrOPSRegion;
}

int QtOPMessage::getOPSPlayListSize()
{
	return mCurrOPSMsgList.size();
}

void QtOPMessage::switchLoopOPSMsg()
{
	std::list<OPSMsgParam*>::iterator opsiter;
	bool bfind = false;

	if(mCurrOPSMsgList.size() == 1)
	{
		bfind = true;
		opsiter = mCurrOPSMsgList.begin();

	}
	else if(mCurrOPSMsgList.size() > 1)
	{
		bfind = false;
		for (std::list<OPSMsgParam*>::iterator iter =
				mCurrOPSMsgList.begin(); iter != mCurrOPSMsgList.end(); iter++)
		{
			if(mCurrOPSMsg == NULL)
			{
				opsiter = --mCurrOPSMsgList.end();
				bfind = true;
			}
			else if(mCurrOPSMsg->mID == (*iter)->mID/* ||
					mCurrOPSMsg->mTimeStamp == (*iter)->mTimeStamp*/)
			{
				opsiter = iter;
				bfind = true;
				break;
			}
		}

		if(!bfind || opsiter == mCurrOPSMsgList.end())
		{
			opsiter = --mCurrOPSMsgList.end();
			bfind = true;
		}

	}

	if(bfind)
	{
		mCurrOPSMsg = *opsiter;
		mCurrOPSRegion = (OPSDisplayRegion)mCurrOPSMsg->mDisplayRegion;
	}
	else
	{
		mCurrOPSMsg = NULL;
		mCurrOPSRegion = unknownregion;
		LogE("Do not find available OPSMsg\n");
	}
}

void QtOPMessage::cleanOPSPlayList(std::list<int>& cleanidlist)
{
	OPSMsgParam* opsmsg = NULL;
	QDateTime currtime = QDateTime::currentDateTime();
	for (std::list<OPSMsgParam*>::iterator iter =
			mCurrOPSMsgList.begin(); iter != mCurrOPSMsgList.end(); )
	{
		opsmsg = NULL;
		//erase out of data and play end
		if((*iter)->mEndTime <= currtime ||
				((*iter)->mID == mCurrOPSMsg->mID && (*iter)->mTimeStamp == mCurrOPSMsg->mTimeStamp ))
		{
			LogD("clearup play list, id-%d\n",(*iter)->mID);

			cleanidlist.push_back((*iter)->mID);
			opsmsg = *iter;
			iter = mCurrOPSMsgList.erase(iter);
			if(opsmsg!= NULL)
			{
				delete opsmsg;
				opsmsg = NULL;
			}
		}
		else
		{
			iter++;
		}
	}

	LogD("play list, size-%d\n",mCurrOPSMsgList.size());
}

void QtOPMessage::setResolution(QRect &rect)
{
    QSize size;
    size.setHeight(rect.height());
    size.setWidth(rect.width());

    mImageBGLabel->resize(size);
    mImageFrontLabel->resize(size);

	this->setGeometry(rect.left(), rect.top(), rect.right(), rect.bottom());
}

void QtOPMessage::setSpeed(int speed)
{
	if(speed <0 || speed>4)
	{
		LogE("Text speed(%d) should between 0 and 4, speed will be set default value\n",speed);
		mCurrOPSMsg->mSpeed= SpeedLevel_4;
	}

	switch(speed)
	{
	case 0:
		mCurrOPSMsg->mSpeed = SpeedLevel_0;
		break;
	case 1:
		mCurrOPSMsg->mSpeed = SpeedLevel_1;
		break;
	case 2:
		mCurrOPSMsg->mSpeed = SpeedLevel_2;
		break;
	case 3:
		mCurrOPSMsg->mSpeed = SpeedLevel_3;
		break;
	case 4:
		mCurrOPSMsg->mSpeed = SpeedLevel_4;
		break;
	}
}

void QtOPMessage::setBGColor(const QColor &color)
{
	//Qt::GlobalColor::black

    QPalette mDefBGPalette;
	mDefBGPalette.setColor(QPalette::Window, color);
	this->setAutoFillBackground(true);
    setPalette(mDefBGPalette);
}

void QtOPMessage::setBGFile(const std::string &file)
{
	if(mBackPixmap.load(file.c_str()))
	{
		mHaveBackImage = true;
	}
	else
	{
		mHaveBackImage = false;
	}
}

void QtOPMessage::updateTextWidth()
{
//	mCurrOPSMsg->mFont = QFont("黑体",80);
//	mCurrOPSMsg->mFont.setBold(true);
//	mCurrOPSMsg->mFont.setPointSize(80);
	//mCurrOPSMsg->mFont.setPointSize(mCurrOPSMsg->mSize);
	QFontMetrics fm(mCurrOPSMsg->mFont);

	//mTextWidth = fm.width(mCurrOPSMsg->mText);
	mTextWidth = 0;
	mTextHeight = fm.height();
	mTextTotalHeight = 0;

	mLinesInfo.clear();
	int tempwidth = 0;
	QString tempstr = "";

	for(int index = 0;index<mCurrOPSMsg->mText.length();++index)
	{
		if(mCurrOPSMsg->mEffect == (int)UpEffect)
		{
//			mLinesInfo.push_back(LinesInfo(mCurrOPSMsg->mText.at(index), fm.height()));
//			mTextTotalHeight += fm.height();

			if(tempwidth <= this->width() && tempwidth + fm.width(mCurrOPSMsg->mText.at(index)) > this->width())
			{
				mLinesInfo.push_back(LinesInfo(tempstr, tempwidth));
				if(mCurrOPSRegion == OPSDisplayRegion::fullscreenshow ||
						mCurrOPSRegion == OPSDisplayRegion::masterpartation)
				{
					mTextTotalHeight += fm.height()*1.2;
				}
				else
				{
					mTextTotalHeight += fm.height();
				}


				tempwidth = 0;
				tempstr = "";
			}

			tempwidth += fm.width(mCurrOPSMsg->mText.at(index));
			tempstr +=mCurrOPSMsg->mText.at(index);
		}
		else
		{
			mLinesInfo.push_back(LinesInfo(mCurrOPSMsg->mText.at(index), fm.width(mCurrOPSMsg->mText.at(index))));
			mTextWidth+=fm.width(mCurrOPSMsg->mText.at(index));
		}
	}

	if(mCurrOPSMsg->mEffect == (int)UpEffect && tempstr.size() > 0)
	{
		mLinesInfo.push_back(LinesInfo(tempstr, tempwidth));
		mTextTotalHeight += fm.height();
	}

//		for(int i=0;i<mLinesInfo.size();++i)
//		{
//			LogD("---- %s\n",mLinesInfo.at(i).mText.toStdString().c_str());
//		}
}

void QtOPMessage::updateCache()
{
	deleteCache();

	for (std::vector<LinesInfo>::iterator i = mLinesInfo.begin(); i != mLinesInfo.end(); i++) {
		QPixmap *pixmap = new QPixmap(i->mPixWidth, mTextHeight);
		pixmap->fill(Qt::transparent);

		QPainter painter(pixmap);

		painter.setFont(mCurrOPSMsg->mFont);
		//painter.setPen(mCurrOPSMsg->mColor);
		painter.setPen(0xffffff);
		painter.drawText(0, 0, i->mPixWidth, mTextHeight, Qt::AlignCenter, i->mText);

		mCache.push_back(pixmap);
	}
}

void QtOPMessage::deleteCache()
{
	for (std::vector<QPixmap *>::iterator i = mCache.begin(); i != mCache.end(); i++) {
		delete (*i);
	}

	mCache.clear();
}

void QtOPMessage::paintEvent(QPaintEvent * evt)
{
	if(mCurrOPSMsg == NULL)
	{
		LogD("mCurrOPSMsg == NULL\n");
		return ;
	}
	if (mCache.size() == 0) {
		updateTextWidth();
		updateCache();
		return ;
	}

	int width = this->width();
	int height = this->height();

	if (mSpace >= width) {
		mSpace = width / 2;
	}

	QPainter painter1(this);
	painter1.setClipRect(0, 0, width, height, Qt::ReplaceClip);
	painter1.setCompositionMode(QPainter::CompositionMode_Source);

	QRect rect;
	rect.setX(0);
	rect.setY(0);
	rect.setWidth(width);
	rect.setHeight(height);
	//painter1.fillRect(rect,0xff);
	if (mHaveBackImage)
	{
		painter1.drawPixmap(rect, mBackPixmap, geometry());
	}
	painter1.setCompositionMode(QPainter::CompositionMode_Destination);
	painter1.end();

	QPainter painter(this);
	painter.setClipRect(0, 0, width, height, Qt::ReplaceClip);


	if(mCurrOPSRegion == OPSDisplayRegion::fullscreenshow && mLinesInfo.size() <= 6)
	{
		int ipos = height/2 - (mTextHeight*1.2)*mLinesInfo.size()*0.5;

		for (unsigned i = 0; i < mLinesInfo.size(); i++) {

			int ix = 20;
			int iy = ipos;

			painter.drawPixmap(ix, iy, *mCache[i]);

			ipos += mTextHeight*1.2;
		}
		return ;
	}
	else if(mCurrOPSRegion == OPSDisplayRegion::masterpartation && mLinesInfo.size() <= 5)
	{
		int ipos = height/2 - (mTextHeight*1.2)*mLinesInfo.size()*0.5;

		for (unsigned i = 0; i < mLinesInfo.size(); i++) {

			int ix = 20;
			int iy = ipos;

			painter.drawPixmap(ix, iy, *mCache[i]);

			ipos += mTextHeight*1.2;
		}
		return ;
	}
	else if(mCurrOPSRegion == OPSDisplayRegion::partationshow && mLinesInfo.size() <= 3)
	{
		int ipos = height/2 - mTextHeight*mLinesInfo.size()*0.5;

		for (unsigned i = 0; i < mLinesInfo.size(); i++) {

			int ix = 20;
			int iy = ipos;

			painter.drawPixmap(ix, iy, *mCache[i]);

			ipos += mTextHeight;
		}
		return ;
	}

	int ipos = mPaintPos;
	for (unsigned i = 0; i < mLinesInfo.size(); i++) {
		if (true || ipos + mLinesInfo[i].mPixWidth > 0 && ipos <= width && mCache[i]!=NULL) {
			int ix = 0;
			int iy = 0;
			int ix2=0x7fffffff,iy2=0x7fffffff;

			if(mCurrOPSMsg->mEffect == (int)LeftEffect || mCurrOPSMsg->mEffect == (int)RightEffect)
			{
				ix = ipos;iy=(height - mTextHeight) / 2;
				ix2=ix+width/2+mTextWidth;
				iy2=iy;
			}
			else if(mCurrOPSMsg->mEffect == (int)UpEffect)
			{
				ix = 20; iy = ipos;
				ix2=ix;
				if(mCurrOPSRegion == OPSDisplayRegion::fullscreenshow ||
									mCurrOPSRegion == OPSDisplayRegion::masterpartation)
				{
					iy2=iy+height/2+(mTextHeight*1.2*mLinesInfo.size()-mTextHeight*0.2);
				}
				else
				{
					iy2=iy+height/2+mTextHeight*mLinesInfo.size();
				}
			}
//			else if(mCurrOPSMsg->mEffect == (int)UpEffect && mCurrOPSMsg->mAlign == Qt::AlignCenter)
//			{
//				ix = (width - mTextHeight) / 2 ;iy = ipos;
//			}
//			else if(mCurrOPSMsg->mEffect == (int)UpEffect && mCurrOPSMsg->mAlign == Qt::AlignRight)
//			{
//				ix = width - mTextHeight ;iy = ipos;
//			}
			else
			{
				ix = ipos;iy=(height - mTextHeight) / 2;
			}
			painter.drawPixmap(ix, iy, *mCache[i]);
			if(ix2<width&&iy2<height)
			{
				painter.drawPixmap(ix2, iy2, *mCache[i]);
			}
		}

		if(mCurrOPSMsg->mEffect == (int)LeftEffect)
			ipos += mLinesInfo[i].mPixWidth;
		else if(mCurrOPSMsg->mEffect == (int)RightEffect)
			ipos -= mLinesInfo[i].mPixWidth;
		else if(mCurrOPSMsg->mEffect == (int)UpEffect)
		{
			if(mCurrOPSRegion == OPSDisplayRegion::fullscreenshow ||
					mCurrOPSRegion == OPSDisplayRegion::masterpartation)
				ipos += mTextHeight*1.2;
			else
			{
				ipos += mTextHeight;
			}
		}
		else
			ipos += mLinesInfo[i].mPixWidth;
	}
}

void QtOPMessage::invalidate()
{
	if (mRunning && mCurrOPSMsg!= NULL) {

		if (0 == mTimeTrack)
		{
			mTimeTrack = SystemClock::SystemTime(SystemClock::SYSTEM_TIME_MONOTONIC);
		}
		uint64_t time = SystemClock::SystemTime(SystemClock::SYSTEM_TIME_MONOTONIC) - mTimeTrack;
		mTextPos = mLastRemain+(int)(mCurrOPSMsg->mSpeed * (time / 1000000) / 1000.0 + 0.5);

		bool bvalue = false;
		if(mCurrOPSMsg->mEffect == (int)LeftEffect)
			bvalue = (mTextPos >= mTextWidth) && (mTextPos-mTextWidth > this->width());
		else if(mCurrOPSMsg->mEffect == (int)RightEffect)
			bvalue = (mTextPos - mTextWidth >= this->width());
		else if(mCurrOPSMsg->mEffect == (int)UpEffect)
			bvalue = (mTextPos - mTextTotalHeight >= this->height());
		else  //default - left
			bvalue = (mTextPos >= mTextWidth) && (mTextPos-mTextWidth > this->width());

		if (bvalue)
		{
			mShiftCount++;

			mCurrOPSMsg->mPlayedCount++;
			if(mCurrOPSMsgList.size() > 1)
			{
				mStartPlay = false;
				mTimerForMarquee->stop();
			}

			if(mCurrOPSMsg->mPlayCount > 0 && mCurrOPSMsg->mPlayedCount >= mCurrOPSMsg->mPlayCount)
			{
				mTimerForMarquee->stop();
			}

			if(mCurrOPSMsg->mEffect == (int)RightEffect)
				mTextPos = -mTextWidth;
			else if(mCurrOPSMsg->mEffect == (int)LeftEffect)
				mTextPos = this->width()/2;
			else if(mCurrOPSMsg->mEffect == (int)UpEffect)
				mTextPos = this->height()/2;
			else
				mTextPos = 0;//mTextPos - mTextWidth - mSpace;
			mTimeTrack = 0;
			mLastRemain=mTextPos;
		}
	}

	int pos = mPaintPos;
	if(mCurrOPSMsg->mEffect == (int)LeftEffect)
		 pos = this->width()-mTextPos;
	else if(mCurrOPSMsg->mEffect == (int)RightEffect)
		pos = mTextPos;
	else if(mCurrOPSMsg->mEffect == (int)UpEffect)
		pos = this->height()-mTextPos;
	else
		pos = this->width()-mTextPos;

	if (pos != mPaintPos) {
		mPaintPos = pos;

		repaint();
	}

}

QtOPMessage::LinesInfo::LinesInfo(QString text, int pw) : mText(text), mPixWidth(pw)
{

}

const char *QtOPMessage::TAG = "QtOPMessage";
const int QtOPMessage::FPS = 500;
const int QtOPMessage::MAX_CACHE_SIZE = 8192;
