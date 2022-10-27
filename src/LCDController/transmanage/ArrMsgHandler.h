/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : ArrMsgHandler.h
 * @author : Benson
 * @date : Sep 21, 2017
 * @brief :
 */

#ifndef TRANSMANAGE_ARRMSGHANDLER_H_
#define TRANSMANAGE_ARRMSGHANDLER_H_

#include <CommonDef.h>
#include <DataTrans.h>
#include <transmanage/ITransHandler.h>
#include <string>
#include <json/ScheduleObjs.h>

class ArrMsgHandler: public ITransHandler
{
public:
    ArrMsgHandler(TransManager* manager);
    virtual ~ArrMsgHandler();

    virtual void Execute(Message * msg,int flag=0);

    virtual std::string ModifyPath(const TransFileType dltype,
            const std::string& oriPath, void* param);

protected:
    virtual bool handleMessage(Message *msg);
private:
    /**
     * Handle "get arrival information" reply .
     * @param work
     * @param status
     */
    void HandleArrivalInfoReply(DataTrans::TransWork *work,
            DataTrans::DownloadStatus status);

    bool isArrMsgUpdated(Json::ArrivalDetail* arrinfo);

    Json::ArrivalDetail mRTArrmsg;

    int mSignalinterruptDuration;

    static const char *TAG;
};

#endif /* TRANSMANAGE_ARRMSGHANDLER_H_ */
