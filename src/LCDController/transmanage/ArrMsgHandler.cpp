/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : ArrMsgHandler.cpp
 * @author : Benson
 * @date : Sep 21, 2017
 * @brief :
 */

#include <config/configparser.h>
#include <json/ScheduleObjs.h>
#include <Log.h>
#include <Message.h>
#include <transmanage/ArrMsgHandler.h>
#include <transmanage/TransManager.h>
#include <sstream>

const char * ArrMsgHandler::TAG = "ArrMsgHandler";

ArrMsgHandler::ArrMsgHandler(TransManager* manager) :
        ITransHandler(manager)
{
	mRTArrmsg.mTrains.clear();

	const ConfigParser* cfg = mTransManager->GetConfig();

	mSignalinterruptDuration = cfg->mSignalInterruptDuration;

	//sendMessage(new Message(DL_CheckArrivalInfo),mSignalinterruptDuration*1000);
}

ArrMsgHandler::~ArrMsgHandler()
{
}

void ArrMsgHandler::Execute(Message* msg,int flag)
{
    if ( NULL == mTransManager)
    {
    	return;
    }

    if(mTransManager->GetControllerFlag() != LCD_Controller_flag)
    {
    	return;
    }

    LogI("[ArrInfo]-- Send [get arrival information] req!\n ");
    mTransManager->sendRequest(this, DL_ArrivalInfo, NULL);
}

std::string ArrMsgHandler::ModifyPath(const TransFileType dltype,
        const std::string& oriPath, void* param)
{
    const ConfigParser* cfg = mTransManager->GetConfig();
    if (NULL == cfg)
        return oriPath;

    std::stringstream ssUrl;
    ssUrl << oriPath;

    switch (dltype)
    {
    case DL_ArrivalInfo:
    {
        ssUrl << cfg->mDeviceId << "&platform=" << cfg->mPlatformId;

        return ssUrl.str();
    }
    default:
        // Do not need any modification on the path.
        break;
    }
    return oriPath;
}

bool ArrMsgHandler::handleMessage(Message* msg)
{
    if (NULL == msg)
        return false;

    int what = msg->mWhat;

    switch (what)
    {
    case DL_ArrivalInfo:
    { // handle "get arrival information" reply.
        LogI("[ArrInfo]--Handle [get arrival information] reply!\n ");
        DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;
        HandleArrivalInfoReply(work, (DataTrans::DownloadStatus) msg->mArg1);

        DELETE_ALLOCEDRESOURCE(work);

        break;
    }
    case DL_CheckArrivalInfo:
    {
    	Json::ArrivalDetail* arrInfo = NULL;
    	mTransManager->sendMessage(new Message(ArrivalInfoUpdated, arrInfo));
    	mRTArrmsg.mTrains.clear();

    	removeMessage(DL_CheckArrivalInfo);
    	sendMessage(new Message(DL_CheckArrivalInfo),mSignalinterruptDuration*1000);
    	break;
    }
    }
    return false;
}

void ArrMsgHandler::HandleArrivalInfoReply(DataTrans::TransWork *work,
        DataTrans::DownloadStatus status)
{
    if (DataTrans::DownloadSuccess != status || NULL == work)
    {
        // TODO: Download failed.
        return;
    }

    LogD("RTArrival msg#################  :  %s\n",work->mJsonData.c_str());
    // Download succeed, Then parse the result.
    // this object was deleted in QtSchedule.cpp
    Json::ArrivalDetail* arrInfo = new Json::ArrivalDetail();
    bool ret = Json::ArrivalDetail::Parse(work->mJsonData.c_str(),arrInfo);
    if (!ret)
    {
        LogE("ArrivalDetail parse failed!\n");
        DELETE_ALLOCEDRESOURCE(arrInfo);
        return;
    }

    removeMessage(DL_CheckArrivalInfo);
    sendMessage(new Message(DL_CheckArrivalInfo),mSignalinterruptDuration*1000);

    if(isArrMsgUpdated(arrInfo))
    {
    	 LogD("new Arrival message and send arrival msg to play.\n");
    	 mRTArrmsg = *arrInfo;
    	 mTransManager->sendMessage(new Message(ArrivalInfoUpdated, arrInfo));
    }
    else
    {
    	DELETE_ALLOCEDRESOURCE(arrInfo);
    	LogD("This Arrival msg don't need send to play.\n");
    }

}

bool ArrMsgHandler::isArrMsgUpdated(Json::ArrivalDetail* arrinfo)
{
	bool bret = false;

//	if(arrinfo->mTrains.size() == 0)
//	{
//		return false;
//	}

	if(mRTArrmsg.mTrains.size() != arrinfo->mTrains.size())
	{
		return true;
	}

	int size = arrinfo->mTrains.size();
	for(int i=0;i<size;++i)
	{
		if(mRTArrmsg.mTrains[i].mArrivalTime != arrinfo->mTrains[i].mArrivalTime)
		{
			bret = true;
			break;
		}
	}

	return bret;
}
