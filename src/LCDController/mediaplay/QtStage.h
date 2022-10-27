/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file name : comment_example.h
 * @author : alex
 * @date : 2017/7/14 14:29
 * @brief : open a file
 */
#ifndef MEDIAPLAY_QTSTAGE_H_
#define MEDIAPLAY_QTSTAGE_H_

#include <QtCore/qobjectdefs.h>
#include <QtCore/qrect.h>
#include <QtGui/qwidget.h>
#include <QtCore/qmap.h>
#include <mediaplay/QtOPMessage.h>
#include "QtSchedule.h"
class OPSMsgParam;

class LCDController;
class QtOPMessage;
class QtShotWorker;
class ITransHandler;

class QtStage: public QWidget {
	Q_OBJECT
public:
	explicit QtStage(LCDController* lcdcontroller,QWidget *parent = 0);
	~QtStage();

private Q_SLOTS:
	/**
	 * Notify OPS list changed from OPSHandler.
	 * @param msg: OPS detail
	 * @param status: add status, update status or delete status.
	 */
	void onOPSMsgUpdated(void* msg,int status);
	/**
	 * slot to handle ops start play or stop play .
	 * @param bstart: true - start to play, false - play over
	 * @param currOps: a ops info .
	 */
	void onOPMsgPlay(bool bstart, OPSMsgParam* currOps, int display_region);
	/**
	 * send OPS play status to OPShandler.
	 * @param id: play OPS id
	 * @param status: play status.
	 */
	void onOPMsgPlayReply(const int id,const OPSMsgPlayStatus status);

	/**
	 * schedule updated.
	 * @param
	 * @param
	 */
	void onScheduleUpdated(void* schpkg);
	/**
	 * schedule play reply to transmanager.
	 * @param
	 * @param
	 */
	void onSchedulePlayReply();

	void onRTArrivalMsgUpdated(void* data);

	void onTrainTimeUpdated(void* data);

	void onLiveSourceUpdated(int playsource);

protected:
	/**
	 * avoid app exit when press ESC.
	 * @param event
	 */
	void keyPressEvent(QKeyEvent* event);
    /**
     * close process when qt window was closed.
     * @param event
     */
    void closeEvent(QCloseEvent* event);

    void changeEvent (QEvent * event);
private:
	 void setupUi(QWidget *QtStage, int width, int height);
	 void setSize(int width, int height);
	 ITransHandler* getTransLoader(NotifyMessageCode code);
private:
	int mWidth;
	int mHeight;
	LCDController* mLCDController;

	QRect mFullScreenRect;
	QMap<int, QtOPMessage*> qt_ops_msg_;
	QtShotWorker* mQtShortWorker;

	QtSchedule* mQtSchedule;

	static const char *TAG;
};

#endif /* MEDIAPLAY_QTSTAGE_H_ */
