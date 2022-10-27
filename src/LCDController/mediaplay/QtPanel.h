#ifndef SRC_QTPANEL_H_
#define SRC_QTPANEL_H_

#include <QtCore/QObject>
#include <QtGui/QApplication>
#include "Thread.h"
#include "Mutex.h"
#include "mediaplay/QtStage.h"

class LCDController;

class QtPanel : public QObject, public Thread
{
    Q_OBJECT

public:
    QtPanel(LCDController* lcdcontroller);
    ~QtPanel();

    void sendXmlScheduleUpdatedMsg();
    void sendOPMsg(void* msg,int status);

    void sendCurrentLayoutInfo(void* data);
    void sendRTArrMsgUpdated(void* data);
    void sendTrainTimeUpdated(void* data);
    void sendLiveSourceUpdated(int playsource);

Q_SIGNALS:
    void xmlScheduleUpdatedSig();
    void signalSendOPMsg(void* msg,int status);
    void signalLayoutUpdated(void* data);
    void signalRTArrMsgUpdated(void* data);
    void signalTrainTimeUpdated(void* data);
    void signalLiveSourceUpdated(int playsource);

private:
    virtual void run();

private:
    static const char *TAG;

    LCDController* mLCDController;

    int mQtArgc;
    char *mQtArgv[1];


    QApplication *mApp;
    QtStage *mStage;

    Mutex *mThreadSync;
};

#endif /* SRC_QTPANEL_H_ */
