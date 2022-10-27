/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : QtShotWorker.cpp
 * @author : Benson
 * @date : Sep 29, 2017
 * @brief :
 */

#include <config/configparser.h>
#include <LCDController.h>
#include <Log.h>
#include <mediaplay/QtShotWorker.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qtimer.h>
#include <QtGui/qapplication.h>
#include <QtGui/qdesktopwidget.h>
#include <QtGui/qimage.h>
#include <QtGui/qpixmap.h>
#include <SystemClock.h>

QtShotWorker::QtShotWorker(LCDController *ctrl) :
        mLCDCtrl(ctrl)
{
    const ConfigParser *config = ctrl->getConfig();

    if (config != NULL)
    {
        mPeriod = config->mSnapShotPeriod * 1000;
        mWidth = config->mSnapShotW;
        mHeight = config->mSnapShotH;
        mPath = config->mSnapShotPath;
        mSuffix = config->mSnapShotSfx;
    } else
    { // use default value.
        mPeriod = 60 * 1000;
        mWidth = 800;
        mHeight = 600;
        mPath = "/home/workspace/";
        mSuffix = ".jpg";
    }

    mPath.append("/SNAP_" + config->mDeviceId + "_");

    mShotTimer = new QTimer(this);
    mShotTimer->start(mPeriod);
    connect(mShotTimer, SIGNAL(timeout()), this, SLOT(snapshot()));
}

QtShotWorker::~QtShotWorker()
{
	if(mShotTimer != NULL)
	{
		if(mShotTimer->isActive())
			mShotTimer->stop();

		delete mShotTimer;
		mShotTimer = NULL;
	}
}

void QtShotWorker::snapshot()
{
    QPixmap pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());

    QImage image = pixmap.toImage();
    std::string file = mPath;
    file.append(SystemClock::Today(SystemClockTMFormatNoSpace)).append(mSuffix);

    bool save = image.scaled(mWidth, mHeight, Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation).save(file.c_str());
    LogI("Save screen snapshot %s : success?[%d]\n", file.c_str(), save);

}

const char *QtShotWorker::TAG = "QtShotWorker";

