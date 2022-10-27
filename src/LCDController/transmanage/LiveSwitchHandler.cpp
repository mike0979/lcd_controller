/*
 * LiveSwitchHandler.cpp
 *
 *  Created on: Nov 12, 2018
 *      Author: root
 */

#include "LiveSwitchHandler.h"
#include <config/configparser.h>
#include <Log.h>
#include <transmanage/TransManager.h>
#include <Message.h>
#include <json/ScheduleObjs.h>

const char * LiveSwitchHandler::TAG = "LiveSwitchHandler";


LiveSwitchHandler::LiveSwitchHandler(TransManager* manager) :
        ITransHandler(manager)
{
	mLiveSourceConfig = LIVE_AutoPlay;
}

LiveSwitchHandler::~LiveSwitchHandler()
{
}

void LiveSwitchHandler::Execute(Message* msg,int flag)
{
    if ( NULL == mTransManager)
    {
    	return;
    }

    if(mTransManager->GetControllerFlag() != LCD_Controller_flag)
    {
    	return;
    }


    LogI("[LiveSwitchHandler]-- Send [get livesource switch] req!\n ");
    mTransManager->sendRequest(this, DL_LiveSwithConfig, NULL);
}

std::string LiveSwitchHandler::ModifyPath(const TransFileType dltype,
        const std::string& oriPath, void* param)
{
    const ConfigParser* cfg = mTransManager->GetConfig();
    if (NULL == cfg)
        return oriPath;

    std::stringstream ssUrl;
    ssUrl << oriPath;

    switch (dltype)
    {
    case DL_LiveSwithConfig:
    {
        ssUrl << cfg->mDeviceId << "&station=" << cfg->mStationId;

        return ssUrl.str();
    }
    default:
        // Do not need any modification on the path.
        break;
    }
    return oriPath;
}

bool LiveSwitchHandler::handleMessage(Message* msg)
{
	if (NULL == msg)
		return false;

	int what = msg->mWhat;

	switch (what)
	{
	case DL_LiveSwithConfig:
	{
		LogI("[LiveSwitchHandler]--Handle [get live source switch config] reply!\n ");
		DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;
		HandleLiveSwitchReply(work, (DataTrans::DownloadStatus) msg->mArg1);

		DELETE_ALLOCEDRESOURCE(work);

		break;
	}
	}
	return true;
}

void LiveSwitchHandler::HandleLiveSwitchReply(DataTrans::TransWork *work,
        DataTrans::DownloadStatus status)
{
    if (DataTrans::DownloadSuccess != status || NULL == work)
    {
    	LogE("Download livesource switch config failed!\n");
    	return;
    }

	LogD("#####livesource switch config: %s\n",work->mJsonData.c_str());
	if(work->mJsonData.size() <= 2)
	{
		LogE("Live source switch config no content!\n");
		return;
	}

	ConfigParser* cfg = mTransManager->getLCDController()->getConfig();
	if(cfg == NULL)
	{
		LogE("cfg == NULL!\n");
		return;
	}

	Json::LiveSwitchDetail* sourceswitch = new Json::LiveSwitchDetail();
	bool ret = Json::LiveSwitchDetail::Parse(work->mJsonData.c_str(),sourceswitch);
	if (!ret)
	{
		LogE("LiveSwitchDetail parse failed!\n");
		DELETE_ALLOCEDRESOURCE(sourceswitch);
		return;
	}


	if(sourceswitch->mLiveSource.size() > 0)
	{
		std::vector<Json::LiveSourceSwitch>::iterator iter = sourceswitch->mLiveSource.begin();
		LogD("---------- current livesource: %d,   new livesource :%d\n",mLiveSourceConfig,iter->mPlaySource);
		if(mLiveSourceConfig != iter->mPlaySource)
		{
			mLiveSourceConfig = (LiveSourceSwitchConfig)iter->mPlaySource;
			mTransManager->sendMessage(new Message(LiveSourceSwitchUpdated, (int)mLiveSourceConfig));
		}
	}

	DELETE_ALLOCEDRESOURCE(sourceswitch);

}
