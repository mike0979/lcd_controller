/*
 * TrainTimeHandler.cpp
 *
 *  Created on: Apr 8, 2018
 *      Author: root
 */

#include "TrainTimeHandler.h"
#include <config/configparser.h>
#include <Log.h>
#include <transmanage/TransManager.h>
#include <Message.h>
#include <json/ScheduleObjs.h>

const char * TrainTimeHandler::TAG = "TrainTimeHandler";

TrainTimeHandler::TrainTimeHandler(TransManager* manager) :
        ITransHandler(manager)
{
	mFirstShowFromLocal = true;
}

TrainTimeHandler::~TrainTimeHandler()
{
}

void TrainTimeHandler::Execute(Message* msg,int flag)
{
    if ( NULL == mTransManager)
    {
    	return;
    }

    if(mTransManager->GetControllerFlag() != LCD_Controller_flag)
    {
    	return;
    }

    if(mFirstShowFromLocal)
    {
    	sendMessage(new Message(DL_TrainTimeLoadFromLocal));
    	mFirstShowFromLocal = false;
    }

    LogI("[ArrInfo]-- Send [get first-last train time] req!\n ");
    mTransManager->sendRequest(this, DL_TrainTimeConfig, NULL);
}

std::string TrainTimeHandler::ModifyPath(const TransFileType dltype,
        const std::string& oriPath, void* param)
{
    const ConfigParser* cfg = mTransManager->GetConfig();
    if (NULL == cfg)
        return oriPath;

    std::stringstream ssUrl;
    ssUrl << oriPath;

    switch (dltype)
    {
    case DL_TrainTimeConfig:
    {
        ssUrl << cfg->mDeviceId ;

        return ssUrl.str();
    }
    default:
        // Do not need any modification on the path.
        break;
    }
    return oriPath;
}

bool TrainTimeHandler::handleMessage(Message* msg)
{
	if (NULL == msg)
		return false;

	int what = msg->mWhat;

	switch (what)
	{
	case DL_TrainTimeConfig:
	{
		LogI("[TrainTime]--Handle [get first_last train time] reply!\n ");
		DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;
		HandleTrainTimeReply(work, (DataTrans::DownloadStatus) msg->mArg1);

		DELETE_ALLOCEDRESOURCE(work);

		break;
	}
	case DL_TrainTimeLoadFromLocal:
	{
		LogI("[TrainTime]-- Handle [get first-last train time] from localfile!\n ");
		HandleTrainTimeReply(NULL,DataTrans::DownloadCmdError);
		break;
	}
	case DL_ReDownloadTrainTimeConfig:
	{
		LogI("[TrainTime]-- retry download [get first-last train time] req!\n ");
		mTransManager->sendRequest(this, DL_TrainTimeConfig, NULL);
		break;
	}
	}
	return true;
}

void TrainTimeHandler::HandleTrainTimeReply(DataTrans::TransWork *work,
        DataTrans::DownloadStatus status)
{
	const ConfigParser* cfg = mTransManager->GetConfig();
	std::string sfilename = cfg->mDldRootDir + cfg->mSchPlayPath + "traintimeconfig.json";
	Json::TrainTimeDetail* traintime = new Json::TrainTimeDetail();

    if (DataTrans::DownloadSuccess != status || NULL == work)
    {
    	LogD("Get first_last train config form local(%s)\n",sfilename.c_str());
    	bool bret = Json::TrainTimeDetail::Load(sfilename,traintime);
    	if(!bret)
        {
    		LogE("Load TrainTimeDetail from local failed!\n");
    		DELETE_ALLOCEDRESOURCE(traintime);
    		removeMessage(DL_ReDownloadTrainTimeConfig);
    		sendMessage(new Message(DL_ReDownloadTrainTimeConfig),5000);
    		return;
        }
    	else
    	{
    		LogD("Get first_last train time form local success.\n");
    	}
    }
    else
    {
    	bool bgetfromserverok = true;
    	LogD("#####first_last train time config : %s\n",work->mJsonData.c_str());
    	if(work->mJsonData.size() <= 2)
    	{
			LogE("TrainTimeDetail no content!\n");
			bgetfromserverok = false;
			//DELETE_ALLOCEDRESOURCE(traintime);
			//return;
		}

        bool ret = Json::TrainTimeDetail::Parse(work->mJsonData.c_str(),traintime);
        if (!ret)
        {
            LogE("TrainTimeDetail parse failed!\n");
            bgetfromserverok = false;
            //DELETE_ALLOCEDRESOURCE(traintime);
            //return;
        }

        if(traintime != NULL && traintime->mTrainsTime.size() == 0)
        {
        	LogE("TrainTimeDetail vector no content!\n");
        	bgetfromserverok = false;
        }

        if(bgetfromserverok)
        {
        	LogD("Save json to file!\n");
        	Json::TrainTimeDetail::Save(sfilename,work->mJsonData.c_str());
        }
        else
        {
        	LogD("Get first_last train config form local after download fail\n");
			bool bret = Json::TrainTimeDetail::Load(sfilename,traintime);
			if(!bret)
			{
				LogE("Load TrainTimeDetail from local failed again!\n");
				DELETE_ALLOCEDRESOURCE(traintime);
				removeMessage(DL_ReDownloadTrainTimeConfig);
				sendMessage(new Message(DL_ReDownloadTrainTimeConfig),5000);
				return;
			}
			else
			{
				LogD("Get first_last train time form local again success.\n");
			}
        }
    }

	if (traintime != NULL && traintime->mTrainsTime.size() == 0)
    {
    	LogE("TrainTimeDetail vector no content !\n");
    	removeMessage(DL_ReDownloadTrainTimeConfig);
    	sendMessage(new Message(DL_ReDownloadTrainTimeConfig),5000);
    	DELETE_ALLOCEDRESOURCE(traintime);
    	return;
    }

    LogD("Send first_last train time to play.\n");
    if(traintime != NULL)
    {
		LogD("operation_first_train info,mStationCode:%s  mUpdateTime:%s size:%d\n",
				traintime->mStationCode.c_str(),
				traintime->mUpdateTime.c_str(),
				traintime->mTrainsTime.size());
    	for(std::vector<Json::TrainTime>::iterator it_trainT = traintime->mTrainsTime.begin();
    								it_trainT != traintime->mTrainsTime.end();++it_trainT)
    	{
    		LogD("operation_first_train info, direction: %s  - %s - %s\n",
				it_trainT->mUpDown.c_str(),
				it_trainT->mFirstTrainTime.c_str(),
				it_trainT->mLastTrainTime.c_str());
    	}
    }

    mTransManager->sendMessage(new Message(TrainTimeUpdated, traintime));

}


