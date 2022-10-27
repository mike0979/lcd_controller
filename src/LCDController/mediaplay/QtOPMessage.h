/*
 * QtOPMessage.h
 *
 *  Created on: Apr 10, 2017
 *      Author: root
 */

#ifndef SRC_MEDIAPANEL_QTOPMESSAGE_H_
#define SRC_MEDIAPANEL_QTOPMESSAGE_H_


#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <QtCore/QDateTime>
#include <QtGui/QLabel>
#include <QtCore/QTimer>
#include "SystemClock.h"
#include <map>
#include "json/ScheduleObjs.h"

enum OPSMsgType{
	EmergencyMsg = 1,
	ImportanceMsg,
	NormalMsg,
	DefaultMsg,
	UnknownMsg,
};

enum OPSDisplayRegion{
	fullscreenshow = 0,
	partationshow,
	masterpartation,
	unknownregion,
};

enum OPSRunningEffect {
	StaticEffect = 0,
	LeftEffect,
	RightEffect,
	UpEffect,
};

enum OPSMsgPlayStatus{
	ReadyPlay = 1,
	StartPlaying,
	PlayStoped,
	PlayEnd,
};

class OPSMsgParam {
public:
	int mID;
	std::string mTimeStamp;
	int mFormat;
	QFont mFont;
	int mSize;
	QColor mColor;
	QColor mBackColor;
	int mEffect;
	int mSpeed;
	int mPriority;
	int mPlaymode;
	int mPlayCount;
	int mPlayedCount;
	int mDisplayRegion;
	Qt::AlignmentFlag mAlign;
	QString mText;
	QString mBackImgName; // back image file name
	QDateTime mStartTime;
	QDateTime mEndTime;
	OPSMsgType mOPSMsgType;

	OPSMsgPlayStatus mPlayStatus;

	bool mHaveBackColor;
	bool mHaveBackImage;
};

class QtOPMessage : public QWidget
{
	Q_OBJECT

public:
	QtOPMessage(QWidget *parent = 0);
	~QtOPMessage();

	bool setOPSMsg(void* msg, int status);
	bool setParam(Json::OPSMsgDetail* opmsg,OPSMsgParam* &opsparam);
	void setResolution(QRect &rect);

	void setRunning(bool bstart,bool canced = false);
	void setSpeed(int speed);
	void setBGColor(const QColor &color);
	void setBGFile(const std::string &file);

	int getOPSPlayListSize();

	OPSDisplayRegion getDisplayRegion();
	bool mHaveNewOPSMsg;
	OPSMsgType mOPSMsgType;

private:
Q_SIGNALS:
	void signalOPMsgPlay(bool bstart, OPSMsgParam* currOps, int display_region = -1);
	void signalOPMsgPlayReply(const int id,const OPSMsgPlayStatus status);

public slots:
	void slotTimeout();
	void resizeEvent(QResizeEvent* event);

protected:
	void paintEvent(QPaintEvent * evt);

private	Q_SLOTS:
	void invalidate();
private:
	QString textAutoWap(QString text, QFont mFont,int w);
	void updateTextWidth();
	void updateCache();
	void deleteCache();
	void switchLoopOPSMsg();
	void cleanOPSPlayList(std::list<int>& cleanidlist);
	class LinesInfo {
	public:
		LinesInfo(QString text, int pw);

		QString mText;
		int mPixWidth;
	};

private:

	std::map<int, Json::OPSMsgDetail*> mOPSMsgPlayList;

	QPixmap mBackPixmap;
	bool mHaveBackImage;

	QLabel *mImageFrontLabel_header1;
	QLabel *mImageFrontLabel_header2;

    QLabel *mImageFrontLabel;
    QLabel *mImageBGLabel;
    QTimer mTimer;

    OPSMsgParam* mCurrOPSMsg;
    OPSDisplayRegion mCurrOPSRegion;

    std::list<OPSMsgParam*> mCurrOPSMsgList;

	bool mStartPlay;

	QTimer *mTimerForMarquee;
	std::vector<QPixmap *> mCache;
	std::vector<LinesInfo> mLinesInfo;
	uint64_t mTimeTrack;
	int mSpace;
	bool mRunning;
	float mTextPos;
	int mPaintPos;
	int mTextWidth;
	int mTextHeight;
	int mTextTotalHeight;
	long mShiftCount;
	long mPrevShiftCount;
	long mMapIndex;
	float mLastRemain;

	static const char *TAG;
	static const int FPS;
	static const int MAX_CACHE_SIZE;
};


#endif /* SRC_MEDIAPANEL_QTOPMESSAGE_H_ */
