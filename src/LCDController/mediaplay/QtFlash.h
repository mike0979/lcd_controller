/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef MEDIAPLAY_QTFLASH_H_
#define MEDIAPLAY_QTFLASH_H_

#include <QtWebKit/QWebView>

class QtFlash : public QWidget {
public:
	QtFlash(QWidget *parent = 0);
	~QtFlash();

	void show();
	void hide();
	void raise();

	void setZoomFactor(float factor);

	void setGeometry(int x, int y, int w, int h);
	const QRect& geometry();

	void setRunning(bool running);
	void play(const std::string &swf, int width, int height, int volume);

private:
	static const char *TAG;

	QWebView *mWebView;
};

#endif /* MEDIAPLAY_QTFLASH_H_ */
