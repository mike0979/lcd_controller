/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/

#include "QtClock.h"
#include "Log.h"

#include <QtCore/QDateTime>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <boost/lexical_cast.hpp>

QtClock::QtClock(QWidget *parent) : QWidget(parent)
{
	mTimer = new QTimer(this);

    this->setBackgroundRole(QPalette::Window);
	this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	this->setAttribute(Qt::WA_TranslucentBackground,false);

	mType = Combined;
	mRunning = false;
	mShowSec = true;
	mSecBeat = true;
	mUse24 = true;
	mOneline = true;

	//mWeekList = {"星期一","星期二","星期三","星期四","星期五","星期六","星期天"};
	mWeekMap[1]="星期一";
	mWeekMap[2]="星期二";
	mWeekMap[3]="星期三";
	mWeekMap[4]="星期四";
	mWeekMap[5]="星期五";
	mWeekMap[6]="星期六";
	mWeekMap[7]="星期天";
	mWeekMap[8]="Monday";
	mWeekMap[9]="Tuesday";
	mWeekMap[10]="Wednesday";
	mWeekMap[11]="Thursday";
	mWeekMap[12]="Friday";
	mWeekMap[13]="Saturday";
	mWeekMap[14]="Sunday";

	mMonthMap[1] = "January";
	mMonthMap[2] = "February";
	mMonthMap[3] = "March";
	mMonthMap[4] = "April";
	mMonthMap[5] = "May";
	mMonthMap[6] = "June";
	mMonthMap[7] = "July";
	mMonthMap[8] = "August";
	mMonthMap[9] = "September";
	mMonthMap[10] = "October";
	mMonthMap[11] = "November";
	mMonthMap[12] = "December";

	/*if (ClockDial == NULL) {
		ClockDial = new QPixmap("./res/clock_analog_dial.png");
		if (ClockDial->isNull()) {
			LogD("Can NOT load ./res/clock_analog_dial.png\n");
		}
	}

	if (ClockHour == NULL) {
		ClockHour = new QPixmap("./res/clock_analog_hour.png");
		if (ClockHour->isNull()) {
			LogD("Can NOT load ./res/clock_analog_hour.png\n");
		}
	}

	if (ClockMin == NULL) {
		ClockMin = new QPixmap("./res/clock_analog_minute.png");
		if (ClockMin->isNull()) {
			LogD("Can NOT load ./res/clock_analog_minute.png\n");
		}
	}

	if (ClockSec == NULL) {
		ClockSec = new QPixmap("./res/clock_analog_second.png");
		if (ClockSec->isNull()) {
			LogD("Can NOT load ./res/clock_analog_second.png\n");
		}
	}*/

	mFont.setPixelSize(36);

	mAlign = Qt::AlignCenter;

	mTimer->setInterval(500);
	connect(mTimer, SIGNAL(timeout()), this, SLOT(update()));
}

QtClock::~QtClock()
{
	if(mTimer != NULL)
	{
		if(mTimer->isActive())
			mTimer->stop();

		delete mTimer;
		mTimer = NULL;
	}
}

void QtClock::setBlocks(std::vector<Json::LabelInfo> &blocks)
{
	if(blocks.size() == 0)
		return ;

	mBlocks = blocks;
}

void QtClock::setClockType(ClockType type)
{
	if (mType != type) {
		mType = type;

		update();
	}
}

void QtClock::setRunning(bool running, bool cn_flag)
{
	cn_flag_ = cn_flag;
	if (mRunning != running) 
	{
		mRunning = running;

		if (mRunning) {
			mTimer->start();
		}
		else {
			mTimer->stop();
		}
	}
	update();
}

void QtClock::showSecPointer(bool show)
{
	if (mShowSec != show) {
		mShowSec = show;

		update();
	}
}

void QtClock::setFont(const QFont &font)
{
	if (mFont != font) {
		mFont = font;

		update();
	}
}

void QtClock::setAlign(int align)
{
	if(align == 0)
		mAlign = Qt::AlignLeft;
	else if(align == 1)
		mAlign = Qt::AlignCenter;
	else if(align == 2)
		mAlign = Qt::AlignRight;
	else
		mAlign = Qt::AlignCenter;
}

void QtClock::setUse24(bool use24)
{
	if (mUse24 != use24) {
		mUse24 = use24;

		update();
	}
}

void QtClock::setOneline(bool oneline)
{
	if(mOneline != oneline)
		mOneline = oneline;
}

void QtClock::setColor(const QColor &color, const QColor &backcolor)
{
	if (mColor != color || mBackColor != backcolor) {
		mColor = color;
		mBackColor = backcolor;

		update();
	}
}

void QtClock::setBGFile(const std::string &file)
{
	mBackPixmap.load(file.c_str());
}

void QtClock::paintEvent(QPaintEvent * evt)
{
	int width = this->width();
	int height = this->height();
	int side = qMin(width, height);

	QDateTime time = QDateTime::currentDateTime();
	int year = time.date().year();
	int month = time.date().month();
	int day = time.date().day();
	int hour = time.time().hour();
	int min = time.time().minute();
	int sec = time.time().second();
	int week = time.date().dayOfWeek();

	QPainter painter(this);
	for(std::vector<Json::LabelInfo>::iterator i = mBlocks.begin();i!=mBlocks.end();++i)
	{
		std::string sConcent;
		if (cn_flag_ || (*i).mVarText.mText.mContentEn.empty())
		{
			mFont = QFont((*i).mVarText.mText.mFont.mName.c_str(), (*i).mVarText.mText.mFont.mSize);
			sConcent = (*i).mVarText.mText.mContent;
		}
		else
		{
			mFont = QFont((*i).mVarText.mText.mFontEn.mName.c_str(), (*i).mVarText.mText.mFontEn.mSize);
			sConcent = (*i).mVarText.mText.mContentEn;
		}

		mColor = strtoul((*i).mVarText.mText.mForeColor.c_str(), NULL, 16);

		if((*i).mVarText.mText.mFont.mIsBold)
			mFont.setWeight(QFont::Bold);
		else
			mFont.setWeight(QFont::Normal);

		mFont.setItalic((*i).mVarText.mText.mFont.mIsItalic);

		painter.setFont(mFont);
		painter.setPen(mColor);
		painter.setRenderHint(QPainter::Antialiasing);

		//std::string sConcent = (*i).mVarText.mText.mContent;
		int index = 0;
		char stemp[64];
		for(std::vector<Json::VaribleInfo>::iterator v = (*i).mVarText.mVaribles.begin();
				v != (*i).mVarText.mVaribles.end(); ++v)
		{
			std::string tformat = (*v).mTimeFormat;
			std::size_t pos = sConcent.find("{"+boost::lexical_cast<std::string>(index)+"}");
			if(pos != std::string::npos)
			{
				//fix bug-1202
				std::size_t pos_reg = tformat.find("%");
				while(pos_reg != std::string::npos)
				{
					bool bfind = false;
					std::size_t pos_y = tformat.find("%Y");
					if(pos_y != std::string::npos)
					{   bfind = true;
						sprintf(stemp,"%04d",year);
						tformat.replace(pos_y,2,boost::lexical_cast<std::string>(stemp));
					}

					std::size_t pos_m = tformat.find("%m");
					if(pos_m != std::string::npos)
					{   bfind = true;
						sprintf(stemp,"%02d",month);
						tformat.replace(pos_m,2,boost::lexical_cast<std::string>(stemp));
					}

					std::size_t pos_a = tformat.find("%a");
					if(pos_a != std::string::npos)
					{
						bfind = true;
						tformat.replace(pos_a,2, mMonthMap[month]);
					}


					std::size_t pos_d = tformat.find("%d");
					if(pos_d != std::string::npos)
					{   bfind = true;
						sprintf(stemp,"%02d",day);
						tformat.replace(pos_d,2,boost::lexical_cast<std::string>(stemp));
					}

					std::size_t pos_h = tformat.find("%H");
					if(pos_h != std::string::npos)
					{   bfind = true;
						sprintf(stemp,"%02d",hour);
						tformat.replace(pos_h,2,boost::lexical_cast<std::string>(stemp));
					}

					std::size_t pos_M = tformat.find("%M");
					if(pos_M != std::string::npos)
					{   bfind = true;
						sprintf(stemp,"%02d",min);
						tformat.replace(pos_M,2,boost::lexical_cast<std::string>(stemp));
					}

					std::size_t pos_s = tformat.find("%S");
					if(pos_s != std::string::npos)
					{   bfind = true;
						sprintf(stemp,"%02d",sec);
						tformat.replace(pos_s,2,boost::lexical_cast<std::string>(stemp));
					}

					std::size_t pos_W = tformat.find("%W");
					if(pos_W != std::string::npos)
					{   bfind = true;
						tformat.replace(pos_W,2,mWeekMap[week]);
					}

					std::size_t pos_w = tformat.find("%w");
					if(pos_w != std::string::npos)
					{   bfind = true;
						tformat.replace(pos_w,2,mWeekMap[week+7].substr(0,3));
					}


					pos_reg = tformat.find("%");
					if(!bfind && pos_reg != std::string::npos)
					{
						tformat.replace(pos_reg,1,"");
						pos_reg = tformat.find("%");
					}
				}
				sConcent.replace(pos,3,tformat);
			}

			index++;
		}

		int ix = 0;
		int iy = 0;
		int iw = 0;
		int ih = 0;

		i->GetRectInfo(ix,iy,iw,ih);
		int flag = Qt::AlignCenter;
		if(i->mVarText.mText.mAlign == 0)
			flag = Qt::AlignLeft;
		if(i->mVarText.mText.mAlign == 1)
			flag = Qt::AlignCenter;
		if(i->mVarText.mText.mAlign == 2)
			flag = Qt::AlignRight;
		painter.drawText(ix, iy, iw, ih, flag, sConcent.c_str());
	}
}

void QtClock::drawPixmap(QPainter &painter, const QPixmap &pixmap, float rot, float scale)
{
	int px, py, pw, ph;

	pw = pixmap.width();
	ph = pixmap.height();
	px = 0;
	py = 0;

	painter.save();

	//painter.rotate(rot);
	painter.drawPixmap(px, py, pw, pw, pixmap);

//	painter.restore();
}

const char *QtClock::TAG = "QtClock";

//QPixmap *QtClock::ClockDial = NULL;
//QPixmap *QtClock::ClockHour = NULL;
//QPixmap *QtClock::ClockMin = NULL;
//QPixmap *QtClock::ClockSec = NULL;

