/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/

#include "QtImage.h"
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <string.h>
#include "Log.h"

QtImage::QtImage(QWidget *parent) : QWidget(parent)
{
	mFrontMovie = NULL;
	mBackMovie = NULL;
	mRunning = false;
	mFrontGif = false;
	mBackGif = false;
    mImageeffort = NoEffort;
    mMaxBlindsNum = 10;

	mOpacity = 0.0;
	mGraphicsEffect = new QGraphicsOpacityEffect;
	connect(&mTimer,SIGNAL(timeout()),this,SLOT(slotTimeout()));

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

    mLabelItem = new QtLabelItem(this);
    mLabelItem->setBackgroundRole(QPalette::Window);
    mLabelItem->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    mLabelItem->setScaledContents(true);
    mLabelItem->setAttribute(Qt::WA_TranslucentBackground,false);
    mLabelItem->hide();
}

QtImage::~QtImage()
{
	releaseMove();

	if(mGraphicsEffect != NULL)
	{
		delete mGraphicsEffect;
		mGraphicsEffect = NULL;
	}

	if(mImageBGLabel != NULL)
	{
		mImageBGLabel->clear();
		mImageBGLabel->hide();
		delete mImageBGLabel;
	}

	if(mImageFrontLabel != NULL)
	{
		mImageFrontLabel->clear();
		mImageFrontLabel->hide();
		delete mImageFrontLabel;
	}

	if(mLabelItem != NULL)
	{
		mLabelItem->clear();
		mLabelItem->hide();
		delete mLabelItem;
	}

}

void QtImage::resizeEvent(QResizeEvent* event)
{

	QSize size = event->size();

	mImageBGLabel->resize(size);
	mImageFrontLabel->resize(size);

	mLabelItem->setFixedWidth(size.width());
	mLabelItem->setFixedHeight( size.height());
	mLabelItem->setGeometry(0,0,size.width(), size.height());

	int index = 0;
	for(QVector<QtLabelItem*>::iterator i = myimageLabelList.begin();i!=myimageLabelList.end();++i)
	{
		if(mImageeffort ==VShutterEffort)
		{

			(*i)->setFixedWidth((*i)->width());
			(*i)->setFixedHeight(size.height());
			(*i)->setGeometry((*i)->x(),(*i)->y(),(*i)->width(), size.height());

		}else if(mImageeffort ==HShutterEffort)
		{
			int tmp =  size.height()%mMaxBlindsNum;

			(*i)->setFixedWidth(size.width());
			if(index<=tmp)
			{
				(*i)->setFixedHeight(size.height()/mMaxBlindsNum+1);
				(*i)->setGeometry( 0, index*(size.height()/mMaxBlindsNum)+index,
						size.width(), size.height()/mMaxBlindsNum+1);
			}
			else
			{
				(*i)->setFixedHeight(size.height()/mMaxBlindsNum);
				(*i)->setGeometry( 0, index*(size.height()/mMaxBlindsNum)+tmp,
						size.width(), size.height()/mMaxBlindsNum);
			}
			index++;

		}
	}
}

void QtImage::setShowEffort(int efforts,int secondeffort)
{
	if(efforts == 0)
		mImageeffort = NoEffort;
	else if(efforts == 1 && secondeffort == 0)
		mImageeffort = VShutterEffort;
	else if(efforts == 1 && secondeffort == 1)
		mImageeffort = HShutterEffort;
	else if(efforts == 2)
		mImageeffort = BOXEffort;
	else if(efforts == 3)   //RoundEffort
		mImageeffort = BOXEffort;
	else if(efforts == 4)   //ChessBoxEffort
		mImageeffort = BOXEffort;
	else if(efforts == 5)   //FadeEffort
		mImageeffort = FadeEffort;
	else if(efforts == 6)   //SpreadEffort
		mImageeffort = BOXEffort;
	else
		mImageeffort = NoEffort;
}

void QtImage::setBGColor(const QColor &color)
{
    QPalette mDefBGPalette;
    mDefBGPalette.setColor(QPalette::Window, color);
    mImageBGLabel->setAutoFillBackground(true);
    mImageBGLabel->setPalette(mDefBGPalette);
}
void QtImage::setBGFile(const std::string &path,const std::string &file)
{
	if(file.length() <= 0) {
		return ;
	}

	std::string backfile = path + "/" + file;

	mBackGif = isGifFormat(backfile);

	if(!mBackGif) {
		mBGImage.load(backfile.c_str());
		mImageBGLabel->clear();
		mPixImage = mPixImage.fromImage(mBGImage);
		mImageBGLabel->setPixmap(mPixImage);
		mImageBGLabel->setAlignment(Qt::AlignCenter);
	}else {
		mBackMovie = new QMovie(backfile.c_str());
		if (mBackMovie->isValid() == true) {
			mImageBGLabel->setMovie(mBackMovie);
		}
	}
}

void QtImage::setFrontFile(const std::string &pathfile)
{
	if(pathfile.length() <= 0)
		return ;

	std::string frontfile = pathfile;

	mFrontGif = isGifFormat(frontfile);

	if(!mFrontGif) {

		mFrontImage.load(frontfile.c_str());
		mFrontImage = mFrontImage.scaled(this->size());
		mPixImage = mPixImage.fromImage(mFrontImage);

		if(mImageeffort == NoEffort || mImageeffort == FadeEffort)
		{
			mImageFrontLabel->clear();
			mPixImage = mPixImage.fromImage(mFrontImage);
			mImageFrontLabel->setPixmap(mPixImage);
			mImageFrontLabel->setAlignment(Qt::AlignCenter);
			if(mImageeffort == FadeEffort)
			{
				mOpacity = 0.0;
				mGraphicsEffect->setOpacity(mOpacity);
				mImageFrontLabel->setGraphicsEffect(mGraphicsEffect);
				mTimer.start(250);
			}

			//mImageFrontLabel->show();

		} else if (mImageeffort == VShutterEffort || mImageeffort == HShutterEffort) {
			DisplayShutter();
		} else if(mImageeffort == BOXEffort) {
			DisplayBoxEffort();
		} else if(mImageeffort == FadeEffort) {

		}
	}else {
		mFrontMovie = new QMovie(frontfile.c_str());
		if (mFrontMovie->isValid() == true) {
			mImageFrontLabel->setMovie(mFrontMovie);
		}
	}
}

void QtImage::DisplayShutter()
{
	int tmp = 0;
	int tmp1 = 0;
	QVector<QtLabelItem*> items0;

	if(mImageeffort == VShutterEffort)
	{
		tmp = mFrontImage.width()%mMaxBlindsNum;
		tmp1 = width()%mMaxBlindsNum;
	}else
	{
		tmp = mFrontImage.height()%mMaxBlindsNum;
	    tmp1 = height()%mMaxBlindsNum;
	}

    myimageLabelList.clear();
    for(int i = 0; i <= mMaxBlindsNum; ++i)
    {
        QtLabelItem* item = new QtLabelItem(this);
        item->hide();
        myimageLabelList.push_back(item);
    }

	for(int i = 0; i < mMaxBlindsNum; ++i)
	{
		myimageLabelList[i]->clear();
		QPixmap pimage;
		if(mImageeffort == VShutterEffort)
		{
			myimageLabelList[i]->setFixedHeight(height());
			if(i<=tmp1)
			{
				myimageLabelList[i]->setFixedWidth(width()/mMaxBlindsNum+1);
				myimageLabelList[i]->setGeometry( i*(width()/mMaxBlindsNum)+i, 0,
						 width()/mMaxBlindsNum+1, height());
			}
			else
			{
				myimageLabelList[i]->setFixedWidth(width()/mMaxBlindsNum);
				myimageLabelList[i]->setGeometry( i*(width()/mMaxBlindsNum)+tmp1, 0,
						 width()/mMaxBlindsNum, height());
			}

			if(i<=tmp)
			{
				pimage = mPixImage.copy(i*(mFrontImage.width()/mMaxBlindsNum)+i, 0,
					 mFrontImage.width()/mMaxBlindsNum+1, mFrontImage.height());

			}
			else
			{
				pimage = mPixImage.copy(i*(mFrontImage.width()/mMaxBlindsNum)+tmp, 0,
					 mFrontImage.width()/mMaxBlindsNum, mFrontImage.height());

			}
		}
		else
		{
			myimageLabelList[i]->setFixedHeight(width());
			if(i<=tmp1)
			{
				myimageLabelList[i]->setFixedHeight(height()/mMaxBlindsNum+1);
				myimageLabelList[i]->setGeometry( 0, i*(height()/mMaxBlindsNum)+i,
				         width(), height()/mMaxBlindsNum+1);
			}
			else
			{
				myimageLabelList[i]->setFixedHeight(height()/mMaxBlindsNum);
				myimageLabelList[i]->setGeometry( 0, i*(height()/mMaxBlindsNum)+tmp1,
				         width(), height()/mMaxBlindsNum);
			}

			if(i<=tmp)
			{
				pimage = mPixImage.copy( 0, i*(mFrontImage.height()/mMaxBlindsNum)+i,
						mFrontImage.width(), mFrontImage.height()/mMaxBlindsNum+1);
			}
			else
			{
				pimage = mPixImage.copy( 0, i*(mFrontImage.height()/mMaxBlindsNum)+tmp,
						mFrontImage.width(), mFrontImage.height()/mMaxBlindsNum);
			}
		}

		myimageLabelList[i]->setImage(pimage);
		myimageLabelList[i]->setEffect(mImageeffort);
		//myimageLabelList[i]->show();
		items0.push_back(myimageLabelList[i]);
	}
	myimageLabelList[mMaxBlindsNum]->hide();
	mImageFrontLabel->hide();

	foreach(QtLabelItem* item, items0)
	{
		if(item)
			item->startDraw();
	}
}

void QtImage::DisplayBoxEffort()
{
	if(mImageeffort == BOXEffort)
	{
		mImageFrontLabel->hide();

		mLabelItem->clear();
		mLabelItem->setImage(mPixImage);
		mLabelItem->setFixedWidth(width());
		mLabelItem->setFixedHeight(height());
		mLabelItem->setAlignment(Qt::AlignCenter);
		mLabelItem->setEffect(BOXEffort);
		mLabelItem->startDraw();
		mLabelItem->setGeometry( 0, 0,width(), height());
		//mLabelItem.show();
	}
}

void QtImage::setRunning(bool running)
{
	if (mRunning != running) {
		mRunning = running;

		if(mRunning)
		{
			if(mBackGif) {
				mBackMovie->start();
			}

			if(mFrontGif) {
				mFrontMovie->start();
			}

			mImageBGLabel->show();
			mImageFrontLabel->show();

			if(mImageeffort == BOXEffort || mImageeffort == FadeEffort)
				mLabelItem->show();

			if(mImageeffort == VShutterEffort || mImageeffort == HShutterEffort)
			{
				for(QVector<QtLabelItem*>::iterator i = myimageLabelList.begin();i!=myimageLabelList.end();++i)
			    {
					(*i)->show();
			    }
			}
		}
		else
		{
			if(mBackGif) {
				mBackMovie->stop();
			}

			if(mFrontGif) {
				mFrontMovie->stop();
			}

			mImageBGLabel->hide();
			mImageFrontLabel->hide();

			if(mImageeffort == BOXEffort || mImageeffort == FadeEffort)
				mLabelItem->hide();

			if(mImageeffort == VShutterEffort || mImageeffort == HShutterEffort)
			{
				for(QVector<QtLabelItem*>::iterator i = myimageLabelList.begin();i!=myimageLabelList.end();++i)
			    {
					(*i)->hide();
			    }
			}
		}
	}
}

void QtImage::slotTimeout()
{
	if(mImageeffort == FadeEffort)
	{
		if(mOpacity>=1.0)
		{
			mTimer.stop();
		}else
		{
			mGraphicsEffect->setOpacity(mOpacity);
			mImageFrontLabel->setGraphicsEffect(mGraphicsEffect);

			mOpacity += 0.1;
		}
	}
}
void QtImage::releaseMove()
{
	mRunning = false;

	if(mFrontMovie != NULL) {
		delete mFrontMovie;
		mFrontMovie = NULL;
	}

	if(mBackMovie != NULL) {
		delete mBackMovie;
		mBackMovie = NULL;
	}

	mLabelItem->clear();
	mLabelItem->hide();

	mImageBGLabel->clear();
	mImageBGLabel->hide();

	mImageFrontLabel->clear();
	mImageFrontLabel->hide();


	if(mImageeffort == VShutterEffort || mImageeffort == HShutterEffort)
	{
		if(myimageLabelList.size() > 0)
		{
			for(QVector<QtLabelItem*>::iterator i = myimageLabelList.begin();i!=myimageLabelList.end();++i)
			{
					(*i)->clear();
					(*i)->hide();
					delete (*i);
			}
			myimageLabelList.clear();
		}
	}


}

bool QtImage::isGifFormat(const std::string &file)
{
	bool isgif = false;
	size_t suffixpos = file.find_last_of('.');
	if (suffixpos != std::string::npos) {
		if (strcasecmp(file.substr(suffixpos + 1).c_str(), "gif") == 0) {
			isgif = true;
		}
	}
	return isgif;
}

const char *QtImage::TAG = "QtImage";

