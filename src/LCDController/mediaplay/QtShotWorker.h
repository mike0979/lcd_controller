/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : QtShotWorker.h
 * @author : Benson
 * @date : Sep 29, 2017
 * @brief :
 */

#ifndef MEDIAPLAY_QTSHOTWORKER_H_
#define MEDIAPLAY_QTSHOTWORKER_H_

#include <QtCore/qobject.h>
#include <QtCore/qobjectdefs.h>
#include <string>

class QTimer;
class LCDController;

class QtShotWorker : public QObject{
    Q_OBJECT

public:
    QtShotWorker(LCDController *ctrl);
    ~QtShotWorker();

private Q_SLOTS:
    void snapshot();

private:
    static const char *TAG;
    LCDController *mLCDCtrl;

    int mPeriod; // (ms)
    int mWidth;
    int mHeight;

    std::string mPath;
    std::string mSuffix;

    QTimer *mShotTimer;
};

#endif /* MEDIAPLAY_QTSHOTWORKER_H_ */
