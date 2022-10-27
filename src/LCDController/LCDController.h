/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : LCDController.h
* @author : wangfuqiang
* @date : 2017/8/8 14:29
* @brief :
*/
#ifndef LCDCONTROLLER_H_
#define LCDCONTROLLER_H_

#include <Handler.h>
#include <string>
#include "CommonDef.h"

class ConfigParser;
class QtPanel;
class LEDPlayer;
class LCDPlayer;
class TransManager;

class WebSocketNotify;
class NtpDate;

class LCDController : public Handler{

public:
	enum MessageType {
		ParseConfigFile = 0,
		WebSocketPingReq,
		WebSocketPingMonitor,
		WebSocketReConnect,

		OPSMsgUpdated,
		LoginSucceed,
		RefreshToken,
		LayoutUpdated,
		ArrivalInfoUpdated,
		TrainTimeUpdated,
		ScreenOnOffTimeUpdated,
		LiveSourceSwitchUpdated,
		LEDCmdUpdated,
		ArriMsgBlockDisplayed,  //notify to download rtwrrmsg from server
		NtpDateSync,
		HouseKeeping,
	};

public:
	LCDController();
	~LCDController();

	static LCDController* GetInstance();

	ConfigParser* getConfig();

	std::string GetDevId();

	TransManager* GetTransManager();

	LCDLEDControllerFlag getControllerFlag();

	LEDPlayer* GetLEDPlayer();
	LCDPlayer* getLCDPlayer();
private:
	static LCDController* pInstance;
	virtual bool handleMessage(Message *msg);
	void createDirectory();

	LCDLEDControllerFlag mControllerFlag;
	ConfigParser* mConfig;
	TransManager* mTransManager;
	WebSocketNotify* mWsNotify;
	QtPanel* mQtPanel;
	LEDPlayer* mLEDPlayer;
    LCDPlayer* mLCDPlayer;
    NtpDate *mNtpDate;
	std::string mDevId;

	int mLostPingCnt; // the count of ping lost from server.
	static const char *TAG;
};

#endif /* LCDCONTROLLER_H_ */
