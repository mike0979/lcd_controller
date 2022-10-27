/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/

#include "QtFlash.h"
#include "Log.h"

#include <QtCore/QFileInfo>

#include <stdio.h>

QtFlash::QtFlash(QWidget *parent)
{
	mWebView = new QWebView(parent);
    this->setBackgroundRole(QPalette::Window);
	this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	this->setAttribute(Qt::WA_TranslucentBackground,false);

}

QtFlash::~QtFlash()
{
	if(mWebView != NULL)
	{
		delete mWebView;
		mWebView = NULL;
	}

}

void QtFlash::show()
{
	mWebView->show();
}

void QtFlash::hide()
{
	mWebView->hide();
}

void QtFlash::raise()
{
	mWebView->raise();
}

void QtFlash::setZoomFactor(float factor)
{
	mWebView->setZoomFactor(factor);
}

void QtFlash::setGeometry(int x, int y, int w, int h)
{
	mWebView->setGeometry(x, y, w, h);
}

const QRect& QtFlash::geometry()
{
	return mWebView->geometry();
}

void QtFlash::setRunning(bool running)
{
//	LogD("TODO : add setRunning support for flash\n");
}

void QtFlash::play(const std::string &swf, int width, int height, int volume)
{
	std::string file, path;

	std::size_t split = swf.find_last_of("/\\");
	if (split == std::string::npos) {
		file = swf;
	}
	else {
		path = swf.substr(0, split + 1);
		file = swf.substr(split + 1);
	}

	char html[1024];
	snprintf(html, sizeof(html),
			"<html><body><style type=text/css><!--body {margin-left:0px; margin-top:0px; margin-right:0px; margin-bottom:0px; overflow-y:hidden;}--></style><embed src=\"%s\" width=%d height=%d  wmode=transparent volume=%d type=application/x-shockwave-flash /></embed></body></html>",
			file.c_str(), width, height, volume);

	mWebView->setHtml(html, QUrl::fromLocalFile(QFileInfo(path.c_str()).absoluteFilePath()));
	LogD("html = %s\npath = %s\n", html,path.c_str());
}

const char *QtFlash::TAG = "QtFlash";


