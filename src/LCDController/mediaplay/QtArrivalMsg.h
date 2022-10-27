/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef MEDIAPLAY_QTARRIVALMSG_H_
#define MEDIAPLAY_QTARRIVALMSG_H_


#include <QtCore/QTimer>
#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <list>
#include <QtGui/QLabel>
#include "xml/StationEquipDef.h"
#include <json/ScheduleObjs.h>

class CountDown{
public:
	int mX;
	int mY;
	int mH;
	int mW;

	int mAlignType;
	int mSecs;

	QFont mFont;
	QColor mColor;
	QLabel* mLabel;

	std::string mContent;
	int mPos;
	int mLen;
	std::string mTimeFormat;
};
class QtArrivalMsg : public QWidget
{
	Q_OBJECT

public:
	QtArrivalMsg(QWidget *parent = 0);
	~QtArrivalMsg();

	void setFont(const QFont &font);
	void setBGFile(const std::string &file);
	void setColor(const QColor &color, const QColor &backcolor);
	void setBlocks(std::vector<Json::LabelInfo> &blocks);
	void setRTArrMsgFile(Json::ArrivalDetail* rtarrmsg);
	void setTrainTimeInfo(Json::TrainTimeDetail* rtarrmsg);
	void setRunning(bool running, bool cn_flag = true);
	void setStationID(const std::string stationid);
	void setPlayStatus(bool status);
	bool getPlayStatus();

private:
	void showArrMsg(bool cn_flag);

//protected:
//	void paintEvent(QPaintEvent * evt);
public slots:
		void slotTimeout();
private:
	QFont mFont;
	QColor mColor;
	QColor mBackColor;
	QPixmap mBackPixmap;
	bool mRunning;
	std::vector<Json::LabelInfo> mBlocks;
	Json::ArrivalDetail mRTArrmsg;
	bool mParseRTArrMsg;

	QTimer* mTimer;

	std::list<CountDown*> mCountDownList;
	std::list<QLabel *> QLabelListNoVariables;

	QLabel *mImageBGLabel;
	QImage mBGImage;

	xml::StationsDef* mStationEquipDef;

	bool mArrMsgPartationPlaying;

	std::string mStationID;

	Json::TrainTimeDetail mTrainTimeInfo;
	bool mTrainTimeUpdated;


	static const char *TAG;
};

#endif /* MEDIAPLAY_QTARRIVALMSG_H_ */
