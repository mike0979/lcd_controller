/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/

#include "QtLabelItem.h"
#include "QtImage.h"
#include <QtGui/QPainter>
#include "Log.h"
QtLabelItem::QtLabelItem(QWidget* parent)
: QLabel(parent)
, m_currentHeight(0)
{
    effect = 0;
    this->setAttribute(Qt::WA_TranslucentBackground,false);

    mTimeline = new QTimeLine(1000, this);//1 s
    mTimeline->setCurveShape(QTimeLine::LinearCurve);
    connect(mTimeline, SIGNAL(frameChanged(int)), this, SLOT(slot_timeOut(int)));
}

QtLabelItem::~QtLabelItem()
{
	if(mTimeline != NULL)
	{
		delete mTimeline;
		mTimeline = NULL;
	}
}

void QtLabelItem::paintEvent( QPaintEvent * ev )
{
    if(!mDrawImage.isNull())
    {
        QPainter p(this);
        p.setBrush(QBrush(mCurrentImage));
        p.setPen(Qt::NoPen);



        /*      else if(effect == 2)
        {
            p.setWindow(0, 0,
                        mDrawImage.width(),
                        mDrawImage.height());

            p.drawRect(0, 0,
                       mDrawImage.width(),
                       m_currentHeight);
        }

        else */ if(effect == VShutterEffort)
        {
            p.setWindow(0, 0,
                        mDrawImage.width(),
                        mDrawImage.height());

            p.drawRect(0, 0,
                       mDrawImage.width()*m_currentHeight/mDrawImage.height(),
                       mDrawImage.height());
        }
        else if(effect == HShutterEffort)
        {
        	p.setWindow(0, 0,
        	            mDrawImage.width(),
        	            mDrawImage.height());

        	p.drawRect(0, 0,
        	            mDrawImage.width(),
        	            m_currentHeight);
        }
        else if(effect == BOXEffort)
        {
           // qDebug()<<"paintEvent";
            p.setWindow(-(mDrawImage.width()-mDrawImage.width()*m_currentHeight/mDrawImage.height())/2,
                        -(mDrawImage.height()-m_currentHeight)/2,
                        mDrawImage.width(),
                        mDrawImage.height());

            p.drawRect(0, 0,
                       mDrawImage.width()*m_currentHeight/mDrawImage.height(),
                       m_currentHeight);
        }

  /*      else if(effect == 4)
        {
            p.setWindow(-(mDrawImage.width()-mDrawImage.width()*m_currentHeight/mDrawImage.height()),
                        -(mDrawImage.height()-m_currentHeight),
                        mDrawImage.width(),
                        mDrawImage.height());

            p.drawRect(0, 0,
                       mDrawImage.width()*m_currentHeight/mDrawImage.height(),
                       m_currentHeight);
        }

        else if(effect == 5)
        {
            p.setWindow(0, 0,
                        mDrawImage.width(),
                        mDrawImage.height());

            p.drawRect(0, 0,
                       mDrawImage.width()*m_currentHeight/mDrawImage.height(),
                       m_currentHeight);
        } */
        else
        {
            p.setWindow(0, 0,
                        mDrawImage.width(),
                        mDrawImage.height());

            p.drawRect(0, 0,
                       mDrawImage.width(),
                       mDrawImage.height());
        }

    }

    QWidget::paintEvent(ev);
}

void QtLabelItem::setImage( const QPixmap& image )
{
    mDrawImage = image;

}

void QtLabelItem::setEffect(int val)
{
    effect = val;

}

void QtLabelItem::startDraw()
{
    if(!mDrawImage.isNull())
    {
        mTimeline->setFrameRange(0, mDrawImage.height()/8);
        mTimeline->start();
    }
}

void QtLabelItem::slot_timeOut( int val0)
{
    int val = val0*8+(mDrawImage.height())%8;

    /*   else if(effect == 2)
    {
        mCurrentImage = mDrawImage.copy(0, 0,
                        mDrawImage.width(), val);
    }
    else */if(effect == VShutterEffort)
    {
        mCurrentImage = mDrawImage.copy(0, 0,
                        mDrawImage.width()*val/mDrawImage.height(), mDrawImage.height());
    }
    else if(effect == HShutterEffort)
    {
    	mCurrentImage = mDrawImage.copy(0, 0,
    	                        mDrawImage.width(), val);
    }
    else  if(effect == BOXEffort)
    {
        mCurrentImage = mDrawImage.copy((mDrawImage.width()-mDrawImage.width()*val/mDrawImage.height())/2,
                        (mDrawImage.height()-val)/2,
                        mDrawImage.width()*val/mDrawImage.height(),
                        val);
    }
 /*   else if(effect == 4)
    {
        mCurrentImage = mDrawImage.copy(0, 0,
                        mDrawImage.width()*val/mDrawImage.height(),
                        val);
    }
    else if(effect == 5)
    {
        mCurrentImage = mDrawImage.copy(mDrawImage.width()-mDrawImage.width()*val/mDrawImage.height(),
                        mDrawImage.height()-val,
                        mDrawImage.width()*val/mDrawImage.height(),
                        val);
    } */
    else
    {
        mCurrentImage = mDrawImage;
    }

    m_currentHeight = val;
    update();
}

const char *QtLabelItem::TAG = "QtLabelItem";

