/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file name : LCDController.cpp
 * @author : wangfuqiang
 * @date : 2017/8/8 14:29
 * @brief :
 */
#include <transmanage/TransManager.h>
#include "LCDController.h"
#include "mediaplay/QtPanel.h"
#include "LEDPlayer/LEDPlayer.h"
#include "Log.h"
#include "websocket/WebSocketNotify.h"
#include "serial/SerialManager.h"
#include "FileSysUtils.h"
#include "LCDPlayer/LCDPlayer.h"
#include "ntp/NtpDate.h"
#include "Mutex.h"

LCDController* LCDController::pInstance = nullptr;

LCDController::LCDController() :
        mWsNotify(NULL), mQtPanel(NULL),mDevId(""),mLostPingCnt(-1)
{
    pInstance = this;
    mConfig = new ConfigParser("./",
                "config.xml");

    if (mConfig == NULL)
    {
    	printf("parse config file failed.\n");
    	return ;
    }

    createDirectory();

    mDevId = mConfig->mDeviceId;

    std::string modulename = mConfig->mModuleName/*MODULEPROCESSNAME*/;
    if(mConfig->mSWlogDirect == LOG_ToConsole)
    	modulename = "stdout";

    Log::setLogReduce(0);
    Log::LogInit(mConfig->mSftLogPath, modulename, Log::VERBOSE);

    mTransManager = new TransManager(this);
    mTransManager->start();

	mQtPanel = NULL;
	mLCDPlayer = NULL;
	mLEDPlayer = NULL;
    if(mConfig->mLCDLEDFlag == LCD_Controller_flag)
    {
    	LogD("LCDController device.\n");
    	mControllerFlag = LCD_Controller_flag;
    	mQtPanel = new QtPanel(this);
    	mQtPanel->start();

//        mLCDPlayer = new LCDPlayer(this);
//        mLCDPlayer->start();
//
//        mLCDPlayer->sendMessage(new Message(LCDPlayer::LCD_GetStatus));
    }
    else if(mConfig->mLCDLEDFlag == LED_Controller_flag)
    {
    	LogD("LEDController device.\n");
    	mControllerFlag = LED_Controller_flag;
    	mLEDPlayer = new LEDPlayer(this);
    	mLEDPlayer->start();

    	//mLEDPlayer->sendMessage(new Message(LEDPlayer::LED_DateSync), 1000);
    }
    else
    {
    	LogE("Unknown LCD_LED flag, 0 - LCD, 1 - LED.\n");
    	mControllerFlag = Unknown_controller;
    }

    if(mConfig->mNtpEnable == NTPDATE_Function_disable)
    {
    	mNtpDate = NULL;
    }
    else
    {
        mNtpDate = new NtpDate(this);
        if(mNtpDate != NULL)
        {
        	LogD("Ntp function opened!\n");
        	mNtpDate->start();
        }
    }


    //mWsNotify = new WebSocketNotify(mTransManager);

    sendMessage(new Message(ParseConfigFile), 1000);
    sendMessage(new Message(HouseKeeping), 10000);
}

LCDController::~LCDController()
{
    DELETE_ALLOCEDRESOURCE(mWsNotify);
    DELETE_ALLOCEDRESOURCE(mTransManager);
    DELETE_ALLOCEDRESOURCE(mQtPanel);
    DELETE_ALLOCEDRESOURCE(mLEDPlayer);
    DELETE_ALLOCEDRESOURCE(mLCDPlayer);
    DELETE_ALLOCEDRESOURCE(mConfig);
    DELETE_ALLOCEDRESOURCE(mNtpDate);
}


LCDController* LCDController::GetInstance()
{
    return pInstance;
}

bool LCDController::handleMessage(Message *msg)
{
    switch (msg->mWhat)
    {
    case ParseConfigFile:
    {
        //sendMessage(new Message(ParseConfigFile),  1000);
        break;
    }
    case WebSocketPingReq:
    {
        mLostPingCnt = 0; // reset the lost count to 0;
        //LogI("[WebSocketPingReq] Got WebSocketPingReq,mLostPingCnt=%d!\n",mLostPingCnt);
        break;
    }
    case WebSocketPingMonitor:
    {
        ++mLostPingCnt;
        //LogI("\t\t $$$$$$$$$$$$$$$$ [WebSocketPingMonitor] ping lost count =%d!\n",mLostPingCnt);

        removeMessage(WebSocketPingMonitor);
        sendMessage(new Message(WebSocketPingMonitor),5000);

        if( 10 == mLostPingCnt)
        {
            sendMessage(new Message(WebSocketReConnect));
        }

        break;
    }
    case WebSocketReConnect:
    {
        LogI("[WebSocketReConnect] web socket need to re-connect !\n");
        mLostPingCnt = 0;
        mWsNotify->Close();  // mWsNotify thread will auto restart to do re-login req!!

        mTransManager->removeMessage(TransMessageType::LoginReq);
        mTransManager->sendMessage(new Message(TransMessageType::LoginReq));
        break;
    }
    case LoginSucceed:
    {
        LogI("[WebSocketNotify] Start WebSocketNotify thread!\n");
        std::string* token = (std::string*) msg->mData;
        mWsNotify->SetToken(*token);
        mWsNotify->Init();

        // After login succeed, start web-socket client.
        mWsNotify->start();

        // start monitor ping request.
        sendMessage(new Message(WebSocketPingMonitor));

        break;
    }
    case RefreshToken:
    {
        LogI("[WebSocketNotify] Update token!\n");
        std::string* token = (std::string*) msg->mData;
        mWsNotify->SetToken(*token);
        mWsNotify->Init();
        break;
    }
    case OPSMsgUpdated:
    {
        LogE("OPSupdated.\n");
        if(mQtPanel != NULL)
        {
        	mQtPanel->sendOPMsg(msg->mData,msg->mArg1);
        }
        else if(mLEDPlayer != NULL)
        {
        	mLEDPlayer->sendMessage(new Message(LEDPlayer::LED_OPSMsgUpdated,msg->mData,msg->mArg1));
        }

        break;
    }
    case LayoutUpdated:
    {
        LogE("LayoutUpdated.\n");
        if(mQtPanel != NULL)
        {
        	mQtPanel->sendCurrentLayoutInfo(msg->mData);
        }
        else if(mLEDPlayer != NULL)
        {
        	mLEDPlayer->sendMessage(new Message(LEDPlayer::LED_ScheduleUpdated,msg->mData));
        }

        break;
    }
    case ArrivalInfoUpdated:
    {
        LogE("ArrivalInfoUpdated.\n");
        if(mQtPanel != NULL)
        {
        	mQtPanel->sendRTArrMsgUpdated(msg->mData);
        }

        break;
    }
    case TrainTimeUpdated:
    {
    	LogE("TrainTimeUpdated.\n");
		if(mQtPanel != NULL)
		{
			mQtPanel->sendTrainTimeUpdated(msg->mData);
		}

    	break;
    }
    case ScreenOnOffTimeUpdated:
    {
    	LogE("ScreenOnOffTimeUpdated.\n");
		if(mLCDPlayer != NULL)
		{
			mLCDPlayer->sendMessage(new Message(LCDPlayer::LCD_ScreenOnOffTimeUpdated));
		}
    	break;
    }
    case LiveSourceSwitchUpdated:
    {
    	LogE("LiveSourceSwitchUpdated.\n");
    	if(mQtPanel != NULL)
		{
			mQtPanel->sendLiveSourceUpdated(msg->mArg1);
		}
    	break;
    }
    case LEDCmdUpdated:
    {
        LogE("LEDCmdUpdated.\n");
        if(mLEDPlayer != NULL)
        {
        	mLEDPlayer->sendMessage(new Message(LEDPlayer::LED_ExecuteCmd,msg->mData));
        }
    	break;
    }
    case ArriMsgBlockDisplayed:
    {
    	LogE("ArriMsgBlockDisplayed.\n");
    	mTransManager->sendMessage(new Message(TransMessageType::ArrMsgBlockDisplayed));

    	break;
    }
    case NtpDateSync:
    {
    	LogD("Ntp date sync completed.\n");
    	break;
    }
    case HouseKeeping:
    {
    	FileSysUtils::HouseKeeping(mConfig->mSftLogPath, mConfig->mHouseKeepDay);

    	FileSysUtils::HouseKeeping(mConfig->mWatchdogLogPath, mConfig->mHouseKeepDay);

    	FileSysUtils::HouseKeeping(mConfig->mSnapShotPath, mConfig->mHouseKeepDay);

    	FileSysUtils::HouseKeepingByCount(mConfig->mCoreDumpLogPath, mConfig->mCoreDumpNumber);

    	sendMessage(new Message(HouseKeeping), 3600*1000);
    	break;
    }
    default:
        break;
    }
    return true;
}

ConfigParser* LCDController::getConfig()
{
    return mConfig;
}

std::string LCDController::GetDevId()
{
    return mDevId;
}

TransManager* LCDController::GetTransManager()
{
    return mTransManager;
}

LCDLEDControllerFlag LCDController::getControllerFlag()
{
	return mControllerFlag;
}

LEDPlayer* LCDController::GetLEDPlayer()
{
    return mLEDPlayer;
}

LCDPlayer* LCDController::getLCDPlayer()
{
	return mLCDPlayer;
}

void LCDController::createDirectory()
{
	FileSysUtils::MakeDir(mConfig->mDldRootDir);
	FileSysUtils::MakeDir(mConfig->mDldRootDir+mConfig->mSchDldPath);
	FileSysUtils::MakeDir(mConfig->mDldRootDir+mConfig->mSchPlayPath);
	FileSysUtils::MakeDir(mConfig->mDldRootDir+mConfig->mContentpath);

	FileSysUtils::MakeDir(mConfig->mSnapShotPath);
	FileSysUtils::MakeDir(mConfig->mWatchdogLogPath);
	FileSysUtils::MakeDir(mConfig->mCoreDumpLogPath);
}

const char *LCDController::TAG = "LCDController";

