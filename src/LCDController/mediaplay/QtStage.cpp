/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file name : comment_example.h
 * @author : alex
 * @date : 2017/7/14 14:29
 * @brief : open a file
 */

#include <config/configparser.h>
#include <json/ScheduleObjs.h>
#include <LCDController.h>
#include <Log.h>
#include <mediaplay/QtShotWorker.h>
#include <mediaplay/QtStage.h>
#include <QtGui/qwidget.h>
#include <stddef.h>
#include <transmanage/ITransHandler.h>
#include <transmanage/TransHandlerFactory.h>
#include <QtCore/QEvent>

QtStage::QtStage(LCDController* lcdcontroller, QWidget *parent) :
	QWidget(parent), mLCDController(lcdcontroller)
{
    const ConfigParser* config = mLCDController->getConfig();

    setupUi(this, config->mScreenWidth, config->mScreenHeight);

    mFullScreenRect.setLeft(0);
    mFullScreenRect.setTop(0);
    mFullScreenRect.setBottom(config->mScreenHeight);
    mFullScreenRect.setRight(config->mScreenWidth);

    //ops msg object
 //   QtOPMessage* mQtOPMsg = new QtOPMessage(this);
	//mQtOPMsg->hide();
	//mQtOPMsg->setResolution(mFullScreenRect);
    qt_ops_msg_[OPSDisplayRegion::fullscreenshow] = new QtOPMessage(this);
    qt_ops_msg_[OPSDisplayRegion::partationshow] = new QtOPMessage(this);
    qt_ops_msg_[OPSDisplayRegion::masterpartation] = new QtOPMessage(this);
    for (auto it = qt_ops_msg_.begin(); it != qt_ops_msg_.end(); ++it)
    {
        it.value()->hide();
        it.value()->setResolution(mFullScreenRect);
    }
	//connect(mQtOPMsg, SIGNAL(signalOPMsgPlay(bool, const OPSMsgParam*)), this,
	//	SLOT(onOPMsgPlay(bool, const OPSMsgParam*)));
	//connect(mQtOPMsg, SIGNAL(signalOPMsgPlayReply(const int, const OPSMsgPlayStatus)), this,
	//	SLOT(onOPMsgPlayReply(const int, const OPSMsgPlayStatus)));

    //------------
    if(config->mSnapShotEnable == SNAPSHOT_Function_disable)
    {
    	mQtShortWorker = NULL;
    }
    else
    {
    	mQtShortWorker = new QtShotWorker(mLCDController);
    }

    //schedule
    mQtSchedule = new QtSchedule(lcdcontroller,this);
    mQtSchedule->setResolution(mFullScreenRect);
    mQtSchedule->setMediasPath(config->mDldRootDir + config->mContentpath);
    mQtSchedule->show();

	QPalette mDefBGPalette;
	mDefBGPalette.setColor(QPalette::Window, Qt::GlobalColor::black);
	setPalette(mDefBGPalette);
}

QtStage::~QtStage()
{
	for (auto it = qt_ops_msg_.begin(); it != qt_ops_msg_.end(); ++it)
	{
        DELETE_ALLOCEDRESOURCE(it.value());
	}
	DELETE_ALLOCEDRESOURCE(mQtShortWorker);
	DELETE_ALLOCEDRESOURCE(mQtSchedule);
}

void QtStage::setupUi(QWidget* QtStage, int width, int height)
{
    setSize(width, height);

    setAutoFillBackground(true);
    //setPalette(mDefBGPalette);

   // this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
   // this->showFullScreen();
    QMetaObject::connectSlotsByName(this);
}

void QtStage::setSize(int width, int height)
{
    if (width != mWidth || height != mHeight)
    {
        resize(width, height);

        mWidth = width;
        mHeight = height;
    }
}

//-------------------Schedule-------------------
void QtStage::onScheduleUpdated(void* schpkg)
{
	LogE("onScheduleUpdated\n");
	mQtSchedule->setPlayLayoutInfo(schpkg);
}

void QtStage::onSchedulePlayReply()
{

}
void QtStage::onRTArrivalMsgUpdated(void* data)
{
	mQtSchedule->setRTArrivalMsgInfo(data);

}

void QtStage::onTrainTimeUpdated(void* data)
{
	mQtSchedule->setTrainTimeInfo(data);
}

void QtStage::onLiveSourceUpdated(int playsource)
{
	mQtSchedule->setLiveSourceInfo(playsource);
}

//-------------------OPS------------------------
void QtStage::onOPSMsgUpdated(void* msg,int status)
{
	LogE("onOPSMsgUpdated  status - %d\n",status);

    if (status==(int)OPSUpdateStatus::OPS_noopsstatus)
    {
        for (auto it = qt_ops_msg_.begin(); it != qt_ops_msg_.end(); ++it)
        {
            it.value()->hide();
            it.value()->setRunning(false, true);
        }
    }
    else
    {
        if (status == (int)OPSUpdateStatus::OPS_deletestatus)
        {
            for (auto it = qt_ops_msg_.begin(); it != qt_ops_msg_.end(); ++it)
            {
                it.value()->setOPSMsg(msg, status);
            }
    		int* deleteid = (int*)msg;
    		if (deleteid != nullptr)
    		{
    			delete deleteid;
    			deleteid = nullptr;
    		}
        }
        else
        {
            Json::OPSMsgDetail* opsmsg = (Json::OPSMsgDetail*)msg;
            qt_ops_msg_[opsmsg->mDisplayRegion]->setOPSMsg(msg, status);
            LogD("DisplayRegion %d set message %d\n", opsmsg->mDisplayRegion, opsmsg->mBasic.mId);
        }
    }
}

void QtStage::onOPMsgPlay(bool bstart, OPSMsgParam* currOps, int display_region)
{
    if (bstart)
    {
		const ConfigParser* config = mLCDController->getConfig();
		if(currOps->mDisplayRegion == OPSDisplayRegion::fullscreenshow)
		{
			mQtSchedule->freezeUsedWidgets(true);
            mQtSchedule->setOPSFullScreen(currOps);
			qt_ops_msg_[OPSDisplayRegion::fullscreenshow]->setGeometry(0, 0, config->mScreenWidth, config->mScreenHeight);
		}
		else if(currOps->mDisplayRegion == OPSDisplayRegion::masterpartation)
		{
			int x,y,w,h;
			bool ret = mQtSchedule->setOPSHalfScreen(true,x,y,w,h, currOps);
			LogD("#####half#######  %d   %d,%d,%d,%d\n",ret,x,y,w,h);
			if(ret)
			{
				//mQtOPMsg->setGeometry(0,y,1920,h);
				qt_ops_msg_[OPSDisplayRegion::masterpartation]->setGeometry(x, y, w, h);
			}
			else
			{
				LogD("Don't find the master partation.\n");
				return ;
			}
		}
		else if(currOps->mDisplayRegion == OPSDisplayRegion::partationshow)
		{
			int x,y,w,h;
			bool ret = mQtSchedule->setOPSPartation(true,x,y,w,h, currOps);
			LogD("######partation######  %d   %d,%d,%d,%d\n",ret,x,y,w,h);
			if(ret)
			{
				//mQtOPMsg->setGeometry(0,y,1920,h);
				qt_ops_msg_[OPSDisplayRegion::partationshow]->setGeometry(x, y, w, h);
                qt_ops_msg_[currOps->mDisplayRegion]->setBGFile("/home/workspace/media/" + mQtSchedule->getBackImage());
			}
			else
			{
				LogD("Don't find the partation with OPSFlag.\n");
				return ;
			}

		}
		else
		{
			LogE("Unknown region to show OPS message\n");
		}
		
        qt_ops_msg_[currOps->mDisplayRegion]->setRunning(true);
        qt_ops_msg_[currOps->mDisplayRegion]->show();
        qt_ops_msg_[currOps->mDisplayRegion]->raise();
        if(currOps->mDisplayRegion!=OPSDisplayRegion::fullscreenshow&&qt_ops_msg_[OPSDisplayRegion::fullscreenshow]->isVisible())
        {
            	qt_ops_msg_[OPSDisplayRegion::fullscreenshow]->raise();
        }
    }
    else
    {
    	if(display_region == OPSDisplayRegion::fullscreenshow)
		{
    		mQtSchedule->freezeUsedWidgets(false);
		}
    	else if(display_region == OPSDisplayRegion::masterpartation)
    	{
			int x,y,w,h;
			bool ret = mQtSchedule->setOPSHalfScreen(false,x,y,w,h, currOps);
    	}

    	else if(display_region == OPSDisplayRegion::partationshow)
    	{
			int x,y,w,h;
			bool ret = mQtSchedule->setOPSPartation(false,x,y,w,h, currOps);
    	}
    	// temporary modify
    	if (display_region < fullscreenshow || display_region >= unknownregion)
    	{
    		return;
    	}
    	qt_ops_msg_[display_region]->setRunning(false);

        if(qt_ops_msg_[display_region]->getOPSPlayListSize() == 0)
		{
			LogD("OPS play list is empty, and send DL_OPSMsgUpdatedNotify message\n");
			ITransHandler* loader = getTransLoader(NTF_RTOPSMsgUpdated);
			loader->sendMessage(new Message(DL_OPSMsgUpdatedNotify));
			qt_ops_msg_[display_region]->hide();
		}
//        int x,y,w,h;
//        mQtSchedule->setOPSPartation(false,x,y,w,h);
    }
}

void QtStage::onOPMsgPlayReply(const int id,const OPSMsgPlayStatus status)
{
    Message* msg = NULL;
    if(status == OPSMsgPlayStatus::StartPlaying)
    {
    	LogD("## send onOPMsgPlayReply - id:%d, playstatus:%d\n",id, OPS_Playing);
        msg = new Message(UP_OPSReply, id, OPS_Playing);
    }
    else if(status == OPSMsgPlayStatus::PlayEnd)
    {
    	LogD("## send onOPMsgPlayReply - id:%d, playstatus:%d\n",id, OPS_PlayFinished);
    	msg = new Message(UP_OPSReply, id,OPS_PlayFinished);
    }
    else
    {
    	//LogD("## send onOPMsgPlayReply - playstatus:%d\n", OPS_PlayWithDrawed);
    	//msg = new Message(UP_OPSReply, id,OPS_PlayWithDrawed);
        LogD("## send onOPMsgPlayReply - playstatus:%d\n", OPS_PlayFinished);
        msg = new Message(UP_OPSReply, id,OPS_PlayFinished);
    }

    if(msg != NULL)
    {
    	ITransHandler* loader = getTransLoader(NTF_RTOPSMsgUpdated);
    	loader->sendMessage(msg, 0);
    }
    else
    {
    	LogE("OPS Replay msg == NULL.\n");
    }

    //if(mQtOPMsg->getOPSPlayListSize() == 0)
    //{
    //	LogD("OPS play list is empty, and send DL_OPSMsgUpdatedNotify message\n");
    //	ITransHandler* loader = getTransLoader(NTF_RTOPSMsgUpdated);
    //	loader->sendMessage(new Message(DL_OPSMsgUpdatedNotify));
    //	mQtOPMsg->hide();
    //}

}

ITransHandler* QtStage::getTransLoader(NotifyMessageCode code)
{
    TransHandlerFactory* factory = TransHandlerFactory::Instance(
            mLCDController->GetTransManager());
    ITransHandler* loader = factory->GetLoader(NTF_RTOPSMsgUpdated);

    return loader;
}

void QtStage::keyPressEvent(QKeyEvent* event)
{

}

void QtStage::closeEvent(QCloseEvent* event)
{
    exit(0);
}

void QtStage::changeEvent ( QEvent* event )
{

	if(event->type() == QEvent::WindowStateChange )
	{
		if(Qt::WindowState::WindowFullScreen == this->windowState())
		{
			LogD("---------- changeEvent  fullscreen -----------  isMax:%d, isFullScreen:%d\n",
						isMaximized(),isFullScreen());
		}
		else if(Qt::WindowState::WindowMaximized == this->windowState() ||
				Qt::WindowState::WindowMinimized == this->windowState())
		{
			LogD("---------- changeEvent not fullscreen -----------  isMax:%d, isFullScreen:%d, windowState:%d\n",
						isMaximized(),isFullScreen(),(int)(this->windowState()));
			this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
			this->showFullScreen();
		}
		else
		{
			LogD("----------changeEvent other status -----------  isMax:%d, isFullScreen:%d, windowState:%d\n",
						isMaximized(),isFullScreen(),(int)(this->windowState()));
		}
	}
}

const char *QtStage::TAG = "QtStage";
