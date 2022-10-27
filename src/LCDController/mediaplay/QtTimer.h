/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef MEDIAPLAY_QTTIMER_H_
#define MEDIAPLAY_QTTIMER_H_

#include <QtCore/QTimer>

class QtTimer : public QObject {
   	Q_OBJECT
public:
	QtTimer(QObject *parent = NULL);
	~QtTimer();

	void start(int msec);
	void stop();

	void setRunning(bool running,uint64_t rmsec = 0);
	void setTimeout();

	bool mValidDuration;  //false - duration<=0, true - duration > 0;

private:
Q_SIGNALS:
	void timeout();

private Q_SLOTS:
	void qtimerout();

private:
	static const char *TAG;

	QTimer *mTimer;

	bool mRunning;
	bool mStarted;
	int mRemains;

	uint64_t mGoStamp;
};



#endif /* MEDIAPLAY_QTTIMER_H_ */
