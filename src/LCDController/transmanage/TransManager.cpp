/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file name : comment_example.h
 * @author : alex
 * @date : 2017/7/14 14:29
 * @brief : open a file
 */

#include <FileSysUtils.h>
#include <json/DeviceStatusObj.h>
#include <json/LoginObjs.h>
#include <LEDPlayer/LEDPlayer.h>
#include <LCDPlayer/LCDPlayer.h>
#include <Log.h>
#include <Looper.h>
#include <transmanage/TransHandlerFactory.h>
#include <transmanage/TransManager.h>
#include <Version.h>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "bj_pis/convent/hd_opm.h"

const char * TransManager::TAG = "DownloadManager";

// the device controller.
Json::DeviceStatus g_LCDCtrlerDevStatus;
Json::DeviceStatus g_SubDev1DevStatus;

///////////////////////////////////////////////////////////////////
/**
 * Get local device static status.
 * include: 1. hdd size
 *          2. memory size
 *          3. software version.
 * @param[out] status
 */
static void GetLocalSaticStatus(Json::DeviceStatus& status,const ConfigParser* cfg)
{
    if(cfg == NULL)
    {
        return ;
    }

    // hard disk size
    FileSysUtils::DiskSpaceReport rptInfo;
    FileSysUtils::DiskSpaceReporter("/", rptInfo);

    Json::HddInfo hddInfo;
    hddInfo.mHddSize = rptInfo.mTotal / (1024 * 1024)
            + (rptInfo.mTotal % (1024 * 1024) > 0 ? 1 : 0); //GB

    status.mHardware.mHddInfoVec.clear();
    status.mHardware.mHddInfoVec.push_back(hddInfo); // only one hard disk.

    // memory size
    status.mHardware.mMemoryInfo.mMemorySize = FileSysUtils::MemTotal()
            / (1024 * 1024) + (rptInfo.mTotal % (1024 * 1024) > 0 ? 1 : 0);
    ;

    // software version.
//    status.mSoftware.mSysVersion = Version::GetVersion();
    status.mSoftware.mSysVersion = cfg->mVersion;

    // system status.
    status.mHardware.mStatus = Json::HardwareStatus::S_ON;
    status.mSoftware.mStatus = Json::SoftwareStatus::SftStatus::S_Normal;
}

/**
 * Get local device dynamic hardware status.
 * include: 1. cpu radio
 *          2. hdd ratio
 *          3. memory radio
 * @param[out] status
 */
static void GetLocalHdStatus(Json::HardwareStatus& status)
{
    // cpu ratio
    std::stringstream sstrm(FileSysUtils::CpuUsedReporter());
    sstrm >> status.mCpuRadio;

    // hdd ratio
    FileSysUtils::DiskSpaceReport rptInfo;
    FileSysUtils::DiskSpaceReporter("/", rptInfo);

    if (status.mHddInfoVec.size() != 1)
    {
        // should only have one hdd
        return;
    }

    status.mHddInfoVec[0].mHddRatio = rptInfo.mUsedPcg;

    // memory ratio.
    std::stringstream sstrm2(FileSysUtils::MemUsedReporter());
    sstrm2 >> status.mMemoryInfo.mMemoryRatio;

    return;
}

/**
 * Get local device dynamic software status.
 * include:
 *          1. this software used cpu ration.
 *          2. this software used hdd ration.
 *          3. this software used hdd size.
 *          4. this software used memory ration.
 *          5. this software used memory size.
 * @param[out] status
 */
static void GetLocalSftStatus(Json::DeviceStatus& status,
        const ConfigParser* cfg)
{
    if (NULL == cfg)
        return;

    // get used cpu ration,memory ration.
    Json::SoftwareStatus::GetBasicStatus(MODULEPROCESSNAME, status.mSoftware);

    // get software used memory size
    status.mSoftware.mMemoryInfo.mMemorySize =
            status.mHardware.mMemoryInfo.mMemorySize
                    * status.mSoftware.mMemoryInfo.mMemoryRatio;

    status.mSoftware.mCpuRatio = status.mHardware.mCpuRadio;

    // get software used hdd size.
    long dldDirSize = FileSysUtils::GetSpaceSize(cfg->mDldRootDir);
    long snapshortSize = FileSysUtils::GetSpaceSize(cfg->mSnapShotPath);
    status.mSoftware.mHddInfo.mHddSize = (dldDirSize + snapshortSize)
            / (1024 * 1024)
            + ((dldDirSize + snapshortSize) % (1024 * 1024) > 0 ? 1 : 0);

    // get software used hdd ration.
    if(status.mHardware.mHddInfoVec.size()>0 && status.mHardware.mHddInfoVec[0].mHddSize != 0)
		status.mSoftware.mHddInfo.mHddRatio = status.mSoftware.mHddInfo.mHddSize
				/ status.mHardware.mHddInfoVec[0].mHddSize;
    else
    	status.mSoftware.mHddInfo.mHddRatio = 0;

    return;
}

///////////////////////////////////////////////////////////////////
TransManager::TransManager(LCDController* controller) :
        mLCDController(controller), mToken(""), mExpireTime(6000)
{
    const ConfigParser* config = mLCDController->getConfig();

    mDataTrans = new DataTrans(config->mCenterServerIP,
            config->mCenterServerPort);

    mBLogStatus = false;
}

TransManager::~TransManager()
{
	DELETE_ALLOCEDRESOURCE(mDataTrans);
	TransHandlerFactory::Destory();
}

void TransManager::run()
{
    Looper *mlooper = Looper::CreateLooper();
    setLooper(mlooper);

    // 1. Load local schedule first.
    TransHandlerFactory* factory = TransHandlerFactory::Instance(this);
    LogI("[TransManager]--Load local schedule first!\n");
    sendMessage(
            new Message(TransMessageType::LoadLocalSchedule,
                    factory->GetLoader(NTF_ScheduleUpdated)));
    HdOpm::LoadLocal();
    // 2. Login.
    //sendMessage(new Message(TransMessageType::LoginReq),2000);

    // 3. Get initial status of device .
    // sendMessage(new Message(TransMessageType::InitDeviceStatus));

//    TransHandlerFactory* factory = TransHandlerFactory::Instance(this);
//    LogI("[TransManager]--First time get arrival message!\n");
//    sendMessage(
//            new Message(TransMessageType::DownLoadReq,
//                    factory->GetLoader(NTF_RTArrMsgUpdated)), 0);

    //test command add by wfq
//    LogI("[TransManager]--test command!\n");
//    sendMessage(
//            new Message(TransMessageType::DownLoadReq,
//                    factory->GetLoader(NTF_CommandUpdated)), 0);

    mlooper->loop();
}

bool TransManager::handleMessage(Message *msg)
{
    int what = msg->mWhat;

    switch (what)
    {
    case WebSocketPingReq:
    {
        // got ping from server.
        mLCDController->sendMessage(new Message(LCDController::WebSocketPingReq));
        break;
    }
    case LoadLocalSchedule:
    {
        ITransHandler* loader = NULL;
        loader = static_cast<ITransHandler*>(msg->mData);

        if (NULL == loader)
            return false;
        loader->Execute(msg, 1);
        break;
    }
    case LoginReq:
    {
        // 1- Send login request to server.
        LogI("[Login]--Handle Login request!\n ");
        handleLogInReq();
        break;
    }
    case LoginReply:
    {
        // 2- Handle reply of login from server.(got token and expired time)

        // Get reply result.
        DataTrans::UploadStatus rslt =
                static_cast<DataTrans::UploadStatus>(msg->mArg1);
        DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;

        if (DataTrans::UploadSuccess == rslt)
        {

            LogI("[Login]--Handle Login reply!\n ");
            handleLogInReply(work);
            mBLogStatus = true;
        } else
        {
            // Login failed, try re-login 3s later.
            LogI("[Login]-- Try re-login!\n ");
            removeMessage(TransMessageType::LoginReq);
            sendMessage(new Message(TransMessageType::LoginReq), 3000);
            mBLogStatus = false;
        }

        DELETE_ALLOCEDRESOURCE(work);

        break;
    }
    case RefreshTokenReq:
    {
        LogI("[Refresh Token]--Handle Refresh token request!\n ");
        handleRfshTokenReq();

        break;
    }
    case RefreshTokenReply:
    {
        // Get reply result.
        DataTrans::UploadStatus rslt =
                static_cast<DataTrans::UploadStatus>(msg->mArg1);
        DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;

        if (DataTrans::UploadSuccess == rslt)
        {
            LogI("[Refresh Token]-- try refresh token!\n ");

            handleRfshTokenReply(work);
        } else
        {
            static int refreshCnt = 0;
            if (3 == refreshCnt++)
            {
                // after refresh 3 times failed, try relogin.
                LogI("[Login]-- Try re-login!\n ");
                removeMessage(TransMessageType::LoginReq);
                sendMessage(new Message(TransMessageType::LoginReq), 3000);
                refreshCnt = 0;
            } else
            {
                // refresh failed ,default delay 10seconds
            	removeMessage(TransMessageType::RefreshTokenReq);
                sendMessage(new Message(TransMessageType::RefreshTokenReq),
                        3000);
            }
        }

        DELETE_ALLOCEDRESOURCE(work);

        break;
    }
    case InitDeviceStatus:
    {
        GetLocalSaticStatus(g_LCDCtrlerDevStatus, GetConfig());
        sendMessage(new Message(TransMessageType::RptDevStatusReq), 3000);
        break;
    }
    case RptDevStatusReq:
    {
        // request to report device dynamic status.
        handleReportStatusReq();

        const ConfigParser* config = mLCDController->getConfig();

        //get the report interval.
        unsigned rptInterval = config->mStatusRptPeriod * 1000;
        removeMessage(TransMessageType::RptDevStatusReq);
        sendMessage(new Message(TransMessageType::RptDevStatusReq),
                rptInterval);

        break;
    }
    case RptDevStatusReply:
    {
        // Get reply result.
        DataTrans::UploadStatus rslt =
                static_cast<DataTrans::UploadStatus>(msg->mArg1);
        DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;

        if (DataTrans::UploadSuccess == rslt)
        {
            LogI("[Report status]-- Report device status succeed!\n ");
            // TODO: Record the report success
        } else
        {
            // TODO: Report failed, what should i do?
            LogI("[Report status]-- Report device status failed!\n ");
        }

        DELETE_ALLOCEDRESOURCE(work);

        break;
    }
    case DownLoadReq:
    {
        ITransHandler* loader = NULL;
        loader = static_cast<ITransHandler*>(msg->mData);

        if (NULL == loader)
            return false;
        loader->Execute(msg, 0);
        break;
    }
    case OPSMsgUpdated:
    {
        LogE("OPSMsgUpdated\n");
        /*  uint64_t uptime;
         std::string nexttime = "20170925 144000";
         bool ret = SystemClock::StrToUptimeMillis(nexttime, uptime, SystemClockTMFormat);

         uint64_t current = SystemClock::UptimeMillis();
         int64_t next = uptime - current;
         LogE("time:%ld,current:%ld,offset:%ld-%d\n",uptime,current,next,ret);
         */
        mLCDController->sendMessage(
                new Message(LCDController::OPSMsgUpdated, msg->mData,
                        msg->mArg1));
        break;
    }
    case LayoutUpdated:
    {
        LogE("LayoutUpdated\n");

        mLCDController->sendMessage(
                new Message(LCDController::LayoutUpdated, msg->mData));
        break;
    }
    case ArrivalInfoUpdated:
    {
        LogE("ArrivalInfoUpdated\n");
        mLCDController->sendMessage(
                new Message(LCDController::ArrivalInfoUpdated, msg->mData));
        break;
    }
    case TrainTimeUpdated:
    {
        LogE("TrainTimeUpdated\n");
        mLCDController->sendMessage(
                new Message(LCDController::TrainTimeUpdated, msg->mData));
    	break;
    }
    case ScreenOnOffTimeUpdated:
    {
    	LogE("ScreenOnOffTimeUpdated\n");
		mLCDController->sendMessage(
				new Message(LCDController::ScreenOnOffTimeUpdated));
		break;
    }
    case LiveSourceSwitchUpdated:
    {
    	LogE("LiveSourceSwitchUpdated\n");
		mLCDController->sendMessage(
				new Message(LCDController::LiveSourceSwitchUpdated, msg->mArg1));
		break;
    }
    case LEDCmdUpdated:
    {
        LogE("LEDCmdUpdated\n");
        mLCDController->sendMessage(
                new Message(LCDController::LEDCmdUpdated, msg->mData));
        break;
    }
    case ArrMsgBlockDisplayed:
    {
    	LogE("ArriMsgBlockDisplayed.\n");
    	TransHandlerFactory* factory = TransHandlerFactory::Instance(this);
    	if(mBLogStatus)
    	{
			sendMessage(
				new Message(TransMessageType::DownLoadReq,
					factory->GetLoader(NTF_RTArrMsgUpdated)), 0);
    	}

    	/*sendMessage(
				new Message(TransMessageType::DownLoadReq,
						factory->GetLoader(NTF_TrainTimeUpdated)),100);*/

    	break;
    }
    default:
        break;
    }

    return true;
}

void TransManager::sendRequest(ITransHandler* transHandler,
        const TransFileType dltype, void* param)
{
    const ConfigParser* config = mLCDController->getConfig();

    if (NULL == transHandler || NULL == config)
        return;

// Get the url path.
    std::string rawPath;
    config->GetSubPath(dltype, rawPath);

// Modify the url according to different down loader.
    std::string modifiedPath = transHandler->ModifyPath(dltype, rawPath, param);

    mDataTrans->download(transHandler, modifiedPath,
            DataTrans::DataTransType_String, dltype);
}

void TransManager::downloadFile(ITransHandler* transHandler,
        const TransFileType dltype, const std::string &fileNewName,
        const std::string& md5, void* param)
{
    const ConfigParser* config = mLCDController->getConfig();

    if (NULL == transHandler || NULL == config)
        return;

    // Get the url path.
    std::string rawPath;
    config->GetSubPath(dltype, rawPath);

    // Modify the url according to different down loader.
    std::string modifiedPath = transHandler->ModifyPath(dltype, rawPath, param);

    mDataTrans->download(transHandler, modifiedPath,
            DataTrans::DataTransType_Resource, dltype,
            config->mDldRootDir + config->mContentpath, fileNewName, md5);
}

void TransManager::downloadTransWork(DataTrans::TransWork *work)
{
    mDataTrans->download(work, 3000);
}

void TransManager::sendReply(ITransHandler* transHandler,
        const DataTrans::DataTransType mTranstype, const TransFileType dltype,
        const std::string& upData, void* param)
{
    const ConfigParser* config = mLCDController->getConfig();

    if (NULL == transHandler || NULL == config)
        return;

// Get the url path.
    std::string rawPath;
    config->GetSubPath(dltype, rawPath);

// Modify the url according to different down loader.
    std::string modifiedPath = transHandler->ModifyPath(dltype, rawPath, param);

    LogD("########### upload data: %s\n", upData.c_str());
    mDataTrans->upload(transHandler, modifiedPath, mTranstype, dltype, upData);
}

std::string TransManager::GetCtrlerDevId()
{
    if (NULL != mLCDController)
        return mLCDController->GetDevId();

    return "";
}

const ConfigParser* TransManager::GetConfig() const
{
    if (NULL != mLCDController)
        return mLCDController->getConfig();

    return NULL;
}

const LCDLEDControllerFlag TransManager::GetControllerFlag() const
{
    if (NULL != mLCDController)
        return mLCDController->getControllerFlag();

    return Unknown_controller;
}

void TransManager::handleLogInReq()
{
    const ConfigParser* config = mLCDController->getConfig();

// Get the account and password.
    std::string reqBody = Json::LoginReq::ToJson(config->mAccount,
            config->mPassWord);

// Post login request
    mDataTrans->upload(this, config->mLoginPath,
            DataTrans::DataTransType_String_POSTWithRply, LoginReply, reqBody);
}

void TransManager::handleLogInReply(DataTrans::TransWork *work)
{
    if (NULL == work)
        return;

    LogD("### LogIn reply(size-%d): %s\n", work->mJsonRplyData.length(),work->mJsonRplyData.c_str());
//parse the reply json body.
    Json::LoginReply replyData;
    bool ret = Json::LoginReply::Parse(work->mJsonRplyData.c_str(), &replyData);
    if (!ret)
        return;

    mToken = replyData.mToken;
    mExpireTime = replyData.mExpireTime;

    mDataTrans->setToken(mToken);

// send refresh request according expire time.
    int refreshTime = 0;
    if(mExpireTime > 60 || mExpireTime <= 0)
    	 refreshTime = 60;
    else
    	refreshTime = mExpireTime / 2;

    LogD("#################################### Token refresh mExpireTime-%d\n",
            mExpireTime);

//    int refreshTime = 2;
    removeMessage(TransMessageType::RefreshTokenReq);
    sendMessage(new Message(TransMessageType::RefreshTokenReq),
            refreshTime * 1000);

    TransHandlerFactory* factory = TransHandlerFactory::Instance(this);

    LogI("[TransManager]--First time get command list!\n");
    sendMessage(
            new Message(TransMessageType::DownLoadReq,
                    factory->GetLoader(NTF_CommandUpdated)), 0);

    LogI("[TransManager]--First time get ops list!\n");
    sendMessage(
            new Message(TransMessageType::DownLoadReq,
                    factory->GetLoader(NTF_RTOPSMsgUpdated)), 0);

    LogI("[TransManager]--First time get schedule-update list!\n");
    sendMessage(
            new Message(TransMessageType::DownLoadReq,
                    factory->GetLoader(NTF_ScheduleUpdated)), 0);

    LogI("[TransManager]--First time get arrival information!\n");
    sendMessage(
            new Message(TransMessageType::DownLoadReq,
                    factory->GetLoader(NTF_RTArrMsgUpdated)), 0);

/*    LogI("[TransManager]--First time get train time config!\n");
	sendMessage(
			new Message(TransMessageType::DownLoadReq,
					factory->GetLoader(NTF_TrainTimeUpdated)), 0);*/

    LogI("[TransManager]--First time get screen on off config!\n");
	sendMessage(
			new Message(TransMessageType::DownLoadReq,
					factory->GetLoader(NTF_ScreenOnOffUpdated)), 0);

    LogI("[TransManager]--First time live source config!\n");
	sendMessage(
			new Message(TransMessageType::DownLoadReq,
					factory->GetLoader(NTF_LiveSourceUpdated)), 0);

    LogI("[TransManager]--First report device status!\n");
    sendMessage(new Message(TransMessageType::InitDeviceStatus), 0);

// send token to LCDController.
    mLCDController->sendMessage(
            new Message(LCDController::LoginSucceed, &mToken), 0);
}

// Send refresh token request.
void TransManager::handleRfshTokenReq()
{
    const ConfigParser* config = mLCDController->getConfig();

// Post login request
    mDataTrans->upload(this, config->mRefreshPath,
            DataTrans::DataTransType_String_POSTWithRply, RefreshTokenReply);
}

// Handle the reply of refresh token.
void TransManager::handleRfshTokenReply(DataTrans::TransWork *work)
{
    if (NULL == work)
        return;

//parse the reply json body.
    Json::LoginReply replyData;
    LogD("#########  replydata[size=%d]: %s\n", work->mJsonRplyData.size(),
            work->mJsonRplyData.c_str());
    bool ret = Json::LoginReply::Parse(work->mJsonRplyData.c_str(), &replyData);
    if (!ret)
        return;

    mToken = replyData.mToken;
    mExpireTime = replyData.mExpireTime;
    LogD("#################################### Token refresh mExpireTime-%d\n",
            mExpireTime);

    mDataTrans->setToken(mToken);

// send refresh request according expire time.
    int refreshTime = 0;
    if(mExpireTime > 60 || mExpireTime <= 0)
    	 refreshTime = 60;
    else
    	refreshTime = mExpireTime / 2;
//    int refreshTime = 2;
    LogD("----------- Refreshed token;%s----------\n", mToken.c_str());

    removeMessage(TransMessageType::RefreshTokenReq);
    sendMessage(new Message(TransMessageType::RefreshTokenReq),
            refreshTime * 1000);

// send token to LCDController.
    mLCDController->sendMessage(
            new Message(LCDController::RefreshToken, &mToken), 0);
}

// Send report status request.
void TransManager::handleReportStatusReq()
{
    // 1.Report local device.
    reportLocalDevStatus();

    // 2.Report sub-device.
   // reportSubDevStatus();
}

// report local device status.
void TransManager::reportLocalDevStatus()
{
    const ConfigParser* cfg = GetConfig();
    if(g_LCDCtrlerDevStatus.mHardware.mHddInfoVec.size()==0 ||
    		g_LCDCtrlerDevStatus.mHardware.mHddInfoVec[0].mHddSize == 0)
    {
    	GetLocalSaticStatus(g_LCDCtrlerDevStatus,GetConfig());
    }

    // get dynamic hard-ware information.
    GetLocalHdStatus(g_LCDCtrlerDevStatus.mHardware);

    // get dynamic soft-ware information.
    GetLocalSftStatus(g_LCDCtrlerDevStatus, cfg);

    std::string jsonStr;
    bool ret = Json::DeviceStatus::ToJson(g_LCDCtrlerDevStatus, jsonStr);
    LogD("\t\t Dev status report jsonStr= \n %s\n", jsonStr.c_str());
    if (ret)
    {
        // get the complete url.
        std::string urlPath = cfg->mDevStatusSubpath + cfg->mDeviceId;

        // post report status request.
        mDataTrans->upload(this, urlPath, DataTrans::DataTransType_String_POST,
                RptDevStatusReply, jsonStr);
    }
}

// report sub-device status.
void TransManager::reportSubDevStatus()
{
    const ConfigParser* cfg = GetConfig();

    // TODO: Get sub device status and report.
    if (LED_Controller_flag == mLCDController->getControllerFlag())
    {
        LEDPlayer* ledPlayer = mLCDController->GetLEDPlayer();
        if ( NULL == ledPlayer)
        {
            LogE("mLCDController->GetLEDPlayer() returns NULL!\n");
            return;
        }

        std::map<std::string, Json::HardwareStatus::HdStatus> ledStatusMap;
        ledPlayer->GetDevStatus(ledStatusMap);

        std::map<std::string, Json::HardwareStatus::HdStatus>::const_iterator itor =
                ledStatusMap.begin();

        Json::DeviceStatus singleSubDevStatus;
        std::string jsonStr;
        for (; itor != ledStatusMap.end(); ++itor)
        {
            singleSubDevStatus.mHardware.mStatus = itor->second;

            bool ret = Json::DeviceStatus::ToJson(singleSubDevStatus, jsonStr);
            LogD("\t\t Sub-Dev status report jsonStr= \n %s\n",
                    jsonStr.c_str());
            if (ret)
            {
                // get the complete url.
                std::string urlPath = cfg->mDevStatusSubpath + itor->first;

                // post report status request.
                mDataTrans->upload(this, urlPath,
                        DataTrans::DataTransType_String_POST, RptDevStatusReply,
                        jsonStr);
            }
        }
    }
	else if(mLCDController->getControllerFlag() == LCD_Controller_flag)
	{
		LCDPlayer* lcdplayer = mLCDController->getLCDPlayer();
		if(lcdplayer != NULL)
		{
			std::map<std::string, Json::HardwareStatus::HdStatus> deviceStatusMap;
			lcdplayer->getLCDDeviceStatus(deviceStatusMap);
			for(std::map<std::string, Json::HardwareStatus::HdStatus>::iterator iter = deviceStatusMap.begin();
					iter != deviceStatusMap.end();++iter)
			{
				g_SubDev1DevStatus.mHardware.mStatus = iter->second;
				std::string jsonStr;
				bool ret = Json::DeviceStatus::ToJson(g_SubDev1DevStatus, jsonStr);
				LogD("\t\t LCD status report jsonStr= \n %s\n", jsonStr.c_str());
				if (ret)
				{
					// get the complete url.
					std::string urlPath = cfg->mDevStatusSubpath + iter->first;

					// post report status request.
					mDataTrans->upload(this, urlPath, DataTrans::DataTransType_String_POST,
							RptDevStatusReply, jsonStr);
				}
			}
		}
	}

}
LCDController* TransManager::getLCDController()
{
	if(mLCDController != NULL)
	{
		return mLCDController;
	}

	return NULL;
}
