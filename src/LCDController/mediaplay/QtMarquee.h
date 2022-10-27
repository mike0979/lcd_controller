/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef MEDIAPLAY_QTMARQUEE_H_
#define MEDIAPLAY_QTMARQUEE_H_

#include <QtCore/QTimer>
#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <vector>
#include <QtGui/QGraphicsOpacityEffect>
#include "QtTimer.h"
#include "json/ScheduleObjs.h"
#include "SystemClock.h"
#include <QtGui/qlabel.h>
#include "QtMediaDone.h"

class QtMarquee : public QWidget {
	Q_OBJECT

public:
	QtMarquee(QWidget *parent = 0);
	~QtMarquee();

	void setTransparentLevel(int level);
	void setText(const QString &text);
	void setTextEn(const QString& text);
	void setFont(const std::string& fonttype, const int& scale);
	void setFontEn(const std::string &fonttype,const int &scale);
	void setColor(const std::string scolor, const std::string sbackcolor);
	void setBGFile(const std::string &file);

	void setSpeed(int speed);
	void setPixelPerSecond(int pixel);
	void setEffort(int effort);
	void setAlign(Json::TextInfo ti);
	void setSpace(int space);
	void setRunning(bool running, bool rewind = false, bool cn_flag = true);
	long getMarqueeTime();
	void setDurationTimer(QtTimer* timer);
	void setQtMediaDone(QtMediaDone* done);
	void setPartationHeight(int h);

public:
	int mX;
	int mY;
	int mW;
	int mH;

	enum TextRunningEffect {
		StaticEffect = 0,
		LeftEffect,
		RightEffect,
		UpEffect,
	};

protected:
	void paintEvent(QPaintEvent * evt);

private	Q_SLOTS:
	void invalidate();

public Q_SLOTS:
		void resizeEvent(QResizeEvent* event);

private:
	void updateTextWidth();

	void updateCache();
	void deleteCache();

private:
	class LinesInfo {
	public:
		LinesInfo(QString text, int pw);

		QString mText;
		int mPixWidth;
	};

	QTimer *mTimer;
	std::vector<QPixmap *> mCache;
	std::vector<LinesInfo> mLinesInfo;

	QFont mFont;
	QColor mColor;
	QColor mBackColor;
	QPixmap mBackPixmap;

	QString mText;
	QString mTextEn;

	int mSpeed;
	int mEffect;
	Qt::AlignmentFlag mAlign;
	int mSpace;
	bool mRunning;

	float mTextPos;
	int mPaintPos;
	int mTextWidth;
	int mTextHeight;
	int mTextTotalHeight;

	int mFontWeightOrigin;

	uint64_t mTimeTrack;

	QtTimer* mDurationTimer;

	QtMediaDone* mMediaDone;

	int mTransparentLevel;

	bool mFontSizeChanged;

	int mFontScale;
	int mFontScaleEn;
	std::string mFontType;
	std::string mFontTypeEn;

	bool mMarqueeupdated;

	int mPartationLastHeight;

	bool mHaveBackColor;
	bool mHaveBackImage;

	QLabel *mImageBGLabel;
	QLabel* mImageFrontLabel;

	static const char *TAG;
	static const int FPS;
	static const int MAX_CACHE_SIZE;
};

#endif /* MEDIAPLAY_QTMARQUEE_H_ */
