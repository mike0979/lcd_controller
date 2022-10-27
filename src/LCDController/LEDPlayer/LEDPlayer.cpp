/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file name : comment_example.h
 * @author : alex
 * @date : 2017/7/14 14:29
 * @brief : open a file
 */

#include <config/configparser.h>
#include <CommonDef.h>
#include <IPCallID.h>
#include <json/ScheduleObjs.h>
#include <LCDController.h>
#include <LEDPlayer/LEDAdapterClient.h>
#include <LEDPlayer/LEDPlayer.h>
#include <LEDProtocol.h>
#include <Log.h>
#include <Looper.h>
#include <Message.h>
#include <SystemClock.h>
#include <transmanage/ITransHandler.h>
#include <transmanage/TransHandlerFactory.h>
#include <ChineseConvert.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <utility>
#include <vector>

#define MAX_ZONE_CNT    4 // max count of LED zone
#define MAX_ZONE_INDEX  3 // LED zone index [0,3]
#define TIME_ZONE_INDEX 3 // the date time display zone.

class ITransHandler;

static ZoneInfo::ZoneEffect getOPSBestDspMode(int devWidth,
        const std::string& content);
static void ShowContent(const unsigned char *p, int size)
{
    for (int i = 0; i < size; i++)
        printf("%02x ", p[i]);

    printf("\n");
}

//////////////////////////////

// color format : "FF0000" --> "255000000"
void ColorStrCvt(const std::string& inStr, std::string& outStr)
{
    if (inStr.size() != 6)
        return;

    outStr.clear();

    std::string tempStr;
    char* pEnd;
    long int val = 0;
    char tempBuf[4] =
    { 0 };

    for (int i = 0; i < 6; i = i + 2)
    {
        tempStr.append("0x");
        tempStr += inStr.substr(i, 2);

        val = strtol(tempStr.c_str(), &pEnd, 16);
        snprintf(tempBuf, sizeof(tempBuf), "%03ld", val);

        outStr += tempBuf;
        tempStr.clear();
    }

    return;
}

//////////////////////////////
//LEDPlayer::LEDPlayer(ConfigParser* cfg) :
//        mConfig(cfg)
//{
//    if (!GetAllLedDevInfo())
//    {
//        LogE("\t\t @@@@@ GetAllLedDevInfo failed! @@@@@\n");
//    }
//}

LEDPlayer::LEDPlayer(LCDController* lcdcontroller) :
        mLCDController(lcdcontroller), mCurrentOPS(NULL), mCurrentSchedule(
        NULL), mStatus(LED_Idle)
{
    if (!GetAllLedDevInfo())
    {
        LogE("\t\t @@@@@ GetAllLedDevInfo failed! @@@@@\n");
    }

    mLEDAdapterClient = new LEDAdapterClient(this);

    // connect to LEDAdapter call center.
    sendMessage(new Message(LEDPlayer::LED_Adapter_Connect));
    InitCodecToUTF8();
}

LEDPlayer::~LEDPlayer()
{
}

void LEDPlayer::run()
{
    Looper *mlooper = Looper::CreateLooper();
    setLooper(mlooper);

    mlooper->loop();
}

bool LEDPlayer::handleMessage(Message *msg)
{
    switch (msg->mWhat)
    {
    case LED_Adapter_Connect:
    {
        LogD("------------  LEDPlayer try connect to LEDAdapter!!\n");
        removeMessage(LED_Adapter_Connect);
        if (!handleConnectAdapter())
            handleServerUnReachable();

        break;
    }
    case LED_Adapter_DisConnect:
    {
        LogD("--------- LEDAdapterClient disconnected with server!\n");
        handleServerUnReachable();

        break;
    }
    case LED_Adapter_SetDevInfos:
    {
        LogD(
                "------------ After connected! LEDAdapterClient try set device information!!\n");

        if (!handleSetDeviceInfo())
            handleServerUnReachable();

        //  send sync datetime req.
        sendMessage(new Message(LEDPlayer::LED_DateSync));
        break;
    }
    case LED_ExecuteCmd:
    {
        LogD(
                "------------  LEDPlayer try send LED_ExecuteCmd to adapter server!!\n");
        handleCommandUpdated(msg->mData);
        break;
    }
    case LED_CMD_AdapterReply:
    {
        LogD(
                "------------  LEDPlayer got CMD_AdapterReply from adapter server!!\n");
        handleAdaperCmdReply(msg->mData);
        break;
    }
    case LED_ScheduleUpdated:
    {
        LogD(
                "------------  LEDPlayer try send LED_ScheduleUpdated to adapter server!!\n");
        handleScheduleUpdated(msg->mData);
        break;
    }
    case LED_OPSMsgUpdated:
    {
        LogD(
                "------------  LEDPlayer try send LED_OPSMsgUpdated to adapter server!!\n");
        handleOPSUpdated(msg->mData, msg->mArg1);
        break;
    }
    case LED_OPS_AdapterReply:
    {
        // do nothing right now.

        break;
    }
    case LED_OPSMsgFinished:
    {
        LogD("------------  LED_OPSMsgFinished,ops id=%d, !\n", msg->mArg1);

        TransHandlerFactory* factory = TransHandlerFactory::Instance(
                mLCDController->GetTransManager());
        ITransHandler* loader = factory->GetLoader(NTF_RTOPSMsgUpdated);

        if (!clearLEDScreenReq())
        {
            LogE(" clearLEDScreenReq to led adapter server failed!\n ");
        }

        loader->sendMessage(new Message(UP_OPSReply, msg->mArg1, msg->mArg2));/*"msg->mArg1" is ops id,"msg->mArg2" is ops statues*/
        loader->sendMessage(new Message(DL_OPSMsgUpdatedNotify)); // try get ops from ops handler.
        break;
    }
    case LED_DateSync:
    {
        LogD(
                "------------  LEDPlayer try send LED_DateSync to adapter server!!\n");
        if (!timeSync2LEDReq())
        {
            LogE(" timeSync2LEDReq to led adapter server failed!\n");
        }

        removeMessage(LED_DateSync);
        sendMessage(new Message(LEDPlayer::LED_DateSync), 600 * 1000); // every 10 min
        break;
    }
    case LED_GetDevStatus:
    {
        LogD(
                "------------  LEDPlayer try send LED_GetDevStatus to adapter server!!\n");
        handleGetDevStatus();
        sendMessage(new Message(LEDPlayer::LED_GetDevStatus), 10 * 1000); // every 10s
        break;
    }
    case LED_ScreenStatus_AdapterReply:
    {
        LogD(
                "------------  LEDPlayer got LED_ScreenStatus_AdapterReply from adapter server!!\n");

        int devAddr = msg->mArg1;
        int screenStatus = msg->mArg2;

        updateDevScreenStatus(devAddr, screenStatus);
        break;
    }
    }

    return true;
}

bool LEDPlayer::handleConnectAdapter()
{
    bool ret = false;
    ConfigParser* cfg = mLCDController->getConfig();

    if (NULL == cfg)
        return ret;

    // caller name is read from configure file.
    if (mLEDAdapterClient->connect(IPCall_Center, cfg->mAdapterClientName))
    {
        ret = true;
        getLooper()->addFd(mLEDAdapterClient->getSocketFd(),
                Looper::FD_EVENT_INPUT, mLEDAdapterClient);

        sendMessage(new Message(LED_Adapter_SetDevInfos));
        sendMessage(new Message(LED_GetDevStatus), 3000); // get dev status.
    }

    return ret;
}

bool LEDPlayer::handleSetDeviceInfo()
{
    std::vector<LedDevInfo> testList;

    std::map<int, LedDevInfo>::const_iterator itor = mLEDDevCfgMap.begin();
    for (; itor != mLEDDevCfgMap.end(); ++itor)
    {
        testList.push_back(itor->second);
    }

    return mLEDAdapterClient->SetDeviceInfoReq(testList);
}

int LEDPlayer::handleOPSUpdated(void* data, int status)
{
    LogD("LED ops update.\n");
    mStatus = LED_OPS_Playing;

    TransHandlerFactory* factory = TransHandlerFactory::Instance(
            mLCDController->GetTransManager());
    ITransHandler* loader = factory->GetLoader(NTF_RTOPSMsgUpdated);

    switch (status)
    {
    case OPSUpdateStatus::OPS_addstatus:
    case OPSUpdateStatus::OPS_otherstatus:
    {
        mCurrentOPS = (Json::OPSMsgDetail*) data;
        break;
    }
    case OPSUpdateStatus::OPS_deletestatus:
    {
        //Try get ops from ops handler.
        loader->sendMessage(new Message(DL_OPSMsgUpdatedNotify));
        return 0;
    }
    case OPSUpdateStatus::OPS_noopsstatus:
    {
        LogD("------------  OPS_noopsstatus ,clear all ops map\n");
        // no ops to play! continue to play schedule.
        mStatus = LED_Idle;

        clearLEDScreenReq();

        sendMessage(new Message(LED_ScheduleUpdated, mCurrentSchedule));
        return 0;
    }
    default:
        LogD("------------  Got ops status=%d \n", status);
        return 0;
    }

    if (NULL != mCurrentOPS)
    {
        LogD("------------  Current OPS is %d\n", mCurrentOPS->mBasic.mId);
        loader->sendMessage(
                new Message(UP_OPSReply, mCurrentOPS->mBasic.mId,
                        OPSStatus::OPS_Playing));

        // send ops to adapter server.
        if (!setOPS2LEDReq())
        {
            LogD(" setOPS2LED to adapter server failed!\n");
            return -1;
        }
        mStatus = LED_OPS_Playing;

        // Calculate ops end time.
        unsigned period = 0;
        CalculateOPSEndTime(mCurrentOPS, period);
        LogD("------------  CalculateOPSEndTime is %lld\n", period);

        sendMessage(
                new Message(LED_OPSMsgFinished, mCurrentOPS->mBasic.mId,
                        OPSStatus::OPS_PlayFinished), period);
    }

    return 0;
}

// Get the current ops end time(in milliseconds).
void LEDPlayer::CalculateOPSEndTime(const Json::OPSMsgDetail* dtl,
        unsigned& period)
{
    if (NULL == dtl)
        return;

    std::string currenttime = SystemClock::Today(SystemClockTMFormat);
    if (dtl->mBasic.mEndTime < currenttime)
    {
        period = 0;
        return;
    }

    uint64_t opsEndtime;
    uint64_t upcurrtime;
    SystemClock::StrToUptimeMillis(dtl->mBasic.mEndTime, opsEndtime,
    SystemClockTMFormat);
    SystemClock::StrToUptimeMillis(currenttime, upcurrtime,
    SystemClockTMFormat);

    period = opsEndtime - upcurrtime;
}

/**
 * Get OPS best display mode.
 * @param devWidth
 * @param content
 * @return
 */
ZoneInfo::ZoneEffect getOPSBestDspMode(int devWidth, const std::string& content)
{
    ZoneInfo::ZoneEffect retEffect = ZoneInfo::ZoneEffect::Static;

    VMS_WORD totalWidth = VMS::GetStringWidth(8, content);

    if (totalWidth > devWidth)
        retEffect = ZoneInfo::ZoneEffect::LeftScroll;

    return retEffect;
}

void InitOPSTitles(std::map<int, ZoneInfo>& mZoneMaps,
        const Json::OPSMsgDetail* opsDtl, int devWidth, int devHeight)
{
    if ( NULL == opsDtl)
        return;

    ZoneInfo titleZone;
    titleZone.zoneId = 0;
    titleZone.backColor = "000000";
    titleZone.effect = ZoneInfo::Static; // static
    titleZone.speed = (ZoneInfo::ZoneSpeed) opsDtl->mText.mSpeed;
    titleZone.yPos = 0;
    titleZone.width = devWidth;
    titleZone.height = devHeight;

    ZoneInfo subTitleZone;
    subTitleZone.zoneId = 1;
    subTitleZone.backColor = "000000";
    subTitleZone.effect = ZoneInfo::Static; // static
    subTitleZone.speed = (ZoneInfo::ZoneSpeed) opsDtl->mText.mSpeed;
    subTitleZone.yPos = 16;
    subTitleZone.width = devWidth;
    subTitleZone.height = devHeight;

    VMS_WORD titleStartPos = 0;
    VMS_WORD subTitleStartPos = 0;
    if (OPSMsgPriority::OPS_PriorityEmergency == opsDtl->mPriority)
    { // emergency OPS level.
        titleZone.foreColor = "FF0000";
        titleZone.dataStr = "紧 急 通 知";
        VMS::GetCenterStartPos(devWidth, 8, titleZone.dataStr, titleStartPos);
        titleZone.xPos = titleStartPos;

        subTitleZone.foreColor = "FF0000";
        subTitleZone.dataStr = "Emergency";
        VMS::GetCenterStartPos(devWidth, 8, subTitleZone.dataStr,
                subTitleStartPos);
        subTitleZone.xPos = subTitleStartPos;
    } else
    { // other OPS level.
        titleZone.foreColor = "00FF00";
        titleZone.dataStr = "通 知";
        VMS::GetCenterStartPos(devWidth, 8, titleZone.dataStr, titleStartPos);
        titleZone.xPos = titleStartPos;

        subTitleZone.foreColor = "00FF00";
        subTitleZone.dataStr = "Notification";
        VMS::GetCenterStartPos(devWidth, 8, subTitleZone.dataStr,
                subTitleStartPos);
        subTitleZone.xPos = subTitleStartPos;
    }

    ZoneInfo dataZone;
    dataZone.zoneId = 2;
    dataZone.foreColor = opsDtl->mText.mForeColor;
    dataZone.backColor = opsDtl->mText.mBackColor;
    //dataZone.effect = (ZoneInfo::ZoneEffect) opsDtl->mText.mEffect;
    //dataZone.speed = (ZoneInfo::ZoneSpeed) opsDtl->mText.mSpeed;
    dataZone.dataStr = opsDtl->mContent;
    dataZone.xPos = 0;
    dataZone.yPos = 48;
    dataZone.width = devWidth;
    dataZone.height = devHeight;

    // get the best ops display mode: static or left scroll.
    dataZone.effect = getOPSBestDspMode(dataZone.width, dataZone.dataStr);
    dataZone.speed = ZoneInfo::ZoneSpeed::Fast;

    mZoneMaps.clear();
    mZoneMaps.insert(std::make_pair(titleZone.zoneId, titleZone));
    mZoneMaps.insert(std::make_pair(subTitleZone.zoneId, subTitleZone));
    mZoneMaps.insert(std::make_pair(dataZone.zoneId, dataZone)); // ops has only 3 zone!!
}

bool LEDPlayer::setOPS2LEDReq()
{
    bool rt = true;

    std::map<int, LedDevInfo>::const_iterator itor = mLEDDevCfgMap.begin();
    for (; itor != mLEDDevCfgMap.end(); ++itor)
    {
        InitOPSTitles(mZoneMaps, mCurrentOPS, itor->second.mLedWidth,
                itor->second.mLedHeight);

        // send setOPS request to LED adapter server.
        if (!mLEDAdapterClient->SetOPSReq(itor->first, mZoneMaps))
        {
            LogD(" SetOPSReq to LED Adapter server failed!!!\n");
            rt = false;
            continue;
        }
    }

    return rt;
}

bool LEDPlayer::timeSync2LEDReq()
{
    bool rt = true;

    std::map<int, LedDevInfo>::const_iterator itor = mLEDDevCfgMap.begin();
    for (; itor != mLEDDevCfgMap.end(); ++itor)
    {
        // send setOPS request to LED adapter server.
        if (!mLEDAdapterClient->DateTimeSyncReq(itor->first))
        {
            LogD(" SetOPSReq to LED Adapter server failed!!!\n");
            rt = false;
            continue;
        }
    }

    return rt;
}

bool LEDPlayer::getLedStatusReq()
{
    bool rt = true;

    std::map<int, LedDevInfo>::const_iterator itor = mLEDDevCfgMap.begin();
    for (; itor != mLEDDevCfgMap.end(); ++itor)
    {
        // send setOPS request to LED adapter server.
        if (!mLEDAdapterClient->GetScreenStatusReq(itor->first))
        {
            LogE(" GetScreenStatusReq to LED Adapter server failed!!!\n");
            rt = false;
            continue;
        }
    }

    return rt;
}

int LEDPlayer::handleScheduleUpdated(void* data)
{
    LogD("LED schedule update.\n");
    Json::LayoutInfo4Qt* fullLayoutInfo = (Json::LayoutInfo4Qt*) data; // after use finished, don't forget to delete it.

    if (LED_OPS_Playing == mStatus)
    { // just store the schedule information. not play right now.
        mCurrentSchedule = fullLayoutInfo;
        LogD("OPS is playing,just store the current schedule!\n");
        return 0;
    }

    LogD(
            "\t\t ---------------- LED handleScheduleUpdated fullLayoutInfo addr=%p.\n",
            fullLayoutInfo);

    if (NULL == fullLayoutInfo)
    {	// clear current screen content.
        // Clear all device screen.
        clearLEDScreenReq();
        mCurrentSchedule = NULL;
        mStatus = LED_Idle;

        LogD(
                "\t\t ---------------- LED schedule clear current screen content.\n");

        return 0;
    }

    mCurrentSchedule = fullLayoutInfo; // store current playing schedule info.
    mZoneMaps.clear();

    if (fullLayoutInfo->layoutDtl.mPartitions.size() > 4)
    {
        LogE("LED Schedule only support 4 zone at most!\n");
    }

    // get all partition info.
    std::vector<Json::PartitionDetail>::const_iterator ptDtlItor =
            fullLayoutInfo->layoutDtl.mPartitions.begin();
    int zoneId = 0;
    ZoneInfo singleZone;

    for (; ptDtlItor != fullLayoutInfo->layoutDtl.mPartitions.end();
            ++zoneId, ++ptDtlItor)
    {
        if (zoneId < MAX_ZONE_CNT) // only get the first 4 zone at most!
        {
            singleZone.zoneId = zoneId;
            singleZone.width = ptDtlItor->mWidth;
            singleZone.height = ptDtlItor->mHeight;
            singleZone.xPos = ptDtlItor->mXpos;
            singleZone.yPos = ptDtlItor->mYpos;

            int ptId = ptDtlItor->mId; // partition id
            Json::LayoutInfo4Qt::MediaContents::const_iterator mediaItor =
                    fullLayoutInfo->mPartitonInfos[ptId].begin();

            if (mediaItor != fullLayoutInfo->mPartitonInfos[ptId].end())
            {

                if (dynamic_cast<Json::MediaText*>(mediaItor->second))
                {
                    Json::MediaText* theText =
                            dynamic_cast<Json::MediaText*>(mediaItor->second);
                    singleZone.speed =
                            (ZoneInfo::ZoneSpeed) theText->mParams.mSpeed;
                    singleZone.backColor = theText->mParams.mBackColor;
                    singleZone.foreColor = theText->mParams.mForeColor;
                    singleZone.effect =
                            (ZoneInfo::ZoneEffect) theText->mParams.mEffect;
                    singleZone.dataStr = theText->mParams.mContent;
                    singleZone.bTimeZone = false;
                } else if (dynamic_cast<Json::MediaDigitalClock*>(mediaItor->second))
                {
                    Json::MediaDigitalClock* digitalClock =
                            dynamic_cast<Json::MediaDigitalClock*>(mediaItor->second);

                    if (digitalClock->mParams.size() < 1)
                        return -1;

                    singleZone.speed =
                            (ZoneInfo::ZoneSpeed) digitalClock->mParams[0].mVarText.mText.mSpeed;
                    singleZone.backColor =
                            digitalClock->mParams[0].mVarText.mText.mBackColor;
                    singleZone.foreColor =
                            digitalClock->mParams[0].mVarText.mText.mForeColor;
                    singleZone.effect =
                            (ZoneInfo::ZoneEffect) digitalClock->mParams[0].mVarText.mText.mEffect;
                    singleZone.dataStr = "";
                    singleZone.bTimeZone = true;
                } else
                {
                    LogE(
                            "The LED media only support Text type or Digital clock!\n");
                    return -1;
                }

                mZoneMaps.insert(std::make_pair(zoneId, singleZone)); // add zoneInfo to zone map.
            }
        } else
            // only get the first 4 zone at most!
            break;
    }

    // send setSchedule2LED request to led adapter server.
    if (!setSchedule2LEDReq())
    {
        LogE(" setSchedule2LEDReq to led adapter server failed!!\n");
        return -1;
    }

    mStatus = LED_Schedule_Playing;

    return 0;
}

bool LEDPlayer::GetAllLedDevInfo()
{
    if (NULL == mLCDController)
        return false;

    ConfigParser* cfg = mLCDController->getConfig();

    if (NULL == cfg)
        return false;

    std::map<std::string, int>::const_iterator iterAddr =
            cfg->mDeviceAddrMap.begin();
    std::map<std::string, int>::const_iterator iterScreenCnt =
            cfg->mDeviceScreenCntMap.begin();
    std::map<std::string, int>::const_iterator iterWidth =
            cfg->mDeviceWidthMap.begin();
    std::map<std::string, int>::const_iterator iterHeight =
            cfg->mDeviceHeightMap.begin();

    std::map<std::string, std::string>::const_iterator iterPort =
            cfg->mDevicePortMap.begin();
    std::map<std::string, int>::const_iterator iterBaud =
            cfg->mBaudRateMap.begin();
    std::map<std::string, int>::const_iterator iterDataBits =
            cfg->mDataBitsMap.begin();
    std::map<std::string, int>::const_iterator iterStopBits =
            cfg->mStopBitsMap.begin();
    std::map<std::string, int>::const_iterator iterParity =
            cfg->mParityMap.begin();
    std::map<std::string, int>::const_iterator iterFlowctrl =
            cfg->mFlowctrlMap.begin();

    std::string ledDevName("");
    LedDevInfo singDevCfg;
    for (; iterAddr != cfg->mDeviceAddrMap.end(); ++iterAddr)
    {
        ledDevName = iterAddr->first;

        singDevCfg.mAddr = iterAddr->second; // LED address
        singDevCfg.mDevName = ledDevName;

        iterScreenCnt = cfg->mDeviceScreenCntMap.find(ledDevName);
        if (iterScreenCnt != cfg->mDeviceScreenCntMap.end())
            singDevCfg.mScreenType = iterScreenCnt->second;

        iterWidth = cfg->mDeviceWidthMap.find(ledDevName);
        if (iterWidth != cfg->mDeviceWidthMap.end())
            singDevCfg.mLedWidth = iterWidth->second;

        iterHeight = cfg->mDeviceHeightMap.find(ledDevName);
        if (iterHeight != cfg->mDeviceHeightMap.end())
            singDevCfg.mLedHeight = iterHeight->second;

        iterPort = cfg->mDevicePortMap.find(ledDevName);
        if (iterPort != cfg->mDevicePortMap.end())
            singDevCfg.mDevicePort = iterPort->second;

        iterBaud = cfg->mBaudRateMap.find(ledDevName);
        if (iterBaud != cfg->mBaudRateMap.end())
            singDevCfg.mBaudRate = iterBaud->second;

        iterDataBits = cfg->mDataBitsMap.find(ledDevName);
        if (iterDataBits != cfg->mDataBitsMap.end())
            singDevCfg.mDataBits = iterDataBits->second;

        iterStopBits = cfg->mStopBitsMap.find(ledDevName);
        if (iterStopBits != cfg->mStopBitsMap.end())
            singDevCfg.mStopBits = iterStopBits->second;

        iterParity = cfg->mParityMap.find(ledDevName);
        if (iterParity != cfg->mParityMap.end())
            singDevCfg.mParity = iterParity->second;

        iterFlowctrl = cfg->mFlowctrlMap.find(ledDevName);
        if (iterFlowctrl != cfg->mFlowctrlMap.end())
            singDevCfg.mFlowctrl = iterFlowctrl->second;

        mLEDDevCfgMap[singDevCfg.mAddr] = singDevCfg;
    } // end of for()

    return true;
}

std::string LEDPlayer::GetDeviceString(int devAddr)
{
    std::map<int, LedDevInfo>::const_iterator itor = mLEDDevCfgMap.find(
            devAddr);
    if (itor != mLEDDevCfgMap.end())
        return itor->second.mDevName;

    return "";
}

const char *LEDPlayer::TAG = "LEDPlayer";

int LEDPlayer::handleCommandUpdated(void* data)
{
    ConfigParser* cfg = mLCDController->getConfig();
    Command* cmd = static_cast<Command*>(data);

    if (NULL == cmd || NULL == cfg)
        return -1;

    LogD("LED command update.\n");
    mLEDCommandMaps[cmd->GetCmdId()] = cmd;

    // get all LED targets of the command.
    std::map<std::string, Command::CommandExecResult>::const_iterator itor =
            cmd->mAvailableLEDCmdMap.begin();
    std::map<std::string, int>::const_iterator devAddrItor =
            cfg->mDeviceAddrMap.begin();
    for (; itor != cmd->mAvailableLEDCmdMap.end(); ++itor)
    {
        // find LED Address according to LED device string id.
        devAddrItor = cfg->mDeviceAddrMap.find(itor->first);
        if (devAddrItor != cfg->mDeviceAddrMap.end())
        {
            int devAddr = devAddrItor->second;

            // judge the command type.
            if (dynamic_cast<StartUp*>(cmd))
            {
                // use LED adapter client to send command to LED adapter server!!!
                mLEDAdapterClient->ScreenOnOffReq(cmd->GetCmdId(), devAddr,
                        true);
            } else if (dynamic_cast<ShutDown*>(cmd))
            {
                mLEDAdapterClient->ScreenOnOffReq(cmd->GetCmdId(), devAddr,
                        false);
            } else if (dynamic_cast<Brightness*>(cmd))
            {
                Brightness* brightnessCmd = dynamic_cast<Brightness*>(cmd);
                mLEDAdapterClient->SetBrightNessReq(cmd->GetCmdId(), devAddr,
                        brightnessCmd->GetBrightness());
            }
        } // end of if(devAddrItor)
    } // end of for loop(mAvailableLEDCmdMap)

    return 0;
}

bool LEDPlayer::clearLEDScreenReq()
{
    mZoneMaps.clear();

    bool rt = true;
    std::map<int, LedDevInfo>::const_iterator itor = mLEDDevCfgMap.begin();
    for (; itor != mLEDDevCfgMap.end(); ++itor)
    {
        if (!mLEDAdapterClient->ClearScreenReq(itor->first))
        {
            rt = false;
            continue;
        }
    }

    return rt;
}

bool LEDPlayer::setSchedule2LEDReq()
{
    bool rt = true;

    std::map<int, LedDevInfo>::const_iterator itor = mLEDDevCfgMap.begin();
    for (; itor != mLEDDevCfgMap.end(); ++itor)
    {
        if (!mLEDAdapterClient->SetSchContentReq(itor->first, mZoneMaps))
        {
            rt = false;
            continue;
        }
    }

    return rt;
}

int LEDPlayer::handleAdaperCmdReply(void* data)
{
    LEDMsgData* msgData = static_cast<LEDMsgData*>(data);
    if (NULL == msgData)
    {
        LogW(" LEDMsgData is NULL!!\n ");
        return -1;
    }

    int cmdId = msgData->mCmdId;
    int devAddr = msgData->mAddr;
    LEDAdapterIPCall::ExecuteRslt cmdRslt = msgData->mCmdExeRslt;

    DELETE_ALLOCEDRESOURCE(msgData); // Release!!!!

    // find the command in command map according to command id.
    std::map<int, Command*>::iterator cmdItor = mLEDCommandMaps.find(cmdId);
    if (cmdItor == mLEDCommandMaps.end())
    { // the command id not found!
        LogW(" Got Cmd reply, buf cannot find command id in command map!!!\n");
        return -1;
    }

    // found the command id in command map.
    Command* theCmd = cmdItor->second;
    if (NULL == theCmd)
    {
        LogW(" Command is NULL!!!\n");
        return -1;
    }

    // according to device address to get the device string id.
    std::string devNameStr = GetDeviceString(devAddr);
    if (devNameStr.empty())
    {
        LogW(" Can not find the device according to address!!\n");
        return -1;
    }

    if (theCmd->setCommandExecStatus(devNameStr,
            (Command::CommandExecResult) cmdRslt))
    { // all targets device of the command execute finished!!
        mLEDCommandMaps.erase(cmdItor);
    }

    // TODO: some problem in CMD_LEDReply!!! Need to be fixed!
    // PS: For one command, each target device generate a reply.
    theCmd->sendMessage(new Message(Command::CMD_LEDReply));
    return 0;
}

int LEDPlayer::handleGetDevStatus()
{
    // send get device status request to LED adapter server.
    if (!getLedStatusReq())
    {
        LogI("getLedStatusReq to LED adapter server failed!.\n");

        handleServerUnReachable();
        return -1;
    }

    return 0;
}

void LEDPlayer::updateDevScreenStatus(int devAddr, int screenStatus)
{
    LogI("Update LED[%d] screen status[%d].\n", devAddr, screenStatus);

    // find the device.
    std::map<int, LedDevInfo>::iterator itor = mLEDDevCfgMap.find(devAddr);
    if (itor != mLEDDevCfgMap.end())
    {
        itor->second.mDevStatus = (Json::HardwareStatus::HdStatus) screenStatus;
    }
}

void LEDPlayer::handleServerUnReachable()
{
    std::map<int, LedDevInfo>::iterator itor = mLEDDevCfgMap.begin();

    for (; itor != mLEDDevCfgMap.end(); ++itor)
        itor->second.mDevStatus = Json::HardwareStatus::S_OFF_LINE;

    if (mLEDAdapterClient->peerConnected())
    {
        getLooper()->removeFd(mLEDAdapterClient->getSocketFd());
        mLEDAdapterClient->closeSocket();
    }

    // try re-connect
    sendMessage(new Message(LED_Adapter_Connect), 3000);
}

void LEDPlayer::GetDevStatus(
        std::map<std::string, Json::HardwareStatus::HdStatus>& ledStatusMap)
{
    ledStatusMap.clear();

    std::map<int, LedDevInfo>::iterator itor = mLEDDevCfgMap.begin();
    for (; itor != mLEDDevCfgMap.end(); ++itor)
    {
        ledStatusMap.insert(
                std::make_pair(itor->second.mDevName,
                        (Json::HardwareStatus::HdStatus) itor->second.mDevStatus));
    }

    return;
}
