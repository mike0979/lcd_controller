/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef SERIAL_SERIALMANAGER_H_
#define SERIAL_SERIALMANAGER_H_

#include "Thread.h"
#include "Handler.h"
#include "SerialHandle.h"
#include "CommonDef.h"
#include <map>

class ConfigParser;
class SerialHandle;

class SerialManager : public Thread,public Handler
{
public:

	enum SerialMessage{
		SPORT_OpenSerialPort,
		SPORT_WriteData,
		SPORT_ReadData,
	} ;

public:
	SerialManager(const ConfigParser* cfg);
	~SerialManager();

	bool WriteSerialPort(void* buf, int bufsize, std::string deviceid,Handler* handler = NULL);
	bool checkDeviceId(std::string sdid);
private:
    virtual void run();
    virtual bool handleMessage(Message *msg);

private:

    std::list<SerialHandle::serialParam> mSerialDevParamList;
    std::map<std::string,SerialHandle*> mSerialDevObjMap;
    std::map<std::string, Handler*> mRunningHandlerMap;

	static const char *TAG;
};



#endif /* SERIAL_SERIALMANAGER_H_ */
