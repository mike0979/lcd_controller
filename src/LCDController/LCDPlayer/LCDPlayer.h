/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef LCDPLAYER_LCDPLAYER_H_
#define LCDPLAYER_LCDPLAYER_H_

#include <serial/SerialHandle.h>
#include "TermiosU.h"
#include <Thread.h>
#include <list>
#include <map>
#include <string>
#include "Mutex.h"

class LCDController;
class ConfigParser;
class Message;

class LCDPlayer:public Handler, public Thread
{
	struct serialParam{
		std::string deviceid;
		std::string name;
		int bandrate = 9600;
		int databits = 8;
		int stopbits = 1;
		Termios::ParityCheckMode paritycheck = Termios::PC_None;
		Termios::FlowControlMode flowcontrol = Termios::FC_None;
	};

public:

    enum
    {
        LCD_GetStatus=7788,
		LCD_SetScreen,
		LCD_CaluPoweronofftime,
		LCD_Poweronofflcd,
		LCD_OpenSerialPort,
		LCD_ReOpenSerialPort,
		LCD_WriteSerialPort,
		LCD_CloseSerialPort,
		LCD_ScreenOnOffTimeUpdated,
    };

	LCDPlayer(LCDController* lcdcontroller);
	~LCDPlayer();

	bool getLCDDeviceStatus(std::map<std::string, Json::HardwareStatus::HdStatus>& statusmap);
private:
	virtual void run();
	virtual bool handleMessage(Message *msg);
	int handleGetStatus();
	int handleSetScreen();
	int handleCaluPoweronoffTime();
	int handlePoweronoffLCD(bool bshutdown);
	int sendLCDCommand(void* data, int len);

	LCDController* mLCDController;
	ConfigParser* mCfg;
	std::list<SerialHandle::serialParam> mSerialDevParamList;
	std::map<std::string, Json::HardwareStatus::HdStatus> mDeviceStatusMap;

	std::string mScreenPowerOnTime;
	std::string mScreenShutdownTime;
	bool mLCDPanelShutdownStatus;
	bool mLCDPanelPoweronStatus;

	Termios mTermiosDev;

	int m_OnoffLCD_retryCount_;

	bool mPAMsgReceived;

	static const char *TAG;
};

#endif /* LCDPLAYER_LCDPLAYER_H_ */
