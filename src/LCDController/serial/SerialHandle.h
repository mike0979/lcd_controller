/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef SERIAL_SERIALHANDLE_H_
#define SERIAL_SERIALHANDLE_H_

#include "Looper.h"
#include "Handler.h"
#include "TermiosU.h"
#include "CommonDef.h"
//#include "SerialManager.h"
class SerialManager;

#define READ_BUFFER_LEN 1024

class SerialHandle:public FdRunnable,public Handler {
public:
	struct serialParam{
		std::string deviceid;
		std::string name;
		int bandrate;
		int databits = 8;
		int stopbits = 1;
		Termios::ParityCheckMode paritycheck = Termios::PC_None;
		Termios::FlowControlMode flowcontrol = Termios::FC_None;
		Termios termios;
	};

public:
	SerialHandle(SerialManager* serialmanager);
	~SerialHandle();

	bool setSerialParam(serialParam& param);
	bool openSerialPort();
	int writeSerialPort(void* data,int size);
	bool getSerialOpenStatus();

private:
	virtual void run();
	virtual bool handleMessage(Message *msg);
private:
	static const char *TAG;

	Termios mTermios;
	int mFD;
	std::string mSerialName;
	int mSerialBandRate;
	int mSerialDataBits;
	int mSerialStopBits;
	Termios::ParityCheckMode mSerialParityCheck;
	Termios::FlowControlMode mSerialFlowControl;

	bool mSerialOpenStatus;

	SerialManager* mSerialmanager;
	std::string mDeviceID;

	unsigned char mReadBuf[READ_BUFFER_LEN];
	int mRecvIndex;

};



#endif /* SERIAL_SERIALHANDLE_H_ */
