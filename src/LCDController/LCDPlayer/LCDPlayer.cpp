/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#include "LCDPlayer.h"
#include <config/configparser.h>
#include <CommonDef.h>
#include <LCDController.h>
#include "Log.h"
#include <errno.h>
#include "SystemClock.h"

Mutex g_MutexForLCDSerialAccess;

LCDPlayer::LCDPlayer(LCDController* lcdcontroller): mLCDController(lcdcontroller)
{
	mCfg = lcdcontroller->getConfig();
	mSerialDevParamList.clear();

	if(mCfg != NULL)
	{
		for(std::map<std::string, std::string>::const_iterator it = mCfg->mDevicePortMap.begin();
				it != mCfg->mDevicePortMap.end();++it)
		{
			SerialHandle::serialParam param;
			param.name = it->second;
			param.deviceid = it->first;
			std::map<std::string,int>::const_iterator it1 = mCfg->mBaudRateMap.find(it->first);
			if(it1 != mCfg->mBaudRateMap.end())
				param.bandrate = it1->second;

			it1 = mCfg->mDataBitsMap.find(it->first);
			if(it1 != mCfg->mDataBitsMap.end())
				param.databits = it1->second;

			it1 = mCfg->mDataBitsMap.find(it->first);
			if(it1 != mCfg->mDataBitsMap.end())
				param.databits = it1->second;

			it1 = mCfg->mStopBitsMap.find(it->first);
			if(it1 != mCfg->mStopBitsMap.end())
				param.stopbits = it1->second;

			it1 = mCfg->mParityMap.find(it->first);
			if(it1 != mCfg->mParityMap.end())
				param.paritycheck = (Termios::ParityCheckMode)(it1->second);

			it1 = mCfg->mFlowctrlMap.find(it->first);
			if(it1 != mCfg->mFlowctrlMap.end())
				param.flowcontrol = (Termios::FlowControlMode)(it1->second);

			mSerialDevParamList.push_back(param);
		}
	}

	mScreenPowerOnTime = mCfg->mPoweronLcdTime;
	mScreenShutdownTime = mCfg->mShutdownLcdTime;

	mLCDPanelShutdownStatus = false;
	mLCDPanelPoweronStatus = false;

	m_OnoffLCD_retryCount_ = 0;

	mPAMsgReceived = false;
}

LCDPlayer::~LCDPlayer()
{
	for(std::list<SerialHandle::serialParam>::iterator iter = mSerialDevParamList.begin() ;
				 iter != mSerialDevParamList.end();++iter)
	{
		if(iter->termios.getFD()>=0)
		{
			iter->termios.close();
		}

		mDeviceStatusMap[iter->deviceid] = Json::HardwareStatus::S_OFF_LINE;
	}
}

void LCDPlayer::run()
{
    Looper *mlooper = Looper::CreateLooper();
    setLooper(mlooper);

    sendMessage(new Message(LCD_OpenSerialPort));

    sendMessage(new Message(LCD_GetStatus),3000);

    sendMessage(new Message(LCD_CaluPoweronofftime),3000);
    mlooper->loop();
}

bool LCDPlayer::handleMessage(Message* msg)
{
	switch (msg->mWhat)
	{
	case LCD_OpenSerialPort:
	{
		for(std::list<SerialHandle::serialParam>::iterator iter = mSerialDevParamList.begin() ;
			 iter != mSerialDevParamList.end();++iter)
		{
			mDeviceStatusMap[iter->deviceid] = Json::HardwareStatus::S_ON;

			//open
			LogD("Serialport name :%s\n",iter->name.c_str());
			int fd = iter->termios.open(iter->name, Termios::RW);
			if(fd<0)
			{
				LogD("open lcd device failed-%s\n",iter->name.c_str());
				mDeviceStatusMap[iter->deviceid] = Json::HardwareStatus::S_OFF_LINE;
				continue;
			}

			iter->termios.setBaudRate(iter->bandrate);
			iter->termios.setDataBits(iter->databits);
			iter->termios.setStopBits(iter->stopbits);
			iter->termios.setParityCheck((Termios::ParityCheckMode)(iter->paritycheck));
			iter->termios.setFlowControl((Termios::FlowControlMode)(iter->flowcontrol));
		}

		break;
	}
	case LCD_ReOpenSerialPort:
	{
		for(std::list<SerialHandle::serialParam>::iterator iter = mSerialDevParamList.begin() ;
					 iter != mSerialDevParamList.end();++iter)
		{
			LogD("Reopen serial port :%s\n",iter->name.c_str());
			if(iter->termios.getFD()>=0)
			{
				iter->termios.close();
			}

			mDeviceStatusMap[iter->deviceid] = Json::HardwareStatus::S_ON;

			//open
			int fd = iter->termios.open(iter->name, Termios::RW);
			if(fd<0)
			{
				LogD("open lcd device failed-%s\n",iter->name.c_str());
				mDeviceStatusMap[iter->deviceid] = Json::HardwareStatus::S_OFF_LINE;
				continue;
			}

			iter->termios.setBaudRate(iter->bandrate);
			iter->termios.setDataBits(iter->databits);
			iter->termios.setStopBits(iter->stopbits);
			iter->termios.setParityCheck((Termios::ParityCheckMode)(iter->paritycheck));
			iter->termios.setFlowControl((Termios::FlowControlMode)(iter->flowcontrol));

			sendLCDCommand(msg->mData,msg->mArg1);
		}

		break;
	}
	case LCD_CloseSerialPort:
	{
		for(std::list<SerialHandle::serialParam>::iterator iter = mSerialDevParamList.begin() ;
					 iter != mSerialDevParamList.end();++iter)
		{
			if(iter->termios.getFD()>=0)
			{
				iter->termios.close();
			}

			mDeviceStatusMap[iter->deviceid] = Json::HardwareStatus::S_OFF_LINE;
		}

		break;
	}
	case LCD_WriteSerialPort:
	{
		if(msg->mArg2 == 101)
		{
			LogD("---- PA msg trigger ---- \n");
			mPAMsgReceived = true;
		}
		else if(msg->mArg2 == 102)
		{
			LogD("---- PA msg cancel ---- \n");
			mPAMsgReceived = false;

			if(mLCDPanelShutdownStatus && !mPAMsgReceived)
			{
				LogD("---- PA msg cancel, do not poweron LCD Panel ---- \n");
				break;
			}
		}

		sendLCDCommand(msg->mData,msg->mArg1);

		break;
	}
	case LCD_GetStatus:
	{
		handleGetStatus();

		//sendMessage(new Message(LCD_GetStatus),mCfg->mStatusRptPeriod*1000);
		break;
	}
	case LCD_SetScreen:
	{
		handleSetScreen();

		break;
	}
	case LCD_CaluPoweronofftime:
	{
		if(mCfg->mOnOffLCDPanelEnable != 0)
		{
			if(!mPAMsgReceived)
			{
				handleCaluPoweronoffTime();
			}
			else
			{
				LogD("--- PA trigger period, don't auto power on-off LCD panel\n");
			}

			sendMessage(new Message(LCD_CaluPoweronofftime),3000);
		}
		break;
	}
	case LCD_Poweronofflcd:
	{
		LogD("Start to shutdown or trun on LCD panel: %d\n",msg->mArg1);
		handlePoweronoffLCD(msg->mArg1);

		break;
	}
	case LCD_ScreenOnOffTimeUpdated:
	{
		mCfg = mLCDController->getConfig();
		mScreenPowerOnTime = mCfg->mPoweronLcdTime;
		mScreenShutdownTime = mCfg->mShutdownLcdTime;
		break;
	}
	}

	return true;
}


int LCDPlayer::sendLCDCommand(void* data, int len)
{
	unsigned char *databuf = (unsigned char *)data;
	for(int i=0;i<len;++i)
	{
		LogD("------- lcd serial data : %02x\n",databuf[i]);
	}
	LogD("------- lcd serial data len : %d\n",len);

	for(std::list<SerialHandle::serialParam>::iterator iter = mSerialDevParamList.begin() ;
		 iter != mSerialDevParamList.end();++iter)
	{
		int sendlen = 0;
		if (TEMP_FAILURE_RETRY(
				sendlen = ::write(iter->termios.getFD(), data, len) != len))
		{
			LogE("write lcd monitor error %s [fd = %d]:%s,  sendlen - %d\n", iter->name.c_str(),
					iter->termios.getFD(), strerror(errno),sendlen);
			iter->termios.close();
			sendMessage(new Message(LCD_ReOpenSerialPort, data, len),1000);
			mDeviceStatusMap[iter->deviceid] = Json::HardwareStatus::S_OFF_LINE;
			return -1;
		}
		LogD("write LCD serialport success %s.\n", iter->name.c_str());

		usleep(1000*400);

		const int READ_LEN = 10;
		unsigned char readbuf[10];
		int recvlen = ::read(iter->termios.getFD(), readbuf, READ_LEN);

		if (recvlen <= 0)
		{
			LogE("Read LCD device status error:%d.\n",errno);
			//mDeviceStatusMap[iter->deviceid] = Json::HardwareStatus::S_OFF_LINE;
		}
		else if(recvlen == READ_LEN)
		{
			if(readbuf[5] == 0x4f && readbuf[6] == 0x4b && readbuf[8] == 0x31)
			{
				mDeviceStatusMap[iter->deviceid] = Json::HardwareStatus::S_ON;
			}
		}
	}

	return 0;
}

bool LCDPlayer::getLCDDeviceStatus(std::map<std::string, Json::HardwareStatus::HdStatus>& statusmap)
{
	statusmap = mDeviceStatusMap;

	return true;
}

int LCDPlayer::handleCaluPoweronoffTime()
{
	std::string sday = SystemClock::Today(SystemClockDateFormat);
	std::string stime = SystemClock::Today(SystemClockTimeFormat);

	bool bShutdownLCD = false;

//	if(stime == mCfg->mShutdownLcdTime)
//	{
//		bShutdownLCD = true;
//		sendMessage(new Message(LCD_Poweronofflcd, (int)bShutdownLCD));
//	}
//	else if(stime == mCfg->mPoweronLcdTime)
//	{
//		bShutdownLCD = false;
//		sendMessage(new Message(LCD_Poweronofflcd, (int)bShutdownLCD));
//	}

	std::string ontime = mScreenPowerOnTime;
	std::string offtime = mScreenShutdownTime;
	if((offtime != ontime) &&
			((ontime > offtime && stime >= offtime && stime < ontime) ||
			(offtime > ontime && ((stime >= offtime && stime <= "240000")||
					stime < ontime))))
	//if(stime >= mCfg->mShutdownLcdTime && stime < mCfg->mPoweronLcdTime)
	{
		bShutdownLCD = true;
		if(!mLCDPanelShutdownStatus)
		{
			removeMessage(LCD_Poweronofflcd);
			sendMessage(new Message(LCD_Poweronofflcd, (int)bShutdownLCD));
			m_OnoffLCD_retryCount_ = 0;
		}
		else
		{
			if(!hasMessage(LCD_Poweronofflcd) && m_OnoffLCD_retryCount_++ < 5)
			{
				LogD("retry to shutdown lcd pane, count: \n",m_OnoffLCD_retryCount_);
				sendMessage(new Message(LCD_Poweronofflcd, (int)bShutdownLCD), 10 * 1000);
			}
		}
	}
	else
	{
		bShutdownLCD = false;
		if(!mLCDPanelPoweronStatus)
		{
			removeMessage(LCD_Poweronofflcd);
			sendMessage(new Message(LCD_Poweronofflcd, (int)bShutdownLCD));
			m_OnoffLCD_retryCount_ = 0;
		}
		else
		{
			if(!hasMessage(LCD_Poweronofflcd) && m_OnoffLCD_retryCount_++ < 5)
			{
				LogD("retry to poweron lcd pane, count: \n",m_OnoffLCD_retryCount_);
				sendMessage(new Message(LCD_Poweronofflcd, (int)bShutdownLCD), 10 * 1000);
			}
		}
	}

	return 0;
}

int LCDPlayer::handlePoweronoffLCD(bool bshutdown)
{
	Termios dev;
	int fd = -1;

	const int WRITE_LEN = 9;
	const int READ_LEN = 10;
	unsigned char *databuf = new unsigned char[10];

	int index = 0;

	databuf[index++] = 0x6B; //cmd1
	databuf[index++] = 0x61; // cmd2
	databuf[index++] = 0x20; // space1
	databuf[index++] = 0x30; // devnum1
	databuf[index++] = 0x30; // devnum2
	databuf[index++] = 0x20; // space2
	databuf[index++] = 0x30; // data1
	if(bshutdown)
		databuf[index++] = 0x30; // data2 shutdown lcd
	else
		databuf[index++] = 0x31; // data2 poweron lcd
	databuf[index++] = 0x0D; // CR

	sendMessage(new Message(LCD_WriteSerialPort,(void*)databuf,index));

	if(bshutdown)
	{
		mLCDPanelShutdownStatus = true;
		mLCDPanelPoweronStatus = false;
	}
	else
	{
		mLCDPanelShutdownStatus = false;
		mLCDPanelPoweronStatus = true;
	}

//	removeMessage(LCD_CaluPoweronofftime);
//	sendMessage(new Message(LCD_CaluPoweronofftime), 1000);

	return 0;
}

int LCDPlayer::handleGetStatus()
{
	Termios dev;
	int fd = -1;

	const int WRITE_LEN = 9;
	const int READ_LEN = 10;
	unsigned char *databuf = new unsigned char[10];

	int index = 0;
	databuf[index++] = 0x6B; //cmd1
	databuf[index++] = 0x61; // cmd2
	databuf[index++] = 0x20; // space1
	databuf[index++] = 0x30; // devnum1
	databuf[index++] = 0x31; // devnum2
	databuf[index++] = 0x20; // space2
	databuf[index++] = 0x66; // data1
	databuf[index++] = 0x66; // data2
	databuf[index++] = 0x0D; // CR

	sendMessage(new Message(LCD_WriteSerialPort,(void*)databuf,index));

	return 0;
}

int LCDPlayer::handleSetScreen()
{
	Termios dev;
	int fd = -1;

	const int WRITE_LEN = 9;
	const int READ_LEN = 10;
	unsigned char *databuf = new unsigned char[10];

	int index = 0;
	databuf[index++] = 0x6B; //cmd1
	databuf[index++] = 0x63; // cmd2
	databuf[index++] = 0x20; // space1
	databuf[index++] = 0x30; // devnum1
	databuf[index++] = 0x31; // devnum2
	databuf[index++] = 0x20; // space2
	databuf[index++] = 0x30; // data1
	databuf[index++] = 0x39; // data2
	databuf[index++] = 0x0D; // CR

	sendMessage(new Message(LCD_WriteSerialPort,(void*)databuf,index));

	return 0;
}

const char *LCDPlayer::TAG = "LCDPlayer";

