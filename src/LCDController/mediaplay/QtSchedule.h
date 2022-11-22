/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef MEDIAPLAY_QTSCHEDULE_H_
#define MEDIAPLAY_QTSCHEDULE_H_

#include <QtGui/QWidget>
#include "json/ScheduleObjs.h"
#include "Mutex.h"
#include <map>
#include "Handler.h"
#include <QtGui/QLabel>
#include "QtCore/qtimer.h"

class QtMediaDone;
class QtMarquee;
class QtTimer;
class QtImage;
class QtVideoPlayer;
class QtArrivalMsg;
class QtFlash;
class QtClock;
class LCDController;
class QtStreamDetector;
class OPSMsgParam;

class QtSchedule : public QWidget,public Handler {
   	Q_OBJECT
public:

    enum MessagegType{
    	Msg_SwitchMedia = 6789,
    };

	QtSchedule(LCDController* lcdcontroller, QWidget *parent = 0);
	~QtSchedule();
	void setResolution(QRect &rect);
	void setRunning(bool running);
	bool setMediasPath(const std::string &path);

	bool setPlayLayoutInfo(void* data);
	void setRTArrivalMsgInfo(void* data);
	void setTrainTimeInfo(void* data);
	void setLiveSourceInfo(int playsource);
	void setStreamStatus(bool status);
	void setOPSFullScreen(OPSMsgParam* currOps);
	bool setOPSHalfScreen(bool status,int& x ,int& y ,int& w ,int& h, OPSMsgParam* currOps);
	bool setOPSPartation(bool status,int& x ,int& y ,int& w ,int& h, OPSMsgParam* currOps);
	string getBackImage();
	void notifyMediaDone(QtMediaDone *done);
	/**
	 * stop used widget
	 * @param isfreeze - true-stop, false-run
	 */

    void freezeUsedWidgets(bool isfreeze);
    bool getFreezeStatus();

    void setBufferedCountIncrease();
    int getBufferedCount();
private:
	virtual bool handleMessage(Message *msg);

Q_SIGNALS:
	void signalQtMediaDone(QtMediaDone *done);
	void signalStartPlaynewLayout();
	void signalPlayNextContent(Json::PartitionDetail *pPartation, Json::MediaBasic* pContent);
	void signalPauseStreamDetect();
	void signalResumeStreamDetect();

private Q_SLOTS:
	void onQtMediaDone(QtMediaDone *done);
	void slotStartPlaynewLayout();
	void slotPlayNextContent(Json::PartitionDetail *pPartation, Json::MediaBasic* pContent);
	void onProbeStreamReport(bool dc);
	void slotLanguageSwitch();

private:

	bool findPartationByID(int id,Json::LayoutDetail& layout, Json::PartitionDetail*& partation);
	bool findMediaBaise(Json::LayoutInfo4Qt::MediaContents & contents,Json::MediaBasic*& content,Json::MediaBasic* pPrevContent = NULL);

	bool play(Json::PartitionDetail *pPartation, Json::MediaBasic* pContent);
	int playMarquee(QtMediaDone*& done,Json::MediaText*& mediatext,
			int x,int y,int w,int h,const std::string respath);
	int playImage(QtMediaDone*& done,Json::MediaImage*& mediaimage,
			int x,int y,int w,int h,const std::string respath);
	int playVideo(QtMediaDone*& done,Json::MediaVideo*& mediavideo,
			int x,int y,int w,int h,const std::string respath);
	int playLive(QtMediaDone*& done,Json::MediaVideo*& medialive,
			int x,int y,int w,int h,const std::string respath);
	int playArrivalMsg(QtMediaDone*& done,Json::MediaArrivalMsg* mediaarrmsg,
				int x,int y,int w,int h,const std::string respath);
	int playFlash(QtMediaDone*& done,Json::MediaFlash* mediaflash,
				int x,int y,int w,int h,const std::string respath);
	int playClock(QtMediaDone*& done,Json::MediaDigitalClock* mediaaclock,
				int x,int y,int w,int h,const std::string respath);

    QtTimer *obtainQtTimer();
    void releaseQtTimer(QtTimer *timer);

	QtMarquee *obtainQtMarquee();
	void releaseQtMarquee(QtMarquee *marquee);

    QtImage *obtainQtImage();
    void releaseQtImage(QtImage *image);

    QtVideoPlayer *obtainQtVideoPlayer();
    void releaseQtVideoPlayer(QtVideoPlayer *player);

    QtVideoPlayer *obtainQtVideoPlayerLive();
    void releaseQtVideoPlayerLive(QtVideoPlayer *player);

    QtArrivalMsg *obtainQtArrivalMsg();
    void releaseQtArrivalMsg(QtArrivalMsg *arrivalmsg);

    QtFlash *obtainQtFlash();
    void releaseQtFlash(QtFlash *flash);

    QtClock *obtainQtClock();
    void releaseQtClock(QtClock *clock);

    QtMediaDone *obtainQtMediaDone();
    void releaseMediaDone(QtMediaDone *done);

    void releaseLayoutBGPool();

	/**
	 * release used widget when switch schedule
	 * @param
	 */
    void releaseUsedWidgets();

	/**
	 * delete all widget
	 * @param
	 */
    void deleteAllWidgets();

    void releaseMediadone();
private:

    LCDController* mLCDController;
    QtStreamDetector* mStreamDector;

    std::string mMediasPath;

    Mutex mQtTimerDoneMapperMutex;
    std::map<QtTimer *, QtMediaDone *> mQtTimerDoneMapper;

    //Json::SwitchLayoutBasic mSwitchLayoutBasic;
    Json::LayoutInfo4Qt mLayoutInfoForPlay;

    bool mBUsedWidgetsFreeze;

	std::list<QtTimer *> mQtTimerPool;
	std::list<QtTimer *> mQtTimerInUse;

	std::list<QtMarquee *> mQtMarqueePool;
	std::list<QtMarquee *> mQtMarqueeInUse;

    std::list<QtImage *> mQtImagePool;
    std::list<QtImage *> mQtImageInUse;

    Mutex mVideoPlayerMutex;
    std::list<QtVideoPlayer *> mQtVideoPlayerPool;
    std::list<QtVideoPlayer *> mQtVideoPlayerInUse;

    std::list<QtVideoPlayer *> mQtVideoPlayerLivePool;
    std::list<QtVideoPlayer *> mQtVideoPlayerLiveInUse;

    std::list<QtArrivalMsg *> mQtArrivalMsgPool;
    std::list<QtArrivalMsg *> mQtArrivalMsgInUse;

    std::list<QtFlash *> mQtFlashPool;
    std::list<QtFlash *> mQtFlashInUse;

    std::list<QtClock *> mQtClockPool;
    std::list<QtClock *> mQtClockInUse;

    std::list<QtMediaDone *> mQtMediaDonePool;
    std::list<QtMediaDone *> mQtMediaDoneInUse;

    std::list<QLabel *> mQtLayoutBGFilePool;

    int mLiveBufferedCount;
    qint64 mLastMSecsSinceEpoch;

    LiveSourceSwitchConfig mLiveSourceConfig;

	static const unsigned MAX_TIMER_POOL_SIZE;
	static const unsigned MAX_MARQUEE_POOL_SIZE;
	static const unsigned MAX_IMAGE_POOL_SIZE;
	static const unsigned MAX_PLAYER_POOL_SIZE;
	static const unsigned MAX_PLAYER_LIVE_POOL_SIZE;
	static const unsigned MAX_ARRIVALMSG_POOL_SIZE;
	static const unsigned MAX_WEATHERE_POOL_SIZE;
	static const unsigned MAX_FLASH_POOL_SIZE;
	static const unsigned MAX_CLOCK_POOL_SIZE;
	static const unsigned MAX_MEDIADONE_POOL_SIZE;

	static const char *TAG;
	QTimer language_switch_timer_;
	bool cn_flag_{ true };
};

#endif /* MEDIAPLAY_QTSCHEDULE_H_ */
