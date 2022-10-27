/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#include <serial/SerialHandle.h>
#include "Log.h"
#include "SerialManager.h"
#include <Looper.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

SerialHandle::SerialHandle(SerialManager* serialmanager):
	mSerialmanager(serialmanager),mDeviceID("")
{
	Handler::setLooper(serialmanager->getLooper());

	mFD = -1;

	mSerialName = "";
	mSerialBandRate = 9600;
	mSerialDataBits = 8;
	mSerialStopBits = 1;
	mSerialParityCheck = Termios::PC_None;
	mSerialFlowControl = Termios::FC_None;

	mSerialOpenStatus = false;

	mRecvIndex = 0;
}

SerialHandle::~SerialHandle()
{

}

bool SerialHandle::setSerialParam(serialParam& param)
{
	if(param.name.size() == 0)
	{
		LogE("no serialport name\n");
		return false;
	}

	mDeviceID = param.deviceid;

	mSerialName = param.name;
	mSerialBandRate = param.bandrate;
	mSerialDataBits = param.databits;
	mSerialStopBits = param.stopbits;
	mSerialParityCheck = param.paritycheck;
	mSerialFlowControl = param.flowcontrol;

	return true;
}

bool SerialHandle::openSerialPort()
{
	int fd = mTermios.open(mSerialName, Termios::RW);
	if (fd >= 0) {
		mTermios.setBaudRate(mSerialBandRate);
		mTermios.setDataBits(mSerialDataBits);
		mTermios.setStopBits(mSerialStopBits);
		mTermios.setParityCheck(mSerialParityCheck);
		mTermios.setFlowControl(mSerialFlowControl);

		mFD = fd;

		getLooper()->addFd(mFD, Looper::FD_EVENT_INPUT, this);

		mSerialOpenStatus = true;
	}
	else {
		LogE("Failed to open serialport (%s).\n", mSerialName.c_str());
		mSerialOpenStatus = false;
	}

	return mSerialOpenStatus;
}

int SerialHandle::writeSerialPort(void* data,int size)
{
	if (TEMP_FAILURE_RETRY(::write(mFD, data, size) != size))
	{
		LogE("Write DVAS error[mFD = %d]. : %s\n", mFD, strerror(errno));
	}
	else
	{
		mRecvIndex = 0;
	}

	return 0;
}

bool SerialHandle::getSerialOpenStatus()
{
	return mSerialOpenStatus;
}

bool SerialHandle::handleMessage(Message *msg)
{
	for(int i = 0;i<mRecvIndex;++i)
	{
		LogD("####read data  %02x\n",mReadBuf[i]);
	}

	Message* msg1 = new Message(SerialManager::SPORT_ReadData,mReadBuf,mRecvIndex);
	msg1->mStr = mDeviceID;
	mSerialmanager->sendMessage(msg1);

	mRecvIndex = 0;

	return true;
}

void SerialHandle::run()
{

	int recvlen = ::read(mFD, mReadBuf + mRecvIndex, READ_BUFFER_LEN - mRecvIndex);

	if (recvlen <= 0) {
		if (recvlen == 0 || errno != EAGAIN) {
			LogE("Read Serial Device end.\n");
		}
		return ;
	}

	mRecvIndex += recvlen;

	sendMessage(new Message(1),500);

}

const char *SerialHandle::TAG = "SerialHandle";
