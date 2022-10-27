/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file name : comment_example.h
 * @author : alex
 * @date : 2017/7/14 14:29
 * @brief : open a file
 */

#include <config/configparser.h>
#include <curl/curl.h>
#include <json/ScheduleObjs.h>
#include <Log.h>
#include <Message.h>
#include <transmanage/ITransHandler.h>
#include <transmanage/TransHandlerFactory.h>
#include <websocket/WebSocketNotify.h>
#include <cstdio>
#include <iostream>

const char * WebSocketNotify::TAG = "WebSocketNotify";

WebSocketNotify::WebSocketNotify(TransManager* dlmanger) :
        mUrl(" "), mTransManager(dlmanger)
{
    mNotifyMessageType = UnknownType;
}

WebSocketNotify::~WebSocketNotify()
{
    if (mWSClient.GetStatus() == WebSocketClient::k_Connected)
    {
        mWSClient.Close("Normal close!");
    }
}

void WebSocketNotify::Init()
{
    if (NULL == mTransManager)
        return;

    // get device configuration.
    const ConfigParser* cfg = mTransManager->GetConfig();
    if (NULL == cfg)
        return;

    std::stringstream ssUrl;
//    ssUrl << "https://" << cfg->mCenterServerIP << ":" << cfg->mCenterServerPort
//            << cfg->mServerSubpath << cfg->mDeviceId;

    ssUrl << "http://" << cfg->mCenterServerIP << ":" << cfg->mWebsocketPort
                << cfg->mServerSubpath << cfg->mDeviceId;

    mUrl = ssUrl.str();
//    mUrl = "https://192.168.48.185:8089/api/v1/message/code_1";

    mWSClient.SetUrl(mUrl);
    mWSClient.SetInfoHandler(this->NotificationCallBack, (void*) this);
    mWSClient.SetCloseHandler(this->CloseCallBack, (void*) this);

    return;
}

void WebSocketNotify::SetToken(const std::string& token)
{
    mWSClient.SetToken(token);
}

void WebSocketNotify::Close()
{
    mWSClient.CloseSocket();
}

void WebSocketNotify::run()
{
    while (true)
    {
        //It will keep blocking until the connection broken.
        int ret = mWSClient.Connect();
        LogD("Connection with web-socket server-%s broken!! ret val=%d\n",
                mUrl.c_str(), ret);
        if (ret != CURLE_OK)
        {
            LogE("Connection with web-socket server-%s broken!\n",
                    mUrl.c_str());
            mWSClient.Close("");
        }

        WebSocketClient::ConnectState status = mWSClient.GetStatus();

        if (WebSocketClient::k_Connected != status)
        {
            Init();
            LogW("Try to reconnect web-socket server-%s!\n", mUrl.c_str());

            // Try Login.
            mTransManager->removeMessage(TransMessageType::LoginReq);
            mTransManager->sendMessage(new Message(TransMessageType::LoginReq));
            cancel();
            break;
        }
    }
}

int WebSocketNotify::NotificationCallBack(void* obj, std::string* data)
{
    WebSocketNotify* wsNotify = static_cast<WebSocketNotify*>(obj);

    if (NULL == wsNotify || NULL == wsNotify->mTransManager)
        return -1;

    // Parse the notification.
    NotifyMessageCode code;
    Json::ParseNtCode(data->c_str(), code);
    if (NTF_WS_ServerPING == code)
    { // got ping from server.
        //LogD("[WS-Notify]-- Got ping from server!\n");
        wsNotify->mTransManager->sendMessage(
                new Message(TransMessageType::WebSocketPingReq));
        return 0;
    }

    TransHandlerFactory* factory = TransHandlerFactory::Instance(
            wsNotify->mTransManager);

    ITransHandler* loader = factory->GetLoader(code);

    if(NULL == loader)
        return -1;

    LogI("[WS-Notify]--Notify code[%d]!\n",code);
    wsNotify->mTransManager->sendMessage(
            new Message(TransMessageType::DownLoadReq, loader), 0);

    return 0;
}

int WebSocketNotify::CloseCallBack(void* obj, std::string* data)
{
    WebSocketNotify* wsNotify = static_cast<WebSocketNotify*>(obj);
    if (obj != NULL)
    {
        LogD("----- CloseCallBack[%s] -----\n", wsNotify->mUrl.c_str());
    }

    return 0;
}
