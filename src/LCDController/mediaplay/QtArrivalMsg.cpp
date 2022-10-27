/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/


#include "QtArrivalMsg.h"
#include "Log.h"
#include <list>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtCore/QTime>
#include <QtCore/QFile>
#include "FileSysUtils.h"
#include <boost/lexical_cast.hpp>
#include <bj_pis/ats/bj_ats_info.h>

QtArrivalMsg::QtArrivalMsg(QWidget *parent) : QWidget(parent)
{
	mArrMsgPartationPlaying = false;
	mRunning = false;
	//mBlocks = NULL;

	mParseRTArrMsg = false;

    this->setBackgroundRole(QPalette::Window);
	this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	this->setAttribute(Qt::WA_TranslucentBackground,false);

	mTimer = NULL;
//	mTimer = new QTimer(this);
//	mTimer->setInterval(1000);
//	connect(mTimer, SIGNAL(timeout()), this, SLOT(slotTimeout()));


    mImageBGLabel = new QLabel(this);
    mImageBGLabel->setBackgroundRole(QPalette::Window);
    mImageBGLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    mImageBGLabel->setScaledContents(true);
    mImageBGLabel->setAttribute(Qt::WA_TranslucentBackground,false);
    mImageBGLabel->hide();

    mStationEquipDef = new xml::StationsDef;
	xml::StationsDef::StationsDefXMLParser("./StationEquipDef.xml", mStationEquipDef);

	QLabelListNoVariables.clear();

	//set mStationCode "",inorder to note invalid  arrmsg
	mRTArrmsg.mStationCode = "";

	mStationID = "";

	mTrainTimeUpdated = false;
}

QtArrivalMsg::~QtArrivalMsg()
{
//	if(mTimer != NULL)
//	{
//		if(mTimer->isActive())
//			mTimer->stop();
//
//		delete mTimer;
//		mTimer = NULL;
//	}

	if(mImageBGLabel != NULL)
	{
		mImageBGLabel->clear();
		mImageBGLabel->hide();
		delete mImageBGLabel;
	}

	for(std::list<CountDown*>::iterator m = mCountDownList.begin();m!=mCountDownList.end();++m)
	{
		if((*m)->mLabel != NULL)
		{
			(*m)->mLabel->hide();
			delete (*m)->mLabel;
			(*m)->mLabel = NULL;
		}
		delete (*m);
		(*m) = NULL;
	}
	mCountDownList.clear();

	for(std::list<QLabel *>::iterator i = QLabelListNoVariables.begin();i!=QLabelListNoVariables.end();++i)
	{
		(*i)->hide();
		delete (*i);
		(*i) = NULL;
	}
	QLabelListNoVariables.clear();

	DELETE_ALLOCEDRESOURCE(mStationEquipDef);
}

void QtArrivalMsg::setPlayStatus(bool status)
{
	mArrMsgPartationPlaying = status;
}
bool QtArrivalMsg::getPlayStatus()
{
	return mArrMsgPartationPlaying;
}

void QtArrivalMsg::setFont(const QFont &font)
{
	if (mFont != font) {
		mFont = font;
	}
}

void QtArrivalMsg::setBGFile(const std::string &file)
{
	mBGImage.load(file.c_str());
	mImageBGLabel->clear();
	QPixmap pixImage = pixImage.fromImage(mBGImage);
	mImageBGLabel->setPixmap(pixImage);
	mImageBGLabel->setAlignment(Qt::AlignCenter);
}

void QtArrivalMsg::setColor(const QColor &color, const QColor &backcolor)
{
	if (mColor != color || mBackColor != backcolor) {
		mColor = color;
		mBackColor = backcolor;
		//update();
	}
}


void QtArrivalMsg::setBlocks(std::vector<Json::LabelInfo> &blocks)
{
	if(blocks.size() == 0)
		return ;


	mBlocks = blocks;

/*	std::string texttoUpper;

	for(std::list<xml::ArrivalBlock*>::const_iterator i = blocks->mArrivalBlockList.begin();i!=blocks->mArrivalBlockList.end();++i)
	{

		texttoUpper = (*i)->mAlign;
		transform(texttoUpper.begin(),texttoUpper.end(),texttoUpper.begin(),::toupper);
		if(texttoUpper.compare("CENTER") == 0)
			(*i)->mAlignType = Qt::AlignCenter;
		else if(texttoUpper.compare("LEFT") == 0)
			(*i)->mAlignType = Qt::AlignLeft;
		else if(texttoUpper.compare("RIGHT") == 0)
					(*i)->mAlignType = Qt::AlignRight;
		else
			(*i)->mAlignType = Qt::AlignCenter;

		texttoUpper = (*i)->mStyle;
		transform(texttoUpper.begin(),texttoUpper.end(),texttoUpper.begin(),::toupper);
		(*i)->mItalic = false;

		if(texttoUpper.compare("NORMAL") == 0)
		{
			(*i)->mFontType = QFont::Normal;
		}
		else if(texttoUpper.compare("BOLD") == 0)
		{
			(*i)->mFontType = QFont::Bold;
		}
		else if(texttoUpper.compare("ITALIC") == 0)
		{
			(*i)->mFontType = QFont::Normal;
			(*i)->mItalic = true;
		}
		else
		{
			(*i)->mFontType = QFont::Normal;
		}

	}
	*/
}

void QtArrivalMsg::setRTArrMsgFile(Json::ArrivalDetail* rtarrmsg)
{
	if(rtarrmsg != NULL)
	{
		mRTArrmsg = *rtarrmsg;
	}
	else
	{
		//set mStationCode "",inorder to note invalid  arrmsg
		mRTArrmsg.mStationCode = "";
	}
}


void QtArrivalMsg::setTrainTimeInfo(Json::TrainTimeDetail* traintimemsg)
{
	if(traintimemsg != NULL)
	{
		LogD("set train time info.\n");
		mTrainTimeInfo = *traintimemsg;
		mTrainTimeUpdated = true;
	}
//	else
//	{
//		//set mStationCode "",inorder to note invalid  arrmsg
//		mTrainTimeInfo.mStationCode = "";
//	}
}
void QtArrivalMsg::setStationID(const std::string stationid)
{
	mStationID = stationid;
}

void QtArrivalMsg::slotTimeout()
{
	bool isStopTimer = false;
	if(mCountDownList.size() > 0)
	{
		for(std::list<CountDown*>::const_iterator m = mCountDownList.begin();m!=mCountDownList.end();++m)
		{
			char stemp[64];
			std::string scontent = (*m)->mContent;

			if((*m)->mLabel != NULL)
				(*m)->mLabel->clear();

			if((*m)->mSecs < 60)
			{
				scontent = "列车进站";
				(*m)->mLabel->setText(scontent.c_str());
			}
			else
			{
				isStopTimer = true;

				std::string sformat = (*m)->mTimeFormat;
				std::size_t pos_reg = sformat.find("%");
				while(pos_reg != std::string::npos)
				{
					bool bfind = false;
					std::size_t pos_h = sformat.find("%M");
					if(pos_h != std::string::npos)
					{
						bfind = true;
						sprintf(stemp,"%02d",((*m)->mSecs)/60);
						sformat.replace(pos_h,2,boost::lexical_cast<std::string>(stemp));
					}

					std::size_t pos_s = sformat.find("%S");
					if(pos_s != std::string::npos)
					{
						bfind = true;
						sprintf(stemp,"%02d",((*m)->mSecs)%60);
						sformat.replace(pos_s,2,boost::lexical_cast<std::string>(stemp));
					}

					pos_reg = sformat.find("%");
					if(!bfind && pos_reg != std::string::npos)
					{
						sformat.replace(pos_reg,1,"");
						pos_reg = sformat.find("%");
					}
				}

				if(sformat.length() == 0)
				{
					sprintf(stemp,"%02d:%02d",((*m)->mSecs)/60,((*m)->mSecs)%60);
					scontent.replace((*m)->mPos,(*m)->mLen,boost::lexical_cast<std::string>(stemp));
				}
				else
				{
					scontent.replace((*m)->mPos,(*m)->mLen, boost::lexical_cast<std::string>(sformat));
				}

				(*m)->mSecs--;

				(*m)->mLabel->setText(scontent.c_str());
			}
		}

//		if(!isStopTimer && mTimer->isActive())
//			mTimer->stop();

	}
}
void QtArrivalMsg::showArrMsg(bool cn_flag)
{
	bool isvalidarrmsg = true;
//	if(mRTArrmsg.mTrains.size() == 0)
//	{
//		LogD("RT arrivalmsg mTrains size == 0.\n");
//		isvalidarrmsg = false;
//	}

	if(mRTArrmsg.mStationCode.size() == 0)
	{
		//LogD("RT arrivalmsg mStationCode size == 0.\n");
		isvalidarrmsg = false;
	}

	if(mRTArrmsg.mPlatformCode.size() == 0)
	{
		//LogD("RT arrivalmsg mPlatformCode size == 0.\n");
		isvalidarrmsg = false;
	}

//	if(mTimer->isActive())
//		mTimer->stop();

	std::string strtemp = "";
//	for(std::list<QLabel *>::iterator i = QBlockLabelList.begin();i!=QBlockLabelList.end();++i)
//	{
//		delete (*i);
//		(*i) = NULL;
//	}
//	QBlockLabelList.clear();

	int rtmsg_trainmaxindex = mRTArrmsg.mTrains.size();

	for(std::vector<Json::LabelInfo>::iterator i = mBlocks.begin();i!=mBlocks.end();++i)
	{
		std::string sConcent;
		if (cn_flag || (*i).mVarText.mText.mContentEn.empty())
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

		QLabel* label = new QLabel(this);
		label->setBackgroundRole(QPalette::Window);
		label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
		label->setScaledContents(true);
		label->setAttribute(Qt::WA_TranslucentBackground,false);

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

		label->setGeometry(ix,iy, iw, ih);
		label->hide();

		label->setFont(mFont);
		QPalette palette;
		palette.setColor(QPalette::WindowText, mColor);
		//label->setAutoFillBackground(true);
		label->setPalette(palette);

		//

		//std::string sConcent = (*i).mVarText.mText.mContent;

		if((*i).mVarText.mVaribles.size() == 0)
		{
			label->setAlignment((Qt::Alignment)(flag));
			label->setText(sConcent.c_str());
			label->show();
			QLabelListNoVariables.push_back(label);
		}
		else
		{
		    if((*i).mVarText.mVaribles.size() == 0)
		    {
		        LogD("vartext size is 0.\n");
		        DELETE_ALLOCEDRESOURCE(label);
		        continue;
		    }

			int index = 0;
			bool isdowncounttype = false;
			for(std::vector<Json::VaribleInfo>::iterator v = (*i).mVarText.mVaribles.begin();
					v != (*i).mVarText.mVaribles.end(); ++v)
			{
				std::size_t pos = sConcent.find("{"+boost::lexical_cast<std::string>(index)+"}");
				if(pos==std::string::npos)
				{
					index++;
					continue;
				}
				sConcent=bj_ats_info::Instance().GetValue(v->mTypeStr,v->mTrainIndex);
				QLabelListNoVariables.push_back(label);

				//LogD("Arrival Message sConcent: %s \n",sConcent.c_str());
				label->setAlignment((Qt::Alignment)(flag));
				label->setText(sConcent.c_str());
				label->show();

				index++;
			}
		}
	}

	mTrainTimeUpdated = false;

	//20171229 add comment
//	if(mCountDownList.size() > 0)
//		mTimer->start();
}

void QtArrivalMsg::setRunning(bool running, bool cn_flag)
{
		mRunning = running;

//		if(mTimer->isActive())
//			mTimer->stop();

		for(std::list<CountDown*>::iterator m = mCountDownList.begin();m!=mCountDownList.end();++m)
		{
			if((*m)->mLabel != NULL)
			{
				(*m)->mLabel->hide();
				DELETE_ALLOCEDRESOURCE((*m)->mLabel);
			}
			DELETE_ALLOCEDRESOURCE(*m);
		}
		mCountDownList.clear();

		for(std::list<QLabel *>::iterator i = QLabelListNoVariables.begin();i!=QLabelListNoVariables.end();++i)
		{
			(*i)->hide();
			DELETE_ALLOCEDRESOURCE(*i);
		}
		QLabelListNoVariables.clear();

		if(mRunning)
		{
			mImageBGLabel->show();
			showArrMsg(cn_flag);
		}
		else
		{
			mImageBGLabel->hide();
		}

}
const char *QtArrivalMsg::TAG = "QtArrivalMsg";


