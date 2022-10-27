/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef MEDIAPLAY_QTVIDEOPLAYER_H_
#define MEDIAPLAY_QTVIDEOPLAYER_H_

#include "Log.h"
#include <QtGui/QLabel>
#include <QtWebKit/QWebView>

#include <QtAV/QtAV.h>
#include <QtAVWidgets/QtAVWidgets.h>

using namespace QtAV;

class QtVideoPlayer {
public :
	QtVideoPlayer(QWidget *parent = 0, MediaEndAction ea = MediaEndAction_Default);
	~QtVideoPlayer();

	void show();
	void hide();
	void raise();

	void setMute(bool mute);

	void setGeometry(int x, int y, int w, int h);
	const QRect& geometry();

	void play(const std::string &file,bool islive,const std::string &clientip);
	void stop();
	void setRunning(bool running);

	float getCurrPosition();
	void setStartPosition(float pos);

	bool isPlaying();
public:
	AVPlayer *mPlayer;
	WidgetRenderer *mRenderer;

	bool mIsLiveMedia;

private:
	static const char *TAG;

	static const QStringList mVideoDecoderPriority;
	static const QStringList mAudioOutputPriority;
	static const QStringList mAudioOutputNull;
};

#endif /* MEDIAPLAY_QTVIDEOPLAYER_H_ */
