/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#include <Looper.h>
#include "Log.h"
#include "SerialManager.h"
#include "SerialHandle.h"
#include "config/configparser.h"

SerialManager::SerialManager(const ConfigParser* cfg)
{
	mSerialDevObjMap.clear();
	mSerialDevParamList.clear();
	mRunningHandlerMap.clear();

	if(cfg != NULL)
	{
	    for(std::map<std::string, std::string>::const_iterator it = cfg->mDevicePortMap.begin();
	    		it != cfg->mDevicePortMap.end();++it)
	    {
	    	SerialHandle::serialParam param;
	    	param.name = it->second;
	    	std::map<std::string,int>::const_iterator it1 = cfg->mBaudRateMap.find(it->first);
			if(it1 != cfg->mBaudRateMap.end())
			{
				param.deviceid = it->first;
				param.bandrate = it1->second;
			}
			else
			{
				continue;
			}

			it1 = cfg->mDataBitsMap.find(it->first);
			if(it1 != cfg->mDataBitsMap.end())
				param.databits = it1->second;

			it1 = cfg->mDataBitsMap.find(it->first);
			if(it1 != cfg->mDataBitsMap.end())
				param.databits = it1->second;

			it1 = cfg->mStopBitsMap.find(it->first);
			if(it1 != cfg->mStopBitsMap.end())
				param.stopbits = it1->second;

			it1 = cfg->mParityMap.find(it->first);
			if(it1 != cfg->mParityMap.end())
				param.paritycheck = (Termios::ParityCheckMode)(it1->second);

			it1 = cfg->mFlowctrlMap.find(it->first);
			if(it1 != cfg->mFlowctrlMap.end())
				param.flowcontrol = (Termios::FlowControlMode)(it1->second);

			mSerialDevParamList.push_back(param);
	    }
	}

}

SerialManager::~SerialManager()
{
	for(std::map<std::string,SerialHandle*>::iterator iter = mSerialDevObjMap.begin();
			iter != mSerialDevObjMap.end();++iter)
	{
		DELETE_ALLOCEDRESOURCE(iter->second);
	}
}

bool SerialManager::WriteSerialPort(void* buf, int bufsize, std::string deviceid,Handler* handler)
{
	Message* msg = new Message(SerialManager::SPORT_WriteData,buf,bufsize);
	msg->mStr = deviceid;
	sendMessage(msg);

	if(handler != NULL)
		mRunningHandlerMap[deviceid] = handler;

	return true;
}

bool SerialManager::checkDeviceId(std::string sdid)
{
	if(mSerialDevObjMap.find(sdid) != mSerialDevObjMap.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SerialManager::run()
{
    Looper *mlooper = Looper::CreateLooper();
    setLooper(mlooper);

    sendMessage(new Message(SerialManager::SPORT_OpenSerialPort));

    mlooper->loop();
}

bool SerialManager::handleMessage(Message *msg)
{
	switch(msg->mWhat)
	{
		case SPORT_OpenSerialPort:
		{
			for(std::list<SerialHandle::serialParam>::iterator it = mSerialDevParamList.begin();
				    		it != mSerialDevParamList.end();++it)
			{
				SerialHandle* serialHandle = new SerialHandle(this);
				mSerialDevObjMap[it->deviceid] = serialHandle;
				LogD("###################  serialname: %s\n",it->name.c_str());
				LogD("###################  bandrade: %d\n",it->bandrate);
				LogD("###################  databits: %d\n",it->databits);
				LogD("###################  stopbits: %d\n",it->stopbits);
				LogD("###################  paritycheck: %d\n",it->paritycheck);
				LogD("###################  flowcontrol: %d\n",it->flowcontrol);
				serialHandle->setSerialParam(*it);
				if(serialHandle->openSerialPort())
				{
					LogD("Serial device(%s) open success.\n",it->deviceid.c_str());
				}
				else
				{
					LogD("Serial device(%s) open failed!!!.\n",it->deviceid.c_str());
				}
			}

			break;
		}
		case SPORT_WriteData:
		{
			std::map<std::string,SerialHandle*>::iterator iter = mSerialDevObjMap.find(msg->mStr);
			if(iter != mSerialDevObjMap.end() && iter->second->openSerialPort())
			{
				iter->second->writeSerialPort(msg->mData,msg->mArg1);
			}
			else
			{
				LogD("Write serial device failed, deviceid-%s.\n",msg->mStr.c_str());
			}

			break;
		}
		case SPORT_ReadData:
		{
			std::map<std::string, Handler*>::iterator iter = mRunningHandlerMap.find(msg->mStr);
			if(iter != mRunningHandlerMap.end())
			{
				LogD("Reply serial data to sepcial handler. deviceid:%s\n",msg->mStr.c_str());
				Message* msg1 = new Message(1,msg->mData,msg->mArg1);
				msg1->mStr = msg->mStr;  //deviceid
				iter->second->sendMessage(msg1);
				mRunningHandlerMap.erase(iter);
			}

			break;
		}
	}

	//DELETE_ALLOCEDRESOURCE(msg);

	return true;
}


const char *SerialManager::TAG = "SerialManager";
