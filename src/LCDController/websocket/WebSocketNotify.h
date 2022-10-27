/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef DOWNLOAD_WEBSOCKETNOTIFY_H_
#define DOWNLOAD_WEBSOCKETNOTIFY_H_

#include <transmanage/TransManager.h>
#include "CommonDef.h"
#include "websocketclient.h"
#include "Thread.h"
#include <string>

class WebSocketNotify : public Thread {
public:
	WebSocketNotify(TransManager* dlmanger);
	~WebSocketNotify();

	void Init();

	void SetToken(const std::string& token);

	// close the connection to server.
	void Close();
private:
	static int NotificationCallBack(void* obj, std::string* data);
	static int CloseCallBack(void* obj, std::string* data);

    virtual void run();
private:
	std::string mUrl;
	TransManager* mTransManager;
	NotifyMessageCode mNotifyMessageType;
	WebSocketClient mWSClient; // web socket client.

	static const char *TAG;
};


#endif /* DOWNLOAD_WEBSOCKETNOTIFY_H_ */
