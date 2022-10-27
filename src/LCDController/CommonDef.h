/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : CommonDef.h
 * @author : Benson
 * @date : Sep 21, 2017
 * @brief :
 */

#ifndef COMMONDEF_H_
#define COMMONDEF_H_

#include <json/DeviceStatusObj.h>
#include "Mutex.h"

#define DELETE_ALLOCEDRESOURCE(obj)		do {		  \
				if(NULL != obj)	                      \
				{                                     \
                     delete (obj);                    \
                     obj = NULL;                      \
				}                                     \
			} while (0)

#define MODULEPROCESSNAME "LCDController"

const int LCD_LED_prefix_len = 12;
#define LCDPanel_DeviceID_prefix "lcd_monitor_"
#define LED_DeviceID_prefix      "led_monitor_"

#define LANGUAGE_CH "CHS"
#define LANGUAGE_EN "EN"

typedef enum
{
    LOG_ToConsole = 0,
	LOG_ToFile = 1,
}LOGDriectFlag;

typedef enum
{
    LCD_Controller_flag = 0,
    LED_Controller_flag = 1,
	Unknown_controller = 2,
}LCDLEDControllerFlag;

typedef enum
{
    SNAPSHOT_Function_disable = 0,
	SNAPSHOT_Function_enable = 1,
}SNAPSHOTFunctionFlag;

typedef enum
{
    NTPDATE_Function_disable = 0,
	NTPDATE_Function_enable = 1,
}NTPDATEFunctionFlag;

/**
 * Download or Upload types
 */
typedef enum
{
    DL_SchedulesUpdateListReq = 0, //!< DL_SchedulesUpdateList
	DL_SchedulesUpdateList,
    DL_Schedules,              //!< DL_Schedules
    DL_LayoutGroups,
    DL_Medias,                 // media detail
    DL_MediaMD5,               // media information,
    DL_MediaContent,           //!< DL_MediaContent
    NT_ScheduleUpdatedNotify,     //
    NT_UpdateSchedule,
    NT_UpdateLayoutGroup,
    NT_UpdateLayout,

    DL_CmdsUpdateList,         //!< DL_CmdsUpdateList
    DL_CmdRequest,             //!< DL_CmdRequest
    UP_CmdExeReply,            //!< UP_CmdExeReply
    DL_OPMsgList,              //!< DL_OPMsgList
    DL_OPMsgRequest,           //!< DL_OPMsgRequest
    DL_OPSMsgUpdatedNotify ,
    DL_OPSBackImage,
    UP_OPSReply, // notify opshandler msg
	UP_OPSReplyResult,  //upload opsreply to server msg

    DL_RealTimeWeather,        //!< DL_RealTimeWeather
    DL_RealTimeArrMsgList,     //!< DL_RealTimeArrMsgList
    DL_RealTimeArrMsg,         //!< DL_RealTimeArrMsg
    DL_LoadFromLocal,          //!< DL_LoadFromLocal
    DL_ArrivalInfo,
	DL_CheckArrivalInfo,
	DL_TrainTimeConfig,
	DL_TrainTimeLoadFromLocal,
	DL_ReDownloadTrainTimeConfig,
	DL_ScreenOnOffConfig,
	DL_LiveSwithConfig,
    UP_SnapShot,
    UP_SwLog,
    UP_DsplyLog,
} TransFileType;

typedef enum
{
    F_ICON = 1,
    F_MEDIA = 2,
    F_SOFTWARE_LOG = 3,
    F_DISPLAY_LOG = 4,
    F_SNAPSHOT = 5,
    F_CONFIG = 6,
    F_INSTALL_PACKAGE = 7,
}UpLoadFileType;

typedef enum
{
    WebSocketPingReq = 1000,

    LoadLocalSchedule,
    LoginReq,
    LoginReply,
    RefreshTokenReq,
    RefreshTokenReply,
    DownLoadReq,
    ExecuteCmd,
    OPSMsgUpdated,
    LayoutUpdated,
    ArrivalInfoUpdated,
	TrainTimeUpdated,
	ScreenOnOffTimeUpdated,
	LiveSourceSwitchUpdated,
	LEDCmdUpdated,
    InitDeviceStatus,
    RptDevStatusReq,  // report device status request
    RptDevStatusReply, // reply of 'report device status'
	ArrMsgBlockDisplayed, //notify to download rtwrrmsg from server
} TransMessageType;

typedef enum
{
    T_MainDev = 0,
    T_LCD_Monitor = 1,
    T_SignalTransmitter = 2,
    T_SignalReceiver = 3,
    T_Unkown = 0xFF
} DevType;

typedef enum {
    UnknownType = 0,
    NTF_WS_ServerPING = 1,
    NTF_CommandUpdated = 1008,
    NTF_RTOPSMsgUpdated = 1009,
    NTF_ScheduleUpdated = 1015,
    NTF_RTWeatherUpdated = 1018,
    NTF_RTArrMsgUpdated = 1017,
	NTF_TrainTimeUpdated = 1019,
	NTF_ScreenOnOffUpdated = 1020,
	NTF_LiveSourceUpdated = 1021,
}NotifyMessageCode;

typedef enum
{
    MSG_CancelCmd,
    MSG_ExecuteCmd,
    MSG_CmdCompleted,
	MSG_CmdResult,

	MSG_CMDToLED,
	MSG_CMDLEDReply,

    MSG_CMD_StartUp,
    MSG_CMD_ShutDown,
    MSG_CMD_ReBoot,
    MSG_CMD_GetLog,
    MSG_CMD_GetSnapShot,
    MSG_CMD_HouseKeeping,
	MSG_CMD_SetBrightness,
	MSG_CMD_SetVolumn,
	MSG_CMD_SetVolumeMute,
	MSG_CMD_ExecuteResult,
    MSG_CMD_Unknown,
}CommandMsgType;

typedef enum
{
	ReadtoRun = 0,
	RunSuccess = 1,
	RunFailed = 2,
}CommandExecuteReplyVal;

typedef enum
{
    QueueLoop = 1,
    OverWrite = 2,
    JustWait = 3,
}OPSQueueFlag;

typedef enum
{
    OPS_NotReceived = 0,
    OPS_Reveived = 1,
    OPS_Playing = 2,
    OPS_PlayFinished = 3,
	//OPS_PlayWithDrawed = 4,
}OPSStatus;

typedef enum
{
	OPS_addstatus = 0,
    OPS_updatestatus,
    OPS_deletestatus,
	OPS_noopsstatus,
	OPS_otherstatus,
}OPSUpdateStatus;

typedef enum
{
	OPS_PriorityEmergency = 1,
	OPS_PriorityNotify = 2,
	OPS_PriorityDefault = 3,
}OPSMsgPriority;

typedef enum
{
	SpeedLevel_0 = 100,
	SpeedLevel_1 = 90,
	SpeedLevel_2 = 70,
	SpeedLevel_3 = 50,
	SpeedLevel_4 = 30,
}ScrollTextSpeed;

typedef enum
{
	LIVE_AutoPlay = 0,
	LIVE_PlayStream = 1,
	LIVE_PlayLocal = 2,
}LiveSourceSwitchConfig;

extern Json::DeviceStatus g_LCDCtrlerDevStatus;
extern Json::DeviceStatus g_SubDev1DevStatus;
extern Mutex g_MutexForLCDSerialAccess;
#endif /* COMMONDEF_H_ */
