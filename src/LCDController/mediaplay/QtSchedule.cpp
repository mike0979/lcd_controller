/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#include "QtSchedule.h"
#include "json/ScheduleObjs.h"
#include "QtMediaDone.h"
#include "QtMarquee.h"
#include "QtImage.h"
#include "QtVideoPlayer.h"
#include "QtArrivalMsg.h"
#include "QtFlash.h"
#include "QtClock.h"
#include "QtStreamDetector.h"
#include "Log.h"
#include "FileSysUtils.h"
#include "config/configparser.h"
#include "LCDController.h"
#include "CommonDef.h"
#include <map>
#include <QtGui/qpixmapcache.h>
#include "mediaplay/QtOPMessage.h"

#define RELEASE_UI_WIDGET(widget)		do {												\
											while (m##widget##InUse.size()) {				\
												release##widget (m##widget##InUse.front());	\
											}												\
										} while (0)

#define FREEZE_UI_WIDGET(widget, op)	do {																									\
											for (std::list<widget *>::iterator i = m##widget##InUse.begin(); i != m##widget##InUse.end(); i++) {\
												(*i)->setRunning((op) == false);																\
											}																									\
										} while (0)

#define DELETE_UI_WIDGET(widget)		do {																									\
											for (std::list<widget *>::iterator i = m##widget##Pool.begin(); i != m##widget##Pool.end(); i++) {	\
												delete (*i);																					\
											}																									\
											for (std::list<widget *>::iterator i = m##widget##InUse.begin(); i != m##widget##InUse.end(); i++) {\
												delete (*i);																					\
											}																									\
											m##widget##Pool.clear();  \
											m##widget##InUse.clear();  \
										} while (0)


QtSchedule::QtSchedule(LCDController* lcdcontroller,QWidget *parent) : QWidget(parent),
	mLCDController(lcdcontroller)
{
	const ConfigParser* config = mLCDController->getConfig();
	mMediasPath = "";
	mQtTimerDoneMapper.clear();
	mBUsedWidgetsFreeze = false;

	connect(this,SIGNAL(signalQtMediaDone(QtMediaDone *)), this, SLOT(onQtMediaDone(QtMediaDone *)), Qt::QueuedConnection);

	connect(this,SIGNAL(signalStartPlaynewLayout()), this, SLOT(slotStartPlaynewLayout()), Qt::QueuedConnection);
	connect(this,SIGNAL(signalPlayNextContent(Json::PartitionDetail*, Json::MediaBasic*)),
			this, SLOT(slotPlayNextContent(Json::PartitionDetail*, Json::MediaBasic*)), Qt::QueuedConnection);

	mStreamDector = new QtStreamDetector(config->mMultiCastStreamIP,config->mMultiCastStreamPort,config->mMultiCastTimeout);
	connect(mStreamDector,SIGNAL(signalProbeStreamReport(bool)), this, SLOT(onProbeStreamReport(bool)), Qt::QueuedConnection);
	connect(this,SIGNAL(signalPauseStreamDetect()), mStreamDector, SLOT(onPauseStreamDetect()), Qt::QueuedConnection);
	connect(this,SIGNAL(signalResumeStreamDetect()), mStreamDector, SLOT(onResumeStreamDetect()), Qt::QueuedConnection);
	mStreamDector->start();
	emit signalResumeStreamDetect();

	mLiveBufferedCount = 0;
	mLastMSecsSinceEpoch = QDateTime::currentMSecsSinceEpoch();

	mLayoutInfoForPlay = NULL;

	mLiveSourceConfig = LIVE_AutoPlay;

	connect(&language_switch_timer_, SIGNAL(timeout()), this, SLOT(slotLanguageSwitch()));
}

QtSchedule::~QtSchedule()
{
//	std::map<QtTimer *, QtMediaDone *>::iterator itor = mQtTimerDoneMapper.end() ;
//	for(itor = mQtTimerDoneMapper.begin() ;itor!=mQtTimerDoneMapper.end();++itor)
//	{
//		//timer object will be deleted in deleteAllWidgets()
//		DELETE_ALLOCEDRESOURCE(itor->second);
//	}
//	mQtTimerDoneMapper.clear();
	language_switch_timer_.stop();
	releaseMediadone();

	deleteAllWidgets();
	DELETE_ALLOCEDRESOURCE(mLayoutInfoForPlay);

	DELETE_ALLOCEDRESOURCE(mStreamDector);

	releaseLayoutBGPool();
}

void QtSchedule::setResolution(QRect &rect)
{
    QSize size;
    size.setHeight(rect.height());
    size.setWidth(rect.width());

	this->setGeometry(rect.left(), rect.top(), rect.right(), rect.bottom());
	//this->show();
}

void QtSchedule::setRunning(bool running)
{

}

bool QtSchedule::handleMessage(Message *msg)
{
	 switch (msg->mWhat)
	 {
	 case Msg_SwitchMedia:
	 {
		 LogD("--- handleMessage start. \n");
		 onQtMediaDone((QtMediaDone*)(msg->mData));
		 LogD("--- handleMessage end. \n");
		 break;
	 }
	 default:
		 break;
	 }
	return true;
}

void QtSchedule::notifyMediaDone(QtMediaDone *done)
{
	emit signalQtMediaDone(done);
}

void QtSchedule::onQtMediaDone(QtMediaDone *done)
{
	synchronized(mQtTimerDoneMapperMutex)
	{
		if(done == NULL)
		{
			LogE("QtSchedule::onQtMediaDone done == NULL\n");
			return ;
		}

		if(!done->mMediaDoneReleaseFlag)
		{
			//LogD("QtSchedule::onQtMediaDone type:%d\n",done->mMediaType);
			done->mMediaDoneReleaseFlag = true;
		}
		else
		{
			LogE("QtSchedule::onQtMediaDone 1 type:%d\n",done->mMediaType);
			return ;
		}

		bool bUnknownType = false;
		QtTimer *timer = done->mTimer;
		if (done->bPlayStatus)
		{
			//LogD("QtSchedule::onQtMediaDone playstatus:%d\n",done->bPlayStatus);
			switch (done->mMediaType) {
				case Json::MediaBasic::Text:
				{
					done->mMarquee->setDurationTimer(NULL);
					releaseQtMarquee(done->mMarquee);
					done->mMarquee = NULL;
					break;
				}
				case Json::MediaBasic::Image:
				{
					releaseQtImage(done->mImage);
					done->mImage = NULL;
					break;
				}
				case Json::MediaBasic::Video:
				{
					releaseQtVideoPlayer(done->mVideoPlayer);
					done->mVideoPlayer = NULL;
					break;
				}
				case Json::MediaBasic::Live:
				{
					//sendMessage(new Message(2,(void*)done->mVideoPlayer));
//	                if (done->mVideoPlayer != NULL &&
//	                        done->mVideoPlayer->mPlayer != NULL &&
//							g_LCDCtrlerDevStatus.mSoftware.mMode == Json::SoftwareStatus::WorkMode::M_Live)
//	                {
//	                    disconnect(done->mVideoPlayer->mPlayer,
//	                            SIGNAL(mediaStatusChanged(QtAV::MediaStatus)), done,
//	                            SLOT(OnMediaStatusChanged(QtAV::MediaStatus)));
//	                    disconnect(done->mVideoPlayer->mPlayer,
//	                            SIGNAL(positionChanged(qint64)), done,
//	                            SLOT(OnQtPositionChanged(qint64)));
//	                    disconnect(done->mVideoPlayer->mPlayer,
//	                            SIGNAL(videoIsStoped()), done,
//	                            SLOT(OnCheckVideoStop()));
//	                }

					LogD("--- status release QtVideoPlayer\n");
					if(done->mVideoPlayer->mIsLiveMedia == true)
					{
						releaseQtVideoPlayerLive(done->mVideoPlayer);
						LogD("--- releaseQtVideoPlayerLive  live\n");
					}
					else
					{
						releaseQtVideoPlayer(done->mVideoPlayer);
						LogD("--- releaseQtVideoPlayer localmedia\n");
					}
					done->mVideoPlayer = NULL;
					break;
				}
				case Json::MediaBasic::AnalogClock:
				{
					releaseQtFlash(done->mFlash);
					done->mFlash = NULL;
					break;
				}
				case Json::MediaBasic::DigitalClock:
				{
					releaseQtClock(done->mClock);
					done->mClock = NULL;
					break;
				}
				case Json::MediaBasic::Flash:
				{
					releaseQtFlash(done->mFlash);
					done->mFlash = NULL;
					break;
				}
				case Json::MediaBasic::ArrivalMsg:
				{
					done->mArrivalMsg->setPlayStatus(false);
					releaseQtArrivalMsg(done->mArrivalMsg);
					done->mArrivalMsg = NULL;
					break;
				}
				default:
				{
					LogE("--- unknown mediatype 1: %d, playstatus:%d\n",done->mMediaType,done->bPlayStatus);
					bUnknownType = true;
				}
			}
		}
		else
		{
			LogD("QtSchedule::onQtMediaDone playstatus:%d\n",done->bPlayStatus);
			switch (done->mMediaType) {
			case Json::MediaBasic::Text:
			case Json::MediaBasic::Image:
			case Json::MediaBasic::Video:
			case Json::MediaBasic::Live:
			case Json::MediaBasic::AnalogClock:
			case Json::MediaBasic::DigitalClock:
			case Json::MediaBasic::Flash:
			case Json::MediaBasic::ArrivalMsg:
			{
				LogD("QtSchedule::onQtMediaDone playstatus:%d 1\n",done->bPlayStatus);
				break;
			}
			default:
			{
				LogE("--- unknown mediatype 2: %d, playstatus:%d\n",done->mMediaType,done->bPlayStatus);
				bUnknownType = true;
			}
		}
		}

		if(bUnknownType)
		{
			LogE("--- unknown mediatype: %d\n",done->mMediaType);
			return;
		}

		if (timer != NULL) {
			if(timer->mValidDuration)
			{
				//LogD("--- timer stop and disconnect. \n");
				timer->stop();
				disconnect(timer, SIGNAL(timeout()), done, SLOT(onQtMediaDone()));
			}

			//LogD("--- releaseQtTimer \n");
			releaseQtTimer(timer);
		}

		emit signalPlayNextContent(done->pPartation,done->pContent);

		if(done != NULL)
		{
			//LogD("--- delete done object \n");
			done->setDefaultValue();
			//delete done;
			releaseMediaDone(done);
			//done = NULL;
		}
	}
}

bool QtSchedule::setPlayLayoutInfo(void* data)
{
	Json::LayoutInfo4Qt* fullLayoutInfo = (Json::LayoutInfo4Qt*)data;

	releaseMediadone();
	releaseUsedWidgets();
	releaseLayoutBGPool();
	//object created in transmanager module
	DELETE_ALLOCEDRESOURCE(mLayoutInfoForPlay);
	if (fullLayoutInfo != NULL)
	{
		mLayoutInfoForPlay = fullLayoutInfo;
		if (language_switch_timer_.isActive())
		{
			language_switch_timer_.stop();
		}
		language_switch_timer_.setInterval(mLayoutInfoForPlay->layoutDtl.ch_en_switch_ * 1000);
		language_switch_timer_.start();
		emit signalStartPlaynewLayout();
	}
	else
	{
		LogD("############################################### fullLayoutInfo == NULL\n");
		mLayoutInfoForPlay = nullptr;
		deleteAllWidgets();
	}
	
	return true;
}

void QtSchedule::setRTArrivalMsgInfo(void* data)
{
	Json::ArrivalDetail* arrmsg = static_cast<Json::ArrivalDetail*>(data);
//	if(arrmsg == NULL)
//	{
//		return ;
//	}

	synchronized(mQtTimerDoneMapperMutex)
	{
		std::map<QtTimer *, QtMediaDone *>::iterator itor = mQtTimerDoneMapper.end() ;
		for(itor = mQtTimerDoneMapper.begin() ;itor!=mQtTimerDoneMapper.end();++itor)
		{
			QtMediaDone * temp = itor->second;
			if(temp != NULL && temp->mMediaType == Json::MediaBasic::ArrivalMsg)
			{
				if(temp->mArrivalMsg == NULL)
				{
					LogD("------- onRTArrMsgUpdated ,temp->mArrivalMsg == NULL \n ");
					continue;
				}

				if(!(temp->mArrivalMsg->getPlayStatus()))
					continue;

				LogD("------- setRTArrivalMsgInfo , set setRTArrMsgFile\n ");
				temp->mArrivalMsg->setRTArrMsgFile(arrmsg);
				temp->mArrivalMsg->setRunning(true);
			}
		}
	}

	DELETE_ALLOCEDRESOURCE(arrmsg);

}

void QtSchedule::setTrainTimeInfo(void* data)
{
	Json::TrainTimeDetail* traintime = static_cast<Json::TrainTimeDetail*>(data);
	if(traintime == NULL)
	{
		LogE("------- setTrainTimeInfo ,traintime == NULL.\n ");
		return ;
	}

	synchronized(mQtTimerDoneMapperMutex)
	{
		std::map<QtTimer *, QtMediaDone *>::iterator itor = mQtTimerDoneMapper.end() ;
		for(itor = mQtTimerDoneMapper.begin() ;itor!=mQtTimerDoneMapper.end();++itor)
		{
			QtMediaDone * temp = itor->second;
			if(temp != NULL && temp->mMediaType == Json::MediaBasic::ArrivalMsg)
			{
				if(temp->mArrivalMsg == NULL)
				{
					LogD("------- setTrainTimeInfo ,temp->mArrivalMsg == NULL \n ");
					continue;
				}

				if(!(temp->mArrivalMsg->getPlayStatus()))
				{
					LogD("------- setTrainTimeInfo ,mArrivalMsg->getPlayStatus() == false \n ");
					continue;
				}

				LogD("------- setTrainTimeInfo , set TrainTimeInfo\n ");
				temp->mArrivalMsg->setTrainTimeInfo(traintime);
				temp->mArrivalMsg->setRunning(true);
			}
		}
	}

	DELETE_ALLOCEDRESOURCE(traintime);
}

void QtSchedule::setLiveSourceInfo(int playsource)
{
	synchronized(mQtTimerDoneMapperMutex)
	{
		mLiveSourceConfig = (LiveSourceSwitchConfig)playsource;

		std::map<QtTimer *, QtMediaDone *>::iterator itor = mQtTimerDoneMapper.end() ;
		for(itor = mQtTimerDoneMapper.begin() ;itor!=mQtTimerDoneMapper.end();++itor)
		{
			QtMediaDone * temp = itor->second;
			if(temp != NULL && temp->mMediaType == Json::MediaBasic::Live)
			{
				if(temp->mVideoPlayer == NULL)
				{
					LogD("------- setLiveSourceInfo ,temp->mVideoPlayer == NULL \n ");
					continue;
				}

				mLiveSourceConfig = (LiveSourceSwitchConfig)playsource;
				setStreamStatus(true);
				sendMessage(new Message(QtSchedule::Msg_SwitchMedia,temp));
				break;
			}
		}

	}

}

void QtSchedule::setStreamStatus(bool status)
{
	if(mStreamDector != NULL)
	{
		mStreamDector->setStreamStatus(status);
	}
	else
	{
		LogD("mStreamDector == NULL\n");
	}
}


void QtSchedule::setOPSFullScreen(OPSMsgParam* currOps)
{
	currOps->mFont = QFont(mLayoutInfoForPlay->layoutDtl.emer_.fullScreen.emer_font.name.c_str(), mLayoutInfoForPlay->layoutDtl.emer_.halfScreen.emer_font.size * 0.65);
	currOps->mFont.setItalic(mLayoutInfoForPlay->layoutDtl.emer_.fullScreen.emer_font.italic);
	currOps->mFont.setBold(mLayoutInfoForPlay->layoutDtl.emer_.fullScreen.emer_font.bold);
	currOps->mSpeed = mLayoutInfoForPlay->layoutDtl.emer_.fullScreen.speed;
}

bool QtSchedule::setOPSHalfScreen(bool status,int& x ,int& y ,int& w ,int& h, OPSMsgParam* currOps)
{
	bool bFind = false;
	std::map<QtTimer *, QtMediaDone *>::iterator itor = mQtTimerDoneMapper.end() ;
	synchronized(mQtTimerDoneMapperMutex)
	{
		if (mLayoutInfoForPlay != NULL)
		{
			if (currOps != nullptr)
			{
				currOps->mFont = QFont(mLayoutInfoForPlay->layoutDtl.emer_.halfScreen.emer_font.name.c_str(), mLayoutInfoForPlay->layoutDtl.emer_.halfScreen.emer_font.size * 0.65);
				currOps->mFont.setItalic(mLayoutInfoForPlay->layoutDtl.emer_.halfScreen.emer_font.italic);
				currOps->mFont.setBold(mLayoutInfoForPlay->layoutDtl.emer_.halfScreen.emer_font.bold);
				currOps->mSpeed = mLayoutInfoForPlay->layoutDtl.emer_.halfScreen.speed;
			}

			for (std::vector<Json::PartitionDetail>::iterator iter = mLayoutInfoForPlay->layoutDtl.mPartitions.begin();
				iter != mLayoutInfoForPlay->layoutDtl.mPartitions.end(); ++iter)
			{
				//LogD("-----------------------------  parid:%d,  opsfalg\n",iter->mId)
				if (iter->mIsMaster && status)
				{
					bFind = true;
					x = iter->mXpos;
					y = iter->mYpos;
					w = iter->mWidth;
					h = iter->mHeight;

					break;
				}
			}
		}
	}

	return bFind;
}

bool QtSchedule::setOPSPartation(bool status,int& x,int& y,int& w,int& h, OPSMsgParam* currOps)
{
	auto it = mLayoutInfoForPlay->mPartitonInfos.find(301);
	Json::MediaText* media_text = dynamic_cast<Json::MediaText*>(it->second.begin()->second);
	if (nullptr != media_text && nullptr != currOps)
	{
		currOps->mFont = QFont(media_text->mParams.mFont.mName.c_str(), 80);
		currOps->mFont.setItalic(media_text->mParams.mFont.mIsItalic);
		currOps->mFont.setBold(media_text->mParams.mFont.mIsBold);
		currOps->mSpeed = media_text->mParams.mPixelPerSecond;
	}

	bool bFind = false;
	std::map<QtTimer *, QtMediaDone *>::iterator itor = mQtTimerDoneMapper.end() ;
	synchronized(mQtTimerDoneMapperMutex)
	{
		if(mLayoutInfoForPlay != NULL)
		for(std::vector<Json::PartitionDetail>::iterator iter = mLayoutInfoForPlay->layoutDtl.mPartitions.begin();
				iter != mLayoutInfoForPlay->layoutDtl.mPartitions.end();++iter)
		{
			//LogD("-----------------------------  parid:%d,  opsfalg\n",iter->mId)
			if(iter->mOPSflag && status)
			{
				bFind = true;
				x = iter->mXpos;
				y = iter->mYpos;
				w = iter->mWidth;
				h = iter->mHeight;

				break;
			}
		}

		/*for(itor = mQtTimerDoneMapper.begin() ;itor!=mQtTimerDoneMapper.end();++itor)
		{
			if(itor->second && itor->second->isOPSPartation())
			{
				if(itor->second->pPartation != NULL && itor->second->mMediaType == Json::MediaBasic::Text)
				{
					bFind = true;
					if(status)
					{
						//freeze partation playing
						itor->second->mMarquee->setRunning(false);

						x = itor->second->pPartation->mXpos;
						y = itor->second->pPartation->mYpos;
						w = itor->second->pPartation->mWidth;
						h = itor->second->pPartation->mHeight;
					}
					else
					{
						itor->second->mMarquee->setRunning(true);
					}

					break;
				}
				else
				{
					LogD("Please set OPSFlag in text partation.\n");
				}

			}
		}*/
	}

	return bFind;
}


string QtSchedule::getBackImage()
{
	return mLayoutInfoForPlay->layoutDtl.back_image_;
}

void QtSchedule::slotStartPlaynewLayout()
{

		LogD("--------------  start to play layout.\n");

		releaseLayoutBGPool();
		mLayoutInfoForPlay->layoutDtl.sortPartationByZOrder();
		for (std::vector<Json::PartitionDetail>::iterator iter = mLayoutInfoForPlay->layoutDtl.mPartitions.begin();
			iter != mLayoutInfoForPlay->layoutDtl.mPartitions.end(); ++iter)
		{
			LogD("slotStartPlaynewLayout - partationid:%d\n", iter->mId);
			Json::PartitionDetail* partation = &(*iter);

			Json::MediaBasic* mediaContent = NULL;
			if (!findMediaBaise(mLayoutInfoForPlay->mPartitonInfos[iter->mId], mediaContent, NULL))
			{
				LogE("slotStartPlaynewLayout - get media failed!\n");
				mediaContent = NULL;
				//continue;
			}

			if (partation != NULL && partation->mBkgroudFile.size() > 0
				&& QFile::exists((mMediasPath + partation->mBkgroudFile).c_str()))
			{
				QLabel* label = new QLabel(this);
				QRect 	rect;
				rect.setLeft(partation->mXpos);
				rect.setTop(partation->mYpos);
				rect.setBottom(partation->mHeight);
				rect.setRight(partation->mWidth);
				label->setGeometry(rect);
				label->setScaledContents(true);

				LogD("start show background file:%s.\n", (mMediasPath + partation->mBkgroudFile).c_str());
				label->setPixmap(QPixmap((mMediasPath + partation->mBkgroudFile).c_str()));

				label->show();
				label->raise();

				mQtLayoutBGFilePool.push_back(label);
			}

			if (partation != NULL && mediaContent != NULL)
			{
				play(partation, mediaContent);
			}
			else if (partation == NULL)
			{
				LogD("slotStartPlaynewLayout - partation == NULL \n");
			}

		}
}

void QtSchedule::slotPlayNextContent(Json::PartitionDetail *pPartation, Json::MediaBasic* pContent)
{
	//LogD("--------- Partation-%d to play next content.\n",pPartation->mId);

	Json::LayoutInfo4Qt::MediaContents& mediaContents = mLayoutInfoForPlay->mPartitonInfos[pPartation->mId];

	Json::MediaBasic* mediaContent = NULL;
	if(!findMediaBaise(mediaContents,mediaContent,pContent))
	{
		LogE("slotPlayNextContent - get next content failed!\n");
		return ;
	}

	if (pPartation != NULL && mediaContent != NULL)
	{
		play(pPartation, mediaContent);
	}
	else
	{
		LogE("pPartation == NULL || mediaContent == NULL\n");
	}
}

void QtSchedule::onProbeStreamReport(bool dc)
{
	//LogD("############ onProbeStreamReport ############ oldstatus:%d,currstatus:%d\n",mStreamDector->getStreamStatus(),dc);
	if(mStreamDector->getStreamStatus() != dc)
	{
		mStreamDector->setStreamStatus(dc);
	}
	else
	{
		return ;
	}

	LogD("############ onProbeStreamReport ############ oldstatus:%d,currstatus:%d\n",mStreamDector->getStreamStatus(),dc);
	synchronized(mQtTimerDoneMapperMutex)
	{
		std::map<QtTimer *, QtMediaDone *>::iterator itor = mQtTimerDoneMapper.end() ;
		for(itor = mQtTimerDoneMapper.begin() ;itor!=mQtTimerDoneMapper.end();)
		{
			QtMediaDone * temp = itor->second;
			++itor; // -->  ++itor should put here, because onQtMediaDone() may earse mQtTimerDoneMapper
			if(temp != NULL && temp->mMediaType == Json::MediaBasic::Live )
			{
				if(!temp->getSwitchmediaMsg())
				{
					temp->setSwitchmediaMsg(true);
					temp->mTimer->setRunning(false);
					if(hasMessage(QtSchedule::Msg_SwitchMedia) > 0)
						removeMessage(Msg_SwitchMedia);
					sendMessage(new Message(Msg_SwitchMedia,(void*)temp),1000);
					//temp->onQtMediaDone();
				}

			}
		}
	}
}


void QtSchedule::slotLanguageSwitch()
{
	synchronized(mQtTimerDoneMapperMutex)
	{
		std::map<QtTimer*, QtMediaDone*>::iterator itor = mQtTimerDoneMapper.end();
		for (itor = mQtTimerDoneMapper.begin(); itor != mQtTimerDoneMapper.end(); ++itor)
		{
			QtMediaDone* temp = itor->second;
			if (temp != NULL && temp->mMediaType == Json::MediaBasic::ArrivalMsg)
			{
				if (temp->mArrivalMsg == NULL)
				{
					//LogD("------- setTrainTimeInfo ,temp->mArrivalMsg == NULL \n ");
					continue;
				}

				if (!(temp->mArrivalMsg->getPlayStatus()))
				{
					//LogD("------- setTrainTimeInfo ,mArrivalMsg->getPlayStatus() == false \n ");
					continue;
				}

				//LogD("------- setTrainTimeInfo , set TrainTimeInfo\n ");
				temp->mArrivalMsg->setRunning(true, cn_flag_);
			}
			else if (temp != NULL && temp->mMediaType == Json::MediaBasic::DigitalClock)
			{
				temp->mClock->setRunning(true, cn_flag_);
			}
			else if (temp != NULL && temp->mMediaType == Json::MediaBasic::Text)
			{
				temp->mMarquee->setRunning(true, false, cn_flag_);
			}
		}
	}
	cn_flag_ = !cn_flag_;
}

bool QtSchedule::findPartationByID(int id,Json::LayoutDetail& layout, Json::PartitionDetail*& partation)
{
	bool bFind = false;
	for(std::vector<Json::PartitionDetail>::iterator i = layout.mPartitions.begin();
			i != layout.mPartitions.end();++i)
	{
		if((*i).mId == id)
		{

			partation = &(*i);
			bFind = true;
		}
	}

	return bFind;
}

bool QtSchedule::findMediaBaise(Json::LayoutInfo4Qt::MediaContents & contents,Json::MediaBasic*& content,Json::MediaBasic* pPrevContent)
{
	bool bFind = false;
	Json::LayoutInfo4Qt::MediaContents::iterator iter = contents.begin();
	for(iter = contents.begin();iter != contents.end();++iter)
	{
		if(pPrevContent == NULL)
		{
			content = iter->second;
			bFind = true;
			break;
		}
		else
		{
			if(iter->first == pPrevContent->mId)
			{
				if(++iter == contents.end())
				{
					content = contents.begin()->second;
				}
				else
				{
					content = iter->second;
				}
				bFind = true;
				break;
			}
		}

	}

	return bFind;
}

bool QtSchedule::play(Json::PartitionDetail *pPartation, Json::MediaBasic* pContent)
{
//	if(NULL == pPartation || NULL == pContent)
//	{
//		return false;
//	}

	int llpx = pPartation->mXpos;
	int llpy = pPartation->mYpos;
	int llpw = pPartation->mWidth;
	int llph = pPartation->mHeight;

	QtTimer *timer = obtainQtTimer();
	//QtMediaDone *done = new QtMediaDone();
	QtMediaDone *done = obtainQtMediaDone();
	done->mQtSchedule = this;
	//done->pMedias = pMedias;
	done->pPartation = pPartation;
	done->pContent = pContent;
	done->mMediaType = pContent->mType;
	done->mTimer = timer;
	done->setOPSPartation(pPartation->mOPSflag);

	int duration = 0;
	switch (pContent->mType) {
		case Json::MediaBasic::Text:
		{
			Json::MediaText* mediatext = (Json::MediaText*)(pContent);
			if (mediatext != NULL)
			{
				duration = playMarquee(done,mediatext,llpx,llpy,llpw,llph,mMediasPath);
				//done->mMarquee->setDurationTimer(timer);
				done->mMarquee->setQtMediaDone(done);
				done->bPlayStatus = true;
			}
			else
			{
				done->bPlayStatus = false;
			}

			break;
		}
		case Json::MediaBasic::Image:
		{
			Json::MediaImage* mediaImage = dynamic_cast<Json::MediaImage*>(pContent);
			if(mediaImage != NULL && FileSysUtils::Accessible(mMediasPath + mediaImage->mFile, FileSysUtils::FR_OK))
			{
				duration = playImage(done,mediaImage,llpx,llpy,llpw,llph,mMediasPath);
				done->bPlayStatus = true;
			}
			else
			{
				done->bPlayStatus = false;
			}
			break;
		}
		case Json::MediaBasic::Video:
		{
			Json::MediaVideo* mediavideo = dynamic_cast<Json::MediaVideo*>(pContent);
			std::string pathfile = mMediasPath + mediavideo->mFile;
			if(mediavideo != NULL && FileSysUtils::Accessible(pathfile, FileSysUtils::FR_OK))
			{
				duration = playVideo(done,mediavideo,llpx,llpy,llpw,llph,mMediasPath);
				done->bPlayStatus = true;
			}
			else
			{
				done->bPlayStatus = false;
			}

			g_LCDCtrlerDevStatus.mSoftware.mMode = Json::SoftwareStatus::WorkMode::M_Local;
			break;
		}
		case Json::MediaBasic::Live:
		{
			Json::MediaLive* medialive = dynamic_cast<Json::MediaLive*>(pContent);
			if(medialive != NULL )
			{
				duration = playLive(done,medialive,llpx,llpy,llpw,llph,mMediasPath);
				done->bPlayStatus = true;
			}
			else
			{
				done->bPlayStatus = false;
			}

			break;
		}
		case Json::MediaBasic::ArrivalMsg:
		{
			Json::MediaArrivalMsg* mediaarrmsg = dynamic_cast<Json::MediaArrivalMsg*>(pContent);
			if(mediaarrmsg != NULL)
			{
				duration = playArrivalMsg(done,mediaarrmsg,llpx,llpy,llpw,llph,mMediasPath);
				mLCDController->removeMessage(LCDController::ArriMsgBlockDisplayed);
				mLCDController->sendMessage(new Message(LCDController::ArriMsgBlockDisplayed),200);
				done->bPlayStatus = true;
			}
			else
			{
				done->bPlayStatus = false;
			}

			break;
		}
		case Json::MediaBasic::Flash:
		{
			Json::MediaFlash* mediaflash = dynamic_cast<Json::MediaFlash*>(pContent);
			if ( mediaflash!= NULL && FileSysUtils::Accessible(mMediasPath + mediaflash->mFile, FileSysUtils::FR_OK))
			{
				duration = playFlash(done,mediaflash,llpx,llpy,llpw,llph,mMediasPath);
				done->bPlayStatus = true;
			}
			else
			{
				done->bPlayStatus = false;
			}

			break;
		}
		case Json::MediaBasic::AnalogClock:
		{
			Json::MediaFlash* mediaflash = dynamic_cast<Json::MediaFlash*>(pContent);
			if ( mediaflash!= NULL && FileSysUtils::Accessible(mMediasPath + mediaflash->mFile, FileSysUtils::FR_OK))
			{
				duration = playFlash(done,mediaflash,llpx,llpy,llpw,llph,mMediasPath);
				done->bPlayStatus = true;
			}
			else
			{
				done->bPlayStatus = false;
			}
			break;
		}
		case Json::MediaBasic::DigitalClock:
		{
			Json::MediaDigitalClock* mediaaclock = dynamic_cast<Json::MediaDigitalClock*>(pContent);
			if(mediaaclock != NULL)
			{
				duration = playClock(done,mediaaclock,llpx,llpy,llpw,llph,mMediasPath);
				done->bPlayStatus = true;
			}
			else
			{
				done->bPlayStatus = false;
			}

			break;
		}
		default:
		{

			break;
		}
	}

	synchronized(mQtTimerDoneMapperMutex)
	{
		mQtTimerDoneMapper[timer] = done;
	}

	if(duration > 0)
	{
		timer->mValidDuration = true;
		connect(timer, SIGNAL(timeout()), done, SLOT(onQtMediaDone()));
		timer->start((int)(duration * 1000)); //msec
	}
	else
	{
		timer->mValidDuration = false;
	}

	return true;
}

int QtSchedule::playMarquee(QtMediaDone*& done,Json::MediaText*& mediatext,
		int x,int y,int w,int h,const std::string respath)
{
	int duration = -1;
	QtMarquee *marquee = obtainQtMarquee();

	LogD("Play Text : %s, effect : %d\n", mediatext->mParams.mContent.c_str(),(int)(mediatext->mParams.mEffect));
	marquee->setText(mediatext->mParams.mContent.c_str());
	marquee->setTextEn(mediatext->mParams.mContentEn.c_str());
	marquee->setFont(mediatext->mParams.mFont.mName.c_str(), mediatext->mParams.mFont.mSize);
	marquee->setFontEn(mediatext->mParams.mFontEn.mName.c_str(),mediatext->mParams.mFontEn.mSize);
	//marquee->setFont(mediatext->mParams.mFont.mName.c_str(),70);
	marquee->setColor(mediatext->mParams.mForeColor, mediatext->mParams.mBackColor);
	marquee->setBGFile(respath + mediatext->mParams.mBackImage);
	//marquee->setSpeed(mediatext->mParams.mSpeed);
	marquee->setPixelPerSecond(mediatext->mParams.mPixelPerSecond);
	marquee->setEffort(mediatext->mParams.mEffect);
	//marquee->setEffort(3);
	marquee->setAlign(mediatext->mParams);
	marquee->setGeometry(x, y, w, h);
	marquee->setRunning(true, false);

	done->mMarquee = marquee;

	return duration;
}

int QtSchedule::playImage(QtMediaDone*& done,Json::MediaImage*& mediaimage,
		int x,int y,int w,int h,const std::string respath)
{
	int duration = -1;
	QtImage *image = obtainQtImage();

	image->setGeometry(x, y, w, h);
	image->setFrontFile(respath + mediaimage->mFile);
	image->setRunning(true);

	done->mImage = image;
	duration = mediaimage->mDuration;
	return duration;
}

int QtSchedule::playVideo(QtMediaDone*& done,Json::MediaVideo*& mediavideo,
		int x,int y,int w,int h,const std::string respath)
{
	int duration = -1;

	std::string videofile = respath + mediavideo->mFile;
	LogD("Play Video : %s\n", videofile.c_str());
	QtVideoPlayer *player = obtainQtVideoPlayer();
	player->mIsLiveMedia = false;

	done->mVideoPlayer = player;
	//player->setMute(llp->mIsSoundable == false);

	player->setGeometry(x, y, w, h);
	player->mPlayer->setInterruptOnTimeout(false);
	player->play(videofile.c_str(),false,"");
	connect(player->mPlayer, SIGNAL(stopped()), done, SLOT(onQtMediaDone()));
	duration = mediavideo->mDuration;

	return duration;
}

int QtSchedule::playLive(QtMediaDone*& done,Json::MediaVideo*& medialive,
		int x,int y,int w,int h,const std::string respath)
{
	int duration = -1;

	const ConfigParser* config = mLCDController->getConfig();
	//add by guo,202208��������StreamDetector��ip
	QString strUrl(medialive->mUrl.c_str());
	if(strUrl.startsWith("udp://"))
	{
		strUrl.remove(0,6);
		string live_ip="225.0.0.1";
		int live_port=65123;
		if(strUrl.contains(':') && !strUrl.startsWith("0.0.0.0:"))
		{
			live_ip=strUrl.split(':')[0].toStdString();
			live_port=strUrl.split(':')[1].toInt();
		}
		if (mStreamDector->SetIpPort(live_ip, live_port))
		{
			mStreamDector->pauseDetect();
			sleep(1);
			mStreamDector->resumeDetect();
		}
		//emit signalPauseStreamDetect();
	}

	//add by guo,202208��������StreamDetector��ip

	if(mStreamDector->getStreamStatus() && mLiveSourceConfig != LIVE_PlayLocal && !medialive->mUrl.empty())
	{
		mLiveBufferedCount = 0;
		mLastMSecsSinceEpoch = QDateTime::currentMSecsSinceEpoch();

		QtVideoPlayer *player = obtainQtVideoPlayerLive();
		player->mIsLiveMedia = true;
		player->setMute(mBUsedWidgetsFreeze == true);
		player->setGeometry(x, y, w, h);
		done->mVideoPlayer = player;

		std::string liveurl = medialive->mUrl;
		liveurl += "?fifo_size=";
		if(config->mLiveStreamfifosize.size() == 0)
			liveurl += "10000";
		else
			liveurl += config->mLiveStreamfifosize;
		liveurl += "&overrun_nonfatal=1";

		// Pause the detect thread
		emit signalPauseStreamDetect();
		LogD("Play Live : %s\n", liveurl.c_str());
		g_LCDCtrlerDevStatus.mSoftware.mMode = Json::SoftwareStatus::WorkMode::M_Live;

		connect(player->mPlayer, SIGNAL(mediaStatusChanged(QtAV::MediaStatus)), done, SLOT(onmediaStatusChanged(QtAV::MediaStatus)));
		connect(player->mPlayer, SIGNAL(positionChanged(qint64)), done, SLOT(onQtPositionChanged(qint64)));
	    connect(player->mPlayer, SIGNAL(videoIsStoped()), done, SLOT(OnCheckVideoStop()));

		if(config->mIsVideoSyncServer != 0 && config->mVideoSyncServerIP.size() > 0)
		{
			LogD("live video sysc server, clientIP:%s\n", config->mVideoSyncServerIP.c_str());
			player->play(liveurl.c_str(),true,config->mVideoSyncServerIP);
		}
		else
		{
			LogD("live video sysc client\n");
			player->play(liveurl.c_str(),true,"");
		}

		done->checkTimerActiveForLive();
		//duration = medialive->mDuration;
	}
	else
	{
		LogD("StreamStatus=%d LiveSourceConfig=%d Url=%s\n",
			mStreamDector->getStreamStatus(), mLiveSourceConfig, medialive->mUrl.c_str());
		LogD("live stream exception, play local media\n");

		QtVideoPlayer *player = obtainQtVideoPlayer();
		player->mIsLiveMedia = false;
		player->setMute(mBUsedWidgetsFreeze == true);
		player->setGeometry(x, y, w, h);
		done->mVideoPlayer = player;

		std::string videofile = respath + medialive->mFile;
		mStreamDector->setStreamStatus(false);
		g_LCDCtrlerDevStatus.mSoftware.mMode = Json::SoftwareStatus::WorkMode::M_Local;
		LogD("Play Local : %s\n", videofile.c_str());
		player->play(videofile,false,"");
		duration = 6000000;

		if(mLiveSourceConfig != LIVE_PlayLocal && !medialive->mUrl.empty())
			emit signalResumeStreamDetect();

		connect(player->mPlayer, SIGNAL(stopped()), done, SLOT(onQtMediaDone()));
	}

	return duration;
}

int QtSchedule::playArrivalMsg(QtMediaDone*& done,Json::MediaArrivalMsg* mediaarrmsg,
				int x,int y,int w,int h,const std::string respath)
{
	int duration = -1;
	const ConfigParser* config = mLCDController->getConfig();

	QtArrivalMsg *qtarrivalmsg = obtainQtArrivalMsg();

	qtarrivalmsg->setStationID(config->mStationId);
	qtarrivalmsg->setBlocks(mediaarrmsg->mParams);
	//qtarrivalmsg->setBGFile("/home/1.PNG");
	//qtarrivalmsg->setRTArrMsgFile(mSchedule->getRTArrivalMsg());
	qtarrivalmsg->setRunning(true);

	qtarrivalmsg->setGeometry(x, y, w, h);
	done->mArrivalMsg = qtarrivalmsg;

	qtarrivalmsg->setPlayStatus(true);
	return duration;
}

int QtSchedule::playFlash(QtMediaDone*& done,Json::MediaFlash* mediaflash,
			int x,int y,int w,int h,const std::string respath)
{
	int duration = -1;
	QtFlash *flash = obtainQtFlash();
	std::string flashfile = respath + mediaflash->mFile;
	flash->setGeometry(x, y, w, h);
	//flash->setZoomFactor(zoom);
	flash->play(flashfile, w, h,  0);

	done->mFlash = flash;

	duration = mediaflash->mDuration;
	return duration;
}

int QtSchedule::playClock(QtMediaDone*& done,Json::MediaDigitalClock* mediaaclock,
			int x,int y,int w,int h,const std::string respath)
{
	int duration = -1;
	QtClock *clock = obtainQtClock();

	clock->setBlocks(mediaaclock->mParams);

//	clock->setClockType(QtClock::Digital);
//	clock->setUse24(true);
//
//	Json::LabelInfo label = *(mediaaclock->mParams.begin());
//	clock->setColor(strtoul(label.mVarText.mText.mForeColor.c_str(), NULL, 16),
//			strtoul(label.mVarText.mText.mBackColor.c_str(), NULL, 16));
//	clock->setBGFile(respath + label.mVarText.mText.mBackImage);
//	clock->setAlign(label.mVarText.mText.mAlign);
//	clock->setFont(QFont(label.mVarText.mText.mFont.mName.c_str(),label.mVarText.mText.mFont.mSize));
//	clock->setOneline(false);

	clock->setRunning(true);

	clock->setGeometry(x, y, w, h);

	done->mClock = clock;

	return duration;
}

QtTimer *QtSchedule::obtainQtTimer()
{
	QtTimer *timer = NULL;

	if (mQtTimerPool.size() == 0) {
		timer = new QtTimer(this);
	}
	else {
		timer = mQtTimerPool.back();
		mQtTimerPool.pop_back();
	}

	mQtTimerInUse.push_back(timer);
	return timer;
}
void QtSchedule::releaseQtTimer(QtTimer *timer)
{
	if(timer == NULL )
		return ;

	if(timer->mValidDuration)
	{
		timer->stop();
		timer->disconnect();
	}

	//delete mQtTimerSigMapper[timer];
	mQtTimerDoneMapper.erase(timer);

	mQtTimerInUse.remove(timer);

	if (mQtTimerPool.size() >= MAX_TIMER_POOL_SIZE) {
		delete timer;
		timer = NULL;
	}
	else {
		mQtTimerPool.push_back(timer);
	}
}

QtMarquee *QtSchedule::obtainQtMarquee()
{
	QtMarquee *marquee = NULL;

	if (mQtMarqueePool.size() == 0) {
		marquee = new QtMarquee(this);
	}
	else {
		marquee = mQtMarqueePool.back();
		mQtMarqueePool.pop_back();
	}

	marquee->show();
	marquee->raise();
	mQtMarqueeInUse.push_back(marquee);
	return marquee;
}
void QtSchedule::releaseQtMarquee(QtMarquee *marquee)
{
	if(marquee == NULL )
		return ;
	marquee->setRunning(false);
	mQtMarqueeInUse.remove(marquee);

	if (mQtMarqueePool.size() >= MAX_MARQUEE_POOL_SIZE) {
		delete marquee;
	}
	else {
		mQtMarqueePool.push_back(marquee);
	}
}

QtImage *QtSchedule::obtainQtImage()
{
	QtImage *image = NULL;

	if (mQtImagePool.size() == 0) {
		image = new QtImage(this);
	}
	else {
		image = mQtImagePool.back();
		mQtImagePool.pop_back();
	}

	image->show();
	image->raise();
	mQtImageInUse.push_back(image);
	return image;
}
void QtSchedule::releaseQtImage(QtImage *image)
{
	if(image == NULL )
		return ;

	image->hide();
	image->releaseMove();
	mQtImageInUse.remove(image);

	if (mQtImagePool.size() >= MAX_IMAGE_POOL_SIZE) {
		DELETE_ALLOCEDRESOURCE(image);
	}
	else {
		mQtImagePool.push_back(image);
	}
}

QtVideoPlayer *QtSchedule::obtainQtVideoPlayer()
{
	QtVideoPlayer *player = NULL;

	if (mQtVideoPlayerPool.size() == 0) {
		player = new QtVideoPlayer(this);
	}
	else {
		player = mQtVideoPlayerPool.back();
		mQtVideoPlayerPool.pop_back();
	}

	player->show();
	if(!mBUsedWidgetsFreeze)
		player->raise();

	mQtVideoPlayerInUse.push_back(player);

	return player;
}

void QtSchedule::releaseQtVideoPlayer(QtVideoPlayer *player)
{

	if(player == NULL || player->mPlayer == NULL)
		return ;

	player->mPlayer->disconnect();
	player->stop();
	player->hide();

	mQtVideoPlayerInUse.remove(player);
	if (mQtVideoPlayerPool.size() >= MAX_PLAYER_POOL_SIZE) {
		delete player;
		player = NULL;
	}
	else {
		mQtVideoPlayerPool.push_back(player);
	}
}

QtVideoPlayer *QtSchedule::obtainQtVideoPlayerLive()
{
	QtVideoPlayer *player = NULL;

	if (mQtVideoPlayerLivePool.size() == 0) {
		player = new QtVideoPlayer(this);
	}
	else {
		player = mQtVideoPlayerLivePool.back();
		mQtVideoPlayerLivePool.pop_back();
	}

	player->show();
	if(!mBUsedWidgetsFreeze)
		player->raise();

	mQtVideoPlayerLiveInUse.push_back(player);

	return player;
}

void QtSchedule::releaseQtVideoPlayerLive(QtVideoPlayer *player)
{

	if(player == NULL || player->mPlayer == NULL)
		return ;

	player->mPlayer->disconnect();
	player->stop();
	player->hide();

	mQtVideoPlayerLiveInUse.remove(player);
	if (mQtVideoPlayerLivePool.size() >= MAX_PLAYER_LIVE_POOL_SIZE) {
		delete player;
		player = NULL;
	}
	else {
		mQtVideoPlayerLivePool.push_back(player);
	}
}

QtArrivalMsg *QtSchedule::obtainQtArrivalMsg()
{
	QtArrivalMsg *arrivalmsg = NULL;

	if (mQtArrivalMsgPool.size() == 0) {
		arrivalmsg = new QtArrivalMsg(this);
	}
	else {
		arrivalmsg = mQtArrivalMsgPool.back();
		mQtArrivalMsgPool.pop_back();
	}

	arrivalmsg->show();
	arrivalmsg->raise();
	mQtArrivalMsgInUse.push_back(arrivalmsg);
	return arrivalmsg;
}
void QtSchedule::releaseQtArrivalMsg(QtArrivalMsg *arrivalmsg)
{
	if(arrivalmsg == NULL )
		return ;

	arrivalmsg->hide();
	arrivalmsg->setRunning(false);
	mQtArrivalMsgInUse.remove(arrivalmsg);

	if (mQtArrivalMsgPool.size() >= MAX_ARRIVALMSG_POOL_SIZE) {
		delete arrivalmsg;
	}
	else {
		mQtArrivalMsgPool.push_back(arrivalmsg);
	}
}

QtFlash *QtSchedule::obtainQtFlash()
{
	QtFlash *flash = NULL;

	if (mQtFlashPool.size() == 0) {
		flash = new QtFlash(this);
	}
	else {
		flash = mQtFlashPool.back();
		mQtFlashPool.pop_back();
	}

	flash->show();
	flash->raise();
	mQtFlashInUse.push_back(flash);
	return flash;
}

void QtSchedule::releaseQtFlash(QtFlash *flash)
{
	if(flash == NULL )
		return ;

	flash->setRunning(false);
	mQtFlashInUse.remove(flash);

	if (mQtFlashPool.size() >= MAX_FLASH_POOL_SIZE) {
		delete flash;
	}
	else {
		mQtFlashPool.push_back(flash);
	}
}

QtClock *QtSchedule::obtainQtClock()
{
	QtClock *clock = NULL;

	if (mQtClockPool.size() == 0) {
		clock = new QtClock(this);
	}
	else {
		clock = mQtClockPool.back();
		mQtClockPool.pop_back();
	}

	clock->show();
	clock->raise();
	mQtClockInUse.push_back(clock);
	return clock;
}

void QtSchedule::releaseQtClock(QtClock *clock)
{
	if(clock == NULL )
		return ;

	clock->setRunning(false);
	mQtClockInUse.remove(clock);

	if (mQtClockPool.size() >= MAX_CLOCK_POOL_SIZE) {
		delete clock;
	}
	else {
		mQtClockPool.push_back(clock);
	}
}

QtMediaDone *QtSchedule::obtainQtMediaDone()
{
	QtMediaDone *done = NULL;

	if (mQtMediaDonePool.size() == 0) {
		LogD("---- create new mediadone object. \n");
		done = new QtMediaDone();
	}
	else {
		//LogD("---- use mediadone object in buffer. \n");
		done = mQtMediaDonePool.back();
		mQtMediaDonePool.pop_back();
	}

	mQtMediaDoneInUse.push_back(done);
	return done;
}
void QtSchedule::releaseMediaDone(QtMediaDone *done)
{
	if(done == NULL )
		return ;

	mQtMediaDoneInUse.remove(done);

	if (mQtMediaDonePool.size() >= MAX_MEDIADONE_POOL_SIZE) {
		LogD("---- delete  mediadone object size:%d. \n",mQtMediaDonePool.size());
		delete done;
	}
	else {
		//LogD("---- push  mediadone object to buffer, size:%d. \n",mQtMediaDonePool.size());
		mQtMediaDonePool.push_back(done);
	}
}

void QtSchedule::releaseLayoutBGPool()
{
	for(std::list<QLabel *>::iterator iter = mQtLayoutBGFilePool.begin();
			iter != mQtLayoutBGFilePool.end(); ++iter)
	{
		if((*iter)->isVisible())
		{
			(*iter)->clear();
			(*iter)->hide();
		}

		delete (*iter);
		(*iter) = NULL;
	}
	mQtLayoutBGFilePool.clear();
}

bool QtSchedule::getFreezeStatus()
{
	return mBUsedWidgetsFreeze;
}

void QtSchedule::releaseUsedWidgets()
{
	RELEASE_UI_WIDGET(QtVideoPlayer);
	RELEASE_UI_WIDGET(QtMarquee);
	RELEASE_UI_WIDGET(QtImage);
	RELEASE_UI_WIDGET(QtTimer);
	RELEASE_UI_WIDGET(QtArrivalMsg);
	RELEASE_UI_WIDGET(QtFlash);
	RELEASE_UI_WIDGET(QtClock);

	while(mQtVideoPlayerLiveInUse.size()) {
		releaseQtVideoPlayerLive(mQtVideoPlayerLiveInUse.front());
	}
}

void QtSchedule::freezeUsedWidgets(bool isfreeze)
{
	mBUsedWidgetsFreeze = isfreeze;
	FREEZE_UI_WIDGET(QtTimer, isfreeze);
	FREEZE_UI_WIDGET(QtVideoPlayer, isfreeze);
	FREEZE_UI_WIDGET(QtMarquee, isfreeze);
	FREEZE_UI_WIDGET(QtImage, isfreeze);
	FREEZE_UI_WIDGET(QtArrivalMsg, isfreeze);
	FREEZE_UI_WIDGET(QtFlash, isfreeze);
	FREEZE_UI_WIDGET(QtClock, isfreeze);

	for (std::list<QtVideoPlayer *>::iterator i = mQtVideoPlayerLiveInUse.begin(); i != mQtVideoPlayerLiveInUse.end(); i++) {
		(*i)->setRunning(isfreeze == false);
	}
}

void QtSchedule::deleteAllWidgets()
{
	DELETE_UI_WIDGET(QtTimer);
	DELETE_UI_WIDGET(QtVideoPlayer);
	DELETE_UI_WIDGET(QtMarquee);
	DELETE_UI_WIDGET(QtImage);
	DELETE_UI_WIDGET(QtArrivalMsg);
	DELETE_UI_WIDGET(QtFlash);
	DELETE_UI_WIDGET(QtClock);

	for (std::list<QtVideoPlayer *>::iterator i = mQtVideoPlayerLivePool.begin(); i != mQtVideoPlayerLivePool.end(); i++) {
		delete (*i);
	}
	for (std::list<QtVideoPlayer *>::iterator i = mQtVideoPlayerLiveInUse.begin(); i != mQtVideoPlayerLiveInUse.end(); i++) {
		delete (*i);
	}
	mQtVideoPlayerLivePool.clear();
	mQtVideoPlayerLiveInUse.clear();
}

void QtSchedule::releaseMediadone()
{
	synchronized(mQtTimerDoneMapperMutex)
	{
		std::map<QtTimer *, QtMediaDone *>::iterator itor = mQtTimerDoneMapper.end() ;
		for(itor = mQtTimerDoneMapper.begin() ;itor!=mQtTimerDoneMapper.end();++itor)
		{
			//timer object will be deleted in deleteAllWidgets()
			LogD("------- type: %d\n",itor->second->mMediaType);
			itor->second->setDefaultValue();
			releaseMediaDone(itor->second);
			//DELETE_ALLOCEDRESOURCE(itor->second);
		}
		mQtTimerDoneMapper.clear();
	}
}

bool QtSchedule::setMediasPath(const std::string &path)
{
	//need to add code to check valid path
	std::size_t namepos = path.find_last_of("/");
	if (namepos != std::string::npos && namepos + 1 == path.size())
	{
		mMediasPath = path;
	}
	else
	{
		mMediasPath = path;
		mMediasPath.append("/");
	}

	return true;
}

void QtSchedule::setBufferedCountIncrease()
{
	const ConfigParser* config = mLCDController->getConfig();
	if(config->mBufferReopenEnable == 0)
	{
		mLiveBufferedCount = 0;
		LogD("config disable stream buffered reopen.\n");
		return ;
	}

	qint64 msecs = QDateTime::currentMSecsSinceEpoch();
	if(msecs-mLastMSecsSinceEpoch > 1000*60) // 1 minutes
	{
		mLiveBufferedCount = 0;
		LogD("Live buffer count reset = 0\n");
	}
	else
	{
		mLiveBufferedCount++;
		LogD("Live buffer count = %d\n",mLiveBufferedCount);
	}

	mLastMSecsSinceEpoch = msecs;
}
int QtSchedule::getBufferedCount()
{
	return mLiveBufferedCount;
}

const unsigned QtSchedule::MAX_TIMER_POOL_SIZE = 12;
const unsigned QtSchedule::MAX_MARQUEE_POOL_SIZE = 4;
const unsigned QtSchedule::MAX_IMAGE_POOL_SIZE = 4;
const unsigned QtSchedule::MAX_PLAYER_POOL_SIZE = 4;
const unsigned QtSchedule::MAX_PLAYER_LIVE_POOL_SIZE = 4;
const unsigned QtSchedule::MAX_ARRIVALMSG_POOL_SIZE = 4;
const unsigned QtSchedule::MAX_WEATHERE_POOL_SIZE = 4;
const unsigned QtSchedule::MAX_FLASH_POOL_SIZE = 4;
const unsigned QtSchedule::MAX_CLOCK_POOL_SIZE = 4;
const unsigned QtSchedule::MAX_MEDIADONE_POOL_SIZE = 20;

const char *QtSchedule::TAG = "QtSchedule";
