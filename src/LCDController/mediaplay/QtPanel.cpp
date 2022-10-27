#include "QtPanel.h"
#include <stdio.h>
#include <QtCore/QTextCodec>
#include <QtGui/QApplication>
#include <QtWebKit/QWebSettings>
#include "QtStage.h"
#include "LCDController.h"
#include "Log.h"

#define TVS_QT_MSG_SYNC         do {    \
                                    mThreadSync->lock();    \
                                    mThreadSync->unlock();  \
                                } while (0)

extern int MAIN_ARGC;
extern char **MAIN_ARGV;

QtPanel::QtPanel(LCDController* lcdcontroller) : mLCDController(lcdcontroller)
{
   // mQtArgc = 1;
   // mQtArgv[0] = strdup(TAG);

    mApp = NULL;
    mStage = NULL;

    mThreadSync = new Mutex(false);
    mThreadSync->lock();
}

QtPanel::~QtPanel()
{
   // free(mQtArgv[0]);
	mApp->quit();
	DELETE_ALLOCEDRESOURCE(mStage);
    DELETE_ALLOCEDRESOURCE(mApp);
    DELETE_ALLOCEDRESOURCE(mThreadSync);

}

void QtPanel::run()
{
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);

    mApp = new QApplication(MAIN_ARGC, MAIN_ARGV);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    QWebSettings *websetting = QWebSettings::globalSettings();
    websetting->setAttribute(QWebSettings::JavascriptEnabled,true);
    websetting->setAttribute(QWebSettings::PluginsEnabled,true);

    mStage = new QtStage(mLCDController);
    LogD("Set window full screen and no frame\n");
    mStage->setWindowFlags(Qt::FramelessWindowHint /*| Qt::WindowStaysOnTopHint*/);
    mStage->showFullScreen();
    mStage->show();

    QObject::connect(this, SIGNAL(signalSendOPMsg(void*,int)), mStage, SLOT(onOPSMsgUpdated(void*,int)));
    QObject::connect(this, SIGNAL(signalLayoutUpdated(void*)), mStage, SLOT(onScheduleUpdated(void*)));
    QObject::connect(this, SIGNAL(signalRTArrMsgUpdated(void*)), mStage, SLOT(onRTArrivalMsgUpdated(void*)));
    QObject::connect(this, SIGNAL(signalTrainTimeUpdated(void*)), mStage, SLOT(onTrainTimeUpdated(void*)));
    QObject::connect(this, SIGNAL(signalLiveSourceUpdated(int)), mStage, SLOT(onLiveSourceUpdated(int)));

    mThreadSync->unlock();
    mApp->exec();
}

void QtPanel::sendXmlScheduleUpdatedMsg()
{
    TVS_QT_MSG_SYNC;

    //emit xmlScheduleUpdatedSig();
}

void  QtPanel::sendOPMsg(void* msg, int status)
{
    TVS_QT_MSG_SYNC;

    emit signalSendOPMsg(msg,status);
}

void QtPanel::sendCurrentLayoutInfo(void* data)
{
	TVS_QT_MSG_SYNC;
	LogE("sendCurrentLayoutInfo\n");
	emit signalLayoutUpdated(data);
}

void QtPanel::sendRTArrMsgUpdated(void* data)
{
	TVS_QT_MSG_SYNC;

	emit signalRTArrMsgUpdated(data);
}

void QtPanel::sendTrainTimeUpdated(void* data)
{
	TVS_QT_MSG_SYNC;

	emit signalTrainTimeUpdated(data);
}

void QtPanel::sendLiveSourceUpdated(int playsource)
{
	TVS_QT_MSG_SYNC;

	emit signalLiveSourceUpdated(playsource);
}


const char *QtPanel::TAG = "QtPanel";
