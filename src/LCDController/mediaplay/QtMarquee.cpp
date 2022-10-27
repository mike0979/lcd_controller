/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#include "QtMarquee.h"
#include "Log.h"

#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QBitmap>

QtMarquee::QtMarquee(QWidget *parent) : QWidget(parent)
{
	mTimer = new QTimer(this);

	mSpeed = 360;
	mSpace = 120;

	mRunning = false;

	mTextPos = 0;
	mPaintPos = 0;
	mTextWidth = 0;
	mTextHeight = 0;

	mTimeTrack = 0;

	mFontWeightOrigin = mFont.pointSize();

    mImageBGLabel = new QLabel(this);
    mImageBGLabel->setBackgroundRole(QPalette::Window);
    mImageBGLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    mImageBGLabel->setScaledContents(true);
    mImageBGLabel->hide();

    mImageFrontLabel = new QLabel(this);
    mImageFrontLabel->setBackgroundRole(QPalette::Window);
    mImageFrontLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    mImageFrontLabel->setScaledContents(true);
    mImageFrontLabel->setAttribute(Qt::WA_TranslucentBackground,false);
    mImageFrontLabel->hide();

    this->setBackgroundRole(QPalette::Window);
	this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	this->setAttribute(Qt::WA_TranslucentBackground,false);

	connect(mTimer, SIGNAL(timeout()), this, SLOT(invalidate()));

	mDurationTimer = NULL;

	mMediaDone = NULL;

	mTransparentLevel = 0;

	mFontSizeChanged = false;

	mFontScale = 0;

	mFontType = "";

	mMarqueeupdated = false;

	mPartationLastHeight = 0;

	mHaveBackColor = false;

	mHaveBackImage = false;

	mEffect = (int)LeftEffect;

	mAlign = Qt::AlignCenter;
}

QtMarquee::~QtMarquee()
{
	DELETE_ALLOCEDRESOURCE(mImageBGLabel);
	DELETE_ALLOCEDRESOURCE(mImageFrontLabel);
	DELETE_ALLOCEDRESOURCE(mTimer);
	deleteCache();
}

void QtMarquee::setText(const QString &text)
{
	if (mText != text) {
		mText = text;

		mTextPos = 0;
		mPaintPos = 0;

		mMarqueeupdated = true;
	}
}


void QtMarquee::setTextEn(const QString& text)
{
	if (mTextEn != text) 
	{
		mTextEn = text;
		mTextPos = 0;
		mPaintPos = 0;
		mMarqueeupdated = true;
	}
}

void QtMarquee::setFont(const std::string &fonttype,const int &scale)
{
	if(mFontType != fonttype ||	mFontScale != scale)
	{
		mFontType = fonttype;

		mFontScale = scale;

		mMarqueeupdated = true;
	}
}


void QtMarquee::setFontEn(const std::string& fonttype, const int& scale)
{
	if (mFontTypeEn != fonttype || mFontScaleEn != scale)
	{
		mFontTypeEn = fonttype;
		mFontScaleEn = scale;
		mMarqueeupdated = true;
	}
}

void QtMarquee::resizeEvent(QResizeEvent* event)
{
	mX = this->x();
	mY = this->y();
	mW = this->width();
	mH = this->height();

	int height = event->size().height();
	if(height != mPartationLastHeight)
	{
		mMarqueeupdated = true;
		mPartationLastHeight = height;
	}

	mImageBGLabel->resize(event->size());
	mImageFrontLabel->resize(event->size());
}

void QtMarquee::setColor(const std::string scolor, const std::string sbackcolor)
{
	if(scolor.size() > 0)
	{
		mColor = strtoul(scolor.c_str(), NULL, 16);
	}

	if(sbackcolor.size() > 0)
	{
		mBackColor = strtoul(sbackcolor.c_str(), NULL, 16);
		mHaveBackColor = true;
	}
	else
	{
		mHaveBackColor = false;
	}

	mMarqueeupdated = true;
}

void QtMarquee::setBGFile(const std::string &file)
{
	if(mBackPixmap.load(file.c_str()))
	{
		mHaveBackImage = true;
	}
	else
	{
		mHaveBackImage = false;
	}
}

void QtMarquee::setSpeed(int speed)
{
	if(speed <0 || speed>4)
	{
		LogE("Text speed(%d) should between 0 and 4, speed will be set default value\n",speed);
		mSpeed= 60;
	}

	switch(speed)
	{
	case 0:
		mSpeed = 180;
		break;
	case 1:
		mSpeed = 150;
		break;
	case 2:
		mSpeed = 120;
		break;
	case 3:
		mSpeed = 90;
		break;
	case 4:
		mSpeed = 60;
		break;
	}
}

void QtMarquee::setPixelPerSecond(int pixel)
{
	mSpeed = pixel;
}

void QtMarquee::setEffort(int effort)
{
	if(effort > UpEffect || effort < StaticEffect)
	{
		LogE("Unknown text effort-%d.\n",effort);
		mEffect = (int)LeftEffect;
		return ;
	}

	mEffect = effort;
}

void QtMarquee::setAlign(Json::TextInfo ti)
{
	if(ti.mAlign == ti.Left)
		mAlign = Qt::AlignLeft;
	else if(ti.mAlign == ti.Center)
		mAlign = Qt::AlignCenter;
	else if(ti.mAlign == ti.Right)
		mAlign = Qt::AlignRight;
	else
		mAlign = Qt::AlignLeft;

}

void QtMarquee::setTransparentLevel(int level)
{
	mTransparentLevel = level;
}

void QtMarquee::setSpace(int space)
{
	if (space >= 0 && space < width() / 2 && mSpace != space) {
		mSpace = space;
	}
}

void QtMarquee::setRunning(bool running, bool rewind, bool cn_flag)
{
	if (mRunning != running)
	{
		mRunning = running;
	}

	if (mRunning)
	{

		if(mEffect == StaticEffect)
		{
			if(mHaveBackImage)
			{
				mImageBGLabel->setPixmap(mBackPixmap);
				mImageBGLabel->show();
			}

			if (cn_flag)
			{
				mFont = QFont(mFontType.c_str(), mFontScale);
				mImageFrontLabel->setText(mText);
			}
			else
			{
				mFont = QFont(mFontTypeEn.c_str(), mFontScaleEn);
				mImageFrontLabel->setText(mTextEn);
			}

			QPalette PaletteText;
			//mFont = QFont(mFontType.c_str(), mFontScale);
			PaletteText.setColor(QPalette::WindowText, mColor);
			mImageFrontLabel->setPalette(PaletteText);
			mImageFrontLabel->setAlignment(mAlign | Qt::AlignVCenter);
			mImageFrontLabel->setWordWrap(true);
			mImageFrontLabel->setFont(mFont);
			//mImageFrontLabel->setText(mText);
			mImageFrontLabel->show();
		}
		else
		{
			if(mEffect == (int)LeftEffect)
				mPaintPos = this->width();
			else if(mEffect == (int)RightEffect)
				mPaintPos = 0;
			else if(mEffect == (int)UpEffect)
				mPaintPos = this->height();
			else
				mPaintPos = this->width();

			mTimer->start(40);
//				if (mSpeed > FPS) {
//					mTimer->start(1000 / FPS);
//				}
//				else {
//					mTimer->start(1000 / mSpeed);
//				}
		}

	}
	else
	{
		mImageBGLabel->hide();
		mImageFrontLabel->hide();
		mImageBGLabel->clear();
		mImageFrontLabel->clear();

		if(mTimer->isActive())
			mTimer->stop();

		if(mDurationTimer != NULL)
		{
			mDurationTimer->setRunning(false);
		}
	}

	if (rewind)
	{
		mTextPos = 0;
		mPaintPos = 0;
		mTimeTrack = 0;
	}
}

void QtMarquee::updateTextWidth()
{
	mFont = QFont(mFontType.c_str(), mFontScale);
	mFont.setBold(true);

	QFontMetrics fm(mFont);

	mTextWidth = 0;
	//mTextWidth = fm.width(mText);
	mTextHeight = fm.height();
	mTextTotalHeight = 0;

	mLinesInfo.clear();
	int tempwidth = 0;
	QString tempstr = "";
	for(int index = 0;index<mText.length();++index)
	{
		if(mEffect == (int)UpEffect)
		{
			if((tempwidth <= this->width() && tempwidth + fm.width(mText.at(index)) > this->width() )||mText.at(index)=='\n')
			{
				mLinesInfo.push_back(LinesInfo(tempstr, tempwidth));
				mTextTotalHeight += fm.height();
				LogD("--1-- %s\n",tempstr.toStdString().c_str());
				tempwidth = 0;
				tempstr = "";
			}
			if(mText.at(index)!='\n')
			{
				tempwidth += fm.width(mText.at(index));
				tempstr += mText.at(index);
			}
		}
		else
		{
			mLinesInfo.push_back(LinesInfo(mText.at(index), fm.width(mText.at(index))));
			mTextWidth+=fm.width(mText.at(index));
		}
	}

	if(mEffect == (int)UpEffect && tempstr.size() > 0)
	{
		mLinesInfo.push_back(LinesInfo(tempstr, tempwidth));
		mTextTotalHeight += fm.height();
		LogD("--2-- %s\n",tempstr.toStdString().c_str());
	}

//	for(int i=0;i<mLinesInfo.size();++i)
//	{
//		LogD("---- %s\n",mLinesInfo.at(i).mText.toStdString().c_str());
//	}
}

void QtMarquee::setDurationTimer(QtTimer* timer)
{
	mDurationTimer = timer;
}

void QtMarquee::setQtMediaDone(QtMediaDone* done)
{
	mMediaDone = done;
}

long QtMarquee::getMarqueeTime()
{
	return (mTextWidth+this->width())/FPS+1;
}

void QtMarquee::updateCache()
{
	deleteCache();

	for (std::vector<LinesInfo>::iterator i = mLinesInfo.begin(); i != mLinesInfo.end(); i++) {
		QPixmap *pixmap = new QPixmap(i->mPixWidth, mTextHeight);
		pixmap->fill(Qt::transparent);

		QPainter painter(pixmap);
		painter.setCompositionMode(QPainter::CompositionMode_Source);
		painter.setFont(mFont);
		painter.setPen(mColor);
		painter.drawText(0, 0, i->mPixWidth, mTextHeight, Qt::AlignVCenter, i->mText);
		painter.setCompositionMode(QPainter::CompositionMode_Destination);
		painter.end();

		mCache.push_back(pixmap);
	}

}

void QtMarquee::deleteCache()
{
	for (std::vector<QPixmap *>::iterator i = mCache.begin(); i != mCache.end(); i++) {
		delete (*i);
	}

	mCache.clear();
}

void QtMarquee::paintEvent(QPaintEvent * evt)
{
	if(mEffect == (int)StaticEffect)
	{
		return ;
	}

	if (mCache.size() == 0 || mMarqueeupdated) {
		updateTextWidth();
		updateCache();
		mMarqueeupdated = false;
		return ;
	}

	int width = this->width();
	int height = this->height();

	if (mSpace >= width) {
		mSpace = width / 2;
	}

	QPainter painter(this);
	painter.setClipRect(0, 0, width, height, Qt::ReplaceClip);
	painter.setCompositionMode(QPainter::CompositionMode_Source);


	if (!mHaveBackImage)
	{
		if(mHaveBackColor)
		{
			if(mTransparentLevel == 0)
				mBackColor.setAlpha(255);
			else if(mTransparentLevel == 1)
				mBackColor.setAlpha(30);
			else if(mTransparentLevel == 2)
				mBackColor.setAlpha(0);
			else
				mBackColor.setAlpha(255);

			QRect rect;
			rect.setX(0);
			rect.setY(0);
			rect.setWidth(width);
			rect.setHeight(height);
			painter.fillRect(rect,mBackColor);
		}
	}
	else {
		if(mTransparentLevel == 0)
			painter.setOpacity(1);
		else if(mTransparentLevel == 1)
			painter.setOpacity(0.4);
		else if(mTransparentLevel == 2)
			mBackColor.setAlpha(0);
		else
			painter.setOpacity(1);
		painter.drawPixmap(0, 0, width, height, mBackPixmap);
	}
	painter.setCompositionMode(QPainter::CompositionMode_Destination);
	painter.end();


	QPainter painter1(this);
	int ipos = mPaintPos;

	if(mLinesInfo.size() == 1)
	{
		painter1.drawPixmap(10, this->height()/2 - mTextHeight/2, *mCache[0]);
	}
	else if(mLinesInfo.size() == 2)
	{
		painter1.drawPixmap(10, this->height()/2 - mTextHeight, *mCache[0]);
		painter1.drawPixmap(10, this->height()/2 , *mCache[1]);
	}
	else if(mLinesInfo.size() == 3)
	{
		painter1.drawPixmap(10, this->height()/2 - mTextHeight*1.5, *mCache[0]);
		painter1.drawPixmap(10, this->height()/2 - mTextHeight*0.5, *mCache[1]);
		painter1.drawPixmap(10, this->height()/2 + mTextHeight*0.5 , *mCache[2]);
	}
	else
	{

		for (unsigned i = 0; i < mLinesInfo.size(); i++) {
			if (ipos + mLinesInfo[i].mPixWidth > 0 && ipos <= width && mCache[i]!=NULL) {
				int ix = 0;
				int iy = 0;

				if(mEffect == (int)LeftEffect || mEffect == (int)RightEffect)
				{
					ix = ipos;iy=(height - mTextHeight) / 2;
				}
				else if(mEffect == (int)UpEffect && mAlign == Qt::AlignLeft)
				{
					ix = 10; iy = ipos;
				}
				else if(mEffect == (int)UpEffect && mAlign == Qt::AlignCenter)
				{
					//ix = (width - mTextHeight) / 2 ;iy = ipos;
					ix = 10; iy = ipos;
				}
	//			else if(mEffect == (int)UpEffect && mAlign == Qt::AlignRight)
	//			{
	//				ix = width - mTextHeight ;iy = ipos;
	//			}
				else
				{
					ix = ipos;iy=(height - mTextHeight) / 2;
				}
				painter1.drawPixmap(ix, iy, *mCache[i]);

			}

			if(mEffect == (int)LeftEffect)
				ipos += mLinesInfo[i].mPixWidth;
			else if(mEffect == (int)RightEffect)
				ipos -= mLinesInfo[i].mPixWidth;
			else if(mEffect == (int)UpEffect)
				ipos += mTextHeight;
			else
				ipos += mLinesInfo[i].mPixWidth;
		}
	}

	if(mLinesInfo.size() <=3)
	{
		mTimer->stop();
	}

	//painter.setCompositionMode(QPainter::CompositionMode_Destination);
	//painter.end();
}

void QtMarquee::invalidate()
{

	if (mRunning) {

		if(mTimeTrack == 0)
		{
			mTimeTrack=SystemClock::SystemTime(SystemClock::SYSTEM_TIME_MONOTONIC);
		}
		uint64_t time =  SystemClock::SystemTime(SystemClock::SYSTEM_TIME_MONOTONIC) - mTimeTrack;
		//printf("00-----%lu----\n",time / 1000000);
		mTextPos=(int)(mSpeed*(time/1000000)/1000.0+0.5);
		bool bvalue = false;
		if(mEffect == (int)LeftEffect)
			bvalue = (mTextPos >= mTextWidth) && (mTextPos-mTextWidth > this->width());
		else if(mEffect == (int)RightEffect)
			bvalue = (mTextPos - mTextWidth >= this->width());
		else if(mEffect == (int)UpEffect)
			bvalue = (mTextPos - mTextTotalHeight >= this->height());
		else  //default - left
			bvalue = (mTextPos >= mTextWidth) && (mTextPos-mTextWidth > this->width());


		if (bvalue){
			if(mEffect == (int)RightEffect)
				mTextPos = -mTextHeight;
			else
				mTextPos = 0;//mTextPos - mTextWidth - mSpace;

			if(mMediaDone != NULL)
			{
				//mDurationTimer->setTimeout();
				//mMediaDone->onQtMediaDone(true);
				if(mEffect == (int)LeftEffect)
				{
					mPaintPos = this->width();
					mTimeTrack = 0;
				}
				else if(mEffect == (int)RightEffect)
				{
					mPaintPos = 0;
					mTimeTrack = 0;
				}
				else if(mEffect == (int)UpEffect)
				{
					mPaintPos = this->height();
					mTimeTrack = 0;
				}
				return ;
			}
		}
	}

	int pos = mPaintPos;
	if(mEffect == (int)LeftEffect)
		 pos = this->width()-mTextPos;
	else if(mEffect == (int)RightEffect)
		pos = mTextPos;
	else if(mEffect == (int)UpEffect)
		pos = this->height()-mTextPos;
	else
		pos = this->width()-mTextPos;

	if (pos != mPaintPos) {
		mPaintPos = pos;

		repaint();
	}
}

QtMarquee::LinesInfo::LinesInfo(QString text, int pw) : mText(text), mPixWidth(pw)
{

}

const char *QtMarquee::TAG = "QtMarquee";
const int QtMarquee::FPS = 500;
const int QtMarquee::MAX_CACHE_SIZE = 8192;



