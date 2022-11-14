/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/

#include "QtVideoPlayer.h"
#include "CommonDef.h"
QtVideoPlayer::QtVideoPlayer(QWidget *parent, MediaEndAction ea)
{
	mPlayer = new AVPlayer();
	mRenderer = new WidgetRenderer(parent);

//	QColor qColor(0,0,0);
//	mRenderer->setBackgroundRole(QPalette::Window);
//	mRenderer->setBackgroundColor(qColor);

	mRenderer->setOutAspectRatioMode(QtAV::VideoRenderer::RendererAspectRatio);

	mPlayer->setRenderer(mRenderer);
	mPlayer->setMediaEndAction(ea);

//	mPlayer->setBufferMode(BufferPackets);
//	mPlayer->setBufferValue(-1);

	mPlayer->masterClock()->setClockType(AVClock::AudioClock);
//	mPlayer->masterClock()->setClockAuto(true);

	//mPlayer->renderer()->setOutAspectRatio(QtAV::VideoRenderer::RendererAspectRatio);

	mPlayer->setVideoDecoderPriority(mVideoDecoderPriority);
	mPlayer->audio()->setBackends(mAudioOutputPriority);

	mIsLiveMedia = false;
}

QtVideoPlayer::~QtVideoPlayer()
{
	mRenderer->close();
	mPlayer->clearVideoRenderers();

	DELETE_ALLOCEDRESOURCE(mRenderer);
	DELETE_ALLOCEDRESOURCE(mPlayer);
}

void QtVideoPlayer::show()
{
	mRenderer->show();
}

void QtVideoPlayer::hide()
{
	mRenderer->hide();
}

void QtVideoPlayer::raise()
{
	mRenderer->raise();
}

void QtVideoPlayer::setMute(bool mute)
{
#if 1
	mPlayer->audio()->setMute(mute);
#else
	if (mute == true) {
		mPlayer->audio()->setBackends(mAudioOutputNull);
	}
	else {
		mPlayer->audio()->setBackends(mAudioOutputPriority);
	}
#endif
}

void QtVideoPlayer::setGeometry(int x, int y, int w, int h)
{
	mRenderer->setGeometry(x, y, w, h);
}

const QRect& QtVideoPlayer::geometry()
{
	return mRenderer->geometry();
}

void QtVideoPlayer::play(const std::string &file,bool islive,const std::string &clientip)
{
	mPlayer->setFrameRate(0);
	mPlayer->setInterruptTimeout((qint64)5000); // at last wait those seconds  Live --> local media.
	mPlayer->setInterruptOnTimeout(1);
	mPlayer->setBufferMode(BufferPackets);
	mPlayer->setBufferValue(30);
	mPlayer->setRepeat(0);
	mPlayer->masterClock()->setClockType(AVClock::AudioClock);

	//mPlayer->setAudioSyncParam(clientip.c_str(),islive);
	mPlayer->play(file.c_str());
}

bool QtVideoPlayer::isPlaying()
{
	return mPlayer->isPlaying();
}

void QtVideoPlayer::stop()
{
	//mRenderer->close();
	mPlayer->stop();
}

void QtVideoPlayer::setRunning(bool running)
{
	if(mIsLiveMedia)
	{
		setMute(running == false);
	}
	else
	{
		mPlayer->pause(running == false);
	}
}

float QtVideoPlayer::getCurrPosition()
{
	return mPlayer->position();
}

void QtVideoPlayer::setStartPosition(float pos)
{
	mPlayer->setStartPosition(pos);
}

const char *QtVideoPlayer::TAG = "QtVideoPlayer";

//const QStringList QtPlayer::mVideoDecoderPriority = QString("VAAPI,FFmpeg").split(",");
const QStringList QtVideoPlayer::mVideoDecoderPriority = QString("FFmpeg").split(",");
const QStringList QtVideoPlayer::mAudioOutputPriority = QString("Pulse").split(",");
const QStringList QtVideoPlayer::mAudioOutputNull = QString("null").split(",");//QStringList() << "null";
