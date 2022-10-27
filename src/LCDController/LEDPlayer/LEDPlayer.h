/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file name : comment_example.h
 * @author : alex
 * @date : 2017/7/14 14:29
 * @brief : open a file
 */
#ifndef LEDPLAYER_LEDPLAYER_H_
#define LEDPLAYER_LEDPLAYER_H_

#include <command/Command.h>
#include <json/DeviceStatusObj.h>
#include <LEDAdapterIPCall.h>
#include <Mutex.h>
#include <Thread.h>
#include <map>
#include <string>

namespace Json
{
class OPSMsgDetail;
} /* namespace Json */

class LEDAdapterClient;

class LCDController;

class Message;

class LEDPlayer: public Handler, public Thread
{
public:
    LEDPlayer(LCDController* lcdcontroller);

    ~LEDPlayer();

    enum
    {
        LED_Adapter_Connect = 1000,
        LED_Adapter_DisConnect,
        LED_Adapter_SetDevInfos,

        LED_OPSMsgUpdated,
        LED_OPSMsgFinished,

        LED_ScheduleUpdated,
        LED_ExecuteCmd,
        LED_CMD_AdapterReply, // the reply from LED adapter server
        LED_OPS_AdapterReply, // the reply from LED adapter server
        LED_DateSync,
        LED_GetDevStatus,
        LED_ScreenStatus_AdapterReply, // the reply from LED adapter server
    };

    typedef enum
    {
        LED_Idle,LED_OPS_Playing,LED_Schedule_Playing,
    }PlayerStatus;

    /**
     * Get all led devices status.
     * @param ledStatusMap[in,out]: take out led status map.
     */
    void GetDevStatus(std::map<std::string,Json::HardwareStatus::HdStatus>& ledStatusMap);
private:
    virtual void run();
    virtual bool handleMessage(Message *msg);

    bool handleConnectAdapter();

    bool handleSetDeviceInfo();


    int handleOPSUpdated(void* data, int status);
    int handleScheduleUpdated(void* data);

    /**
     * Forward the command to LED adapter server.
     * @param data
     * @return
     */
    int handleCommandUpdated(void* data);

    /**
     * Forward the command execute result received
     * from LED adapter server.
     * @param data
     * @return
     */
    int handleAdaperCmdReply(void* data);

    /**
     * Send clear LED devices screen request to
     * LED adapter server.
     * @return
     */
    bool clearLEDScreenReq();

    bool setSchedule2LEDReq();

    bool setOPS2LEDReq();

    bool timeSync2LEDReq();

    bool getLedStatusReq();

    int handleGetDevStatus();

    void updateDevScreenStatus(int devAddr,int screenStatus);

    /**
     * Handle the adapter server unreachable.
     * set all led device off-line and reconnect to the adapter server.
     */
    void handleServerUnReachable();

private:
    /**
     * Read the LED device related configuration from configure file.
     * @return
     */
    bool GetAllLedDevInfo();

    /**
     * Get device string according to device address.
     * @param devAddr: device address.
     * @return device name string.
     */
    std::string GetDeviceString(int devAddr);

    /**
     * Get the current ops period from now on.(in milliseconds).
     * @param dtl
     * @param endtime: bring out the period of current ops.
     */
    void CalculateOPSEndTime(const Json::OPSMsgDetail* dtl,unsigned& period);

private:
    LEDAdapterClient *mLEDAdapterClient;
    // key: LED address, value: the LED configuration information.
    std::map<int,LedDevInfo> mLEDDevCfgMap;

    std::map<int,Command*> mLEDCommandMaps; // store the commands from LCDController.

    // key: zone id, value: zone information.
    std::map<int,ZoneInfo> mZoneMaps;
    Mutex mMutex;

    LCDController* mLCDController;
    Json::OPSMsgDetail* mCurrentOPS;
    void* mCurrentSchedule; // pointer to current playing schedule.
    PlayerStatus mStatus; // current LED player status.
    static const char *TAG;
};

#endif /* LEDPLAYER_LEDPLAYER_H_ */
