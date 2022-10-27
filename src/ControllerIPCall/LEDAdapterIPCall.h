/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : LEDAdapterIPCall.h
 * @author : Benson
 * @date : Dec 12, 2017
 * @brief :
 */

#ifndef SRC_LEDADAPTERIPCALL_H_
#define SRC_LEDADAPTERIPCALL_H_

#include <Parcelable.h>
#include <string>
#include <map>

#include "IPCall.h"
#include "IPCallID.h"

class LedDevInfo: public Parcelable
{
public:
    typedef enum
    {
        OneScreen = 1, TwoScreen = 2,
    } ScreenType;

    LedDevInfo();

    virtual int writeToParcel(Parcel &out) const;
    virtual int readFromParcel(Parcel &in);

    std::string mDevName;
    int mAddr; // device address
    int mScreenType; // screen type
    int mLedWidth;
    int mLedHeight;
    int mZoneCnt;

    std::string mDevicePort;
    int mBaudRate;
    int mDataBits;
    int mStopBits;
    int mParity;
    int mFlowctrl;

    int mDevStatus;  // 0 screen on, 1 screen off, 2 off-line
};

class ZoneInfo : public Parcelable
{
public:
    typedef enum
    {
      VeryFast = 0, Fast = 1, Normal = 2, Slow = 3, VerySlow = 4,
    }ZoneSpeed;

    typedef enum
    {
        Static = 0, LeftScroll = 1, RightScroll = 2, UpScroll = 3
    }ZoneEffect;

    ZoneInfo();

    virtual int writeToParcel(Parcel &out) const;
    virtual int readFromParcel(Parcel &in);

    int zoneId;
    ZoneSpeed speed;
    std::string foreColor;
    std::string backColor;
    ZoneEffect effect;
    std::string dataStr;
    int xPos;
    int yPos;
    int width; // width of a single zone
    int height;// height of a single zone
    bool bTimeZone; // weather the zone is use to show time.
};



class LEDAdapterIPCall: public IPCall
{
public:
    LEDAdapterIPCall();
    LEDAdapterIPCall(int fd, const std::string &callerName);
    ~LEDAdapterIPCall();

    typedef enum
    {
        Exe_ready2Exec, Exe_Succeed, Exe_Failed
    } ExecuteRslt;

    enum IPCallID
    {
        // 0 is ping; 1 is ping reply
        ID_SetDeviceInfoReq = 2,
        ID_SetDeviceInfoRply,
        ID_ScreenOnReq,
        ID_ScreenOnCtrlRply,
        ID_ScreenOffReq,
        ID_ScreenOffCtrlRply,
        ID_SetBrightNessReq,
        ID_SetBrightNessRply,

        ID_DateTimeSyncReq,
        ID_ClearScreenReq,

        ID_SetScheduleReq,
        ID_SetOPSReq,
        ID_SetOPSRply,
        ID_GetScreenStatusReq,
        ID_GetScreenStatusRply,
    };
};

class LEDMsgData
{
public:
    LEDMsgData(int fd = -1, int addr = -1, int cmdVal = 0, std::string datafmt =
            "", std::string dataContent = ""/*, LEDCommandType cmdType =
            CMD_Invalid*/) :
            mFd(fd), mAddr(addr), mCmdId(-1), mCmdVal(cmdVal),mScreenStatus(2),/* mCmdType(
                    cmdType),*/ mCmdExeRslt(LEDAdapterIPCall::Exe_Failed)
    {

    }

    int mFd;
    int mAddr; // address of LED;
    int mCmdId;
    int mCmdVal;
    int mScreenStatus; // 0-screen on; 1-screen off; 2-screen offline
    //LEDCommandType mCmdType;
    LEDAdapterIPCall::ExecuteRslt mCmdExeRslt; // the result of cmd execute result
    std::map<int,ZoneInfo> mZoneInfos;// key-zoneId value-zoneInfo
};

#endif /* SRC_LEDADAPTERIPCALL_H_ */
