/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef MEDIAPLAY_QTCLOCK_H_
#define MEDIAPLAY_QTCLOCK_H_


#include <QtCore/QTimer>
#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <map>
#include <json/ScheduleObjs.h>

class QtClock : public QWidget {
	Q_OBJECT

public:
	enum ClockType {
		Analog = 0,
		Digital,
		Combined,
	};

public:
	QtClock(QWidget *parent = 0);
	~QtClock();

	void setBlocks(std::vector<Json::LabelInfo> &blocks);
	void setClockType(ClockType type);
	void setRunning(bool running, bool cn_flag = true);
	void showSecPointer(bool show);
	void setFont(const QFont &font);
	void setAlign(int align);
	void setUse24(bool use24);
	void setColor(const QColor &color, const QColor &backcolor = 0xFFFFFF);
	void setBGFile(const std::string &file);
	void setOneline(bool oneline);

protected:
	void paintEvent(QPaintEvent * evt);

private:
	void drawPixmap(QPainter &painter, const QPixmap &pixmap, float rot, float scale);

private:
	std::vector<Json::LabelInfo> mBlocks;
	QTimer *mTimer;
	QFont mFont;
	QColor mColor;
	QColor mBackColor;
	QPixmap mBackPixmap;

	int mAlign;
	ClockType mType;
	bool mRunning;
	bool mShowSec;
	bool mSecBeat;
	bool mUse24;
	bool mOneline;
	std::string mWeekList[7];
	std::map<int,std::string> mWeekMap;
	std::map<int,std::string> mMonthMap;

	static const char *TAG;

	static QPixmap *ClockDial;
	static QPixmap *ClockHour;
	static QPixmap *ClockMin;
	static QPixmap *ClockSec;
	bool cn_flag_ { true };
};

#endif /* MEDIAPLAY_QTCLOCK_H_ */
