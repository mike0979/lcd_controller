/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef MEDIAPLAY_QTMEDIADONE_H_
#define MEDIAPLAY_QTMEDIADONE_H_

#include <QtCore/QObject>
#include "json/ScheduleObjs.h"
#include <QtAV/QtAV.h>
#include <QtCore/QTimer>

class QtSchedule;
class QtMarquee;
class QtTimer;
class QtImage;
class QtVideoPlayer;
class QtArrivalMsg;
class QtFlash;
class QtClock;

class QtMediaDone : public QObject {
   	Q_OBJECT
public:
	QtMediaDone();
	~QtMediaDone();

	void setOPSPartation(bool flag);
	bool isOPSPartation();

	void setSwitchmediaMsg(bool status);
	bool getSwitchmediaMsg();
	void checkTimerActiveForLive();

public Q_SLOTS:
	void slotTimeout();
	void onQtMediaDone(bool paused = true);
	void onQtPositionChanged(qint64 position);
	void onmediaStatusChanged(QtAV::MediaStatus status);
	void OnCheckVideoStop();
	void slotsingletimeshot();
	void setDefaultValue();
public:
	QtSchedule *mQtSchedule;
	Json::PartitionMedias * pMedias;
	Json::PartitionDetail *pPartation;
	Json::MediaBasic* pContent;

	union {
		void* pVoid;
		QtMarquee *mMarquee;
		QtImage *mImage;
		QtVideoPlayer *mVideoPlayer;
		QtArrivalMsg *mArrivalMsg;
		QtFlash *mFlash;
		QtClock *mClock;
	};

	int mMediaType;

	QtTimer *mTimer;
	bool bPlayStatus;

	bool mMediaDoneReleaseFlag;
private:

	qint64 mLastPosition;

	qint64 mLastPositionForTimer;

	QTimer mTimerForLive;
	QTimer mTimerForCheckBlack;

	int mCountTime;

	bool mIsOpsFlag;
	bool mOPSPlaying;

	int mVideostopCount;
	qint64 mLastMSecsSinceEpoch;

	bool mSwitchmediaMsgSended; //fix msg

	static const char *TAG;

};

#endif /* MEDIAPLAY_QTMEDIADONE_H_ */
