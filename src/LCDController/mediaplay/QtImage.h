/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef MEDIAPLAY_QTIMAGE_H_
#define MEDIAPLAY_QTIMAGE_H_


#include <QtCore/QTimer>
#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <QtGui/QMovie>
#include <QtGui/QLabel>
#include <QtGui/QGraphicsOpacityEffect>
#include "QtLabelItem.h"

class QtImage : public QWidget {
	Q_OBJECT

public:
	QtImage(QWidget *parent = 0);
	~QtImage();

	void setBGColor(const QColor &color);
	void setShowEffort(int efforts,int secondeffort);
	void setBGFile(const std::string &path,const std::string &file);
	void setFrontFile(const std::string &pathfile);
	void setRunning(bool running);

	bool isGifFormat(const std::string &file);
	void releaseMove();

public slots:
	void slotTimeout();
	//void repaint();
	void resizeEvent(QResizeEvent* event);

private:

	void DisplayShutter();
	void DisplayBoxEffort();

	ShowImageEffort mImageeffort;
	int mMaxBlindsNum ;

	QMovie *mFrontMovie;
	QMovie *mBackMovie;

    QImage mBGImage;
    QImage mFrontImage;

	QPixmap mBackPixmap;
	QPixmap mPixmap;

    QLabel *mImageFrontLabel;
    QLabel *mImageBGLabel;

    QPixmap mPixImage;
    QtLabelItem* mLabelItem;
    QVector<QtLabelItem*> myimageLabelList;

	bool mRunning;
	bool mFrontGif;
	bool mBackGif;

	bool mHaveBGFile = false;

	float mOpacity;
	QGraphicsOpacityEffect *mGraphicsEffect;
	QTimer mTimer;

	static const char *TAG;
};

#endif /* MEDIAPLAY_QTIMAGE_H_ */
