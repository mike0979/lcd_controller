/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef MEDIAPLAY_QTSTREAMDETECTOR_H_
#define MEDIAPLAY_QTSTREAMDETECTOR_H_
#include "Thread.h"
#include <QtCore/QObject>
#include <arpa/inet.h>

class QtStreamDetector :public QObject,  public Thread
{
	Q_OBJECT

public:
	explicit QtStreamDetector(std::string ip,int port,int timeout);

	virtual ~QtStreamDetector();

	void pauseDetect();

	void resumeDetect();

	void doInit();

	void setStreamStatus(bool status);
	bool getStreamStatus();

	bool SetIpPort(const std::string& ip,int port);

Q_SIGNALS:
	void signalProbeStreamReport(bool dc);

public Q_SLOTS:
	void onPauseStreamDetect();
	void onResumeStreamDetect();

private:

    virtual void run();
private:
    bool mLiveStreamOK;
    pthread_mutex_t mMutex;
    pthread_cond_t mCond;
    bool mbPause;
    std::string mIp;
    int mPort;
    bool mIpPortChanged;
    int mMultiCastTimeout;

    int mFd;
    struct sockaddr_in mAddr;
	struct ip_mreq mMreq;
    static const char *TAG;
};





#endif /* MEDIAPLAY_QTSTREAMDETECTOR_H_ */
