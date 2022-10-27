/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef NTP_NTPDATE_H_
#define NTP_NTPDATE_H_

#ifndef SRC_NTPDATE_H_
#define SRC_NTPDATE_H_

#include "Handler.h"
#include "Thread.h"

class LCDController;

class NtpDate : public Thread, public Handler {
public:
	enum NtpDateMessage {
		NtpPeriod = 0,
	};

public:
	NtpDate(LCDController *lcdcontroller);

private:
	virtual void run();
	virtual bool handleMessage(Message *msg);
	static void* ntpSyncProc(void* args);

private:
	LCDController *mLCDController;

	static std::string mServer;
	static int mPeriod;

	static const char *TAG;
};

#endif /* SRC_NTPDATE_H_ */





#endif /* NTP_NTPDATE_H_ */
