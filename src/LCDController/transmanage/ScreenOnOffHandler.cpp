/*
 * ScreenOnOffHandler.cpp
 *
 *  Created on: Sep 27, 2018
 *      Author: root
 */


#include "ScreenOnOffHandler.h"
#include <config/configparser.h>
#include <Log.h>
#include <transmanage/TransManager.h>
#include <Message.h>
#include <json/ScheduleObjs.h>

const char * ScreenOnOffHandler::TAG = "ScreenOnOffHandler";

ScreenOnOffHandler::ScreenOnOffHandler(TransManager* manager) :
        ITransHandler(manager)
{

}

ScreenOnOffHandler::~ScreenOnOffHandler()
{
}

void ScreenOnOffHandler::Execute(Message* msg,int flag)
{
    if ( NULL == mTransManager)
    {
    	return;
    }

    if(mTransManager->GetControllerFlag() != LCD_Controller_flag)
    {
    	return;
    }


    LogI("[ScreenOnOff Time]-- Send [get screen on off time] req!\n ");
    mTransManager->sendRequest(this, DL_ScreenOnOffConfig, NULL);
}

std::string ScreenOnOffHandler::ModifyPath(const TransFileType dltype,
        const std::string& oriPath, void* param)
{
    const ConfigParser* cfg = mTransManager->GetConfig();
    if (NULL == cfg)
        return oriPath;

    std::stringstream ssUrl;
    ssUrl << oriPath;

    switch (dltype)
    {
    case DL_ScreenOnOffConfig:
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

bool ScreenOnOffHandler::handleMessage(Message* msg)
{
	if (NULL == msg)
		return false;

	int what = msg->mWhat;

	switch (what)
	{
	case DL_ScreenOnOffConfig:
	{
		LogI("[ScreenOnOffHandler]--Handle [get screen on-off time config] reply!\n ");
		DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;
		HandleScreenOnOffReply(work, (DataTrans::DownloadStatus) msg->mArg1);

		DELETE_ALLOCEDRESOURCE(work);

		break;
	}
	}
	return true;
}

void ScreenOnOffHandler::HandleScreenOnOffReply(DataTrans::TransWork *work,
        DataTrans::DownloadStatus status)
{
    if (DataTrans::DownloadSuccess != status || NULL == work)
    {
    	LogE("Download screen on off config failed!\n");
    	return;
    }

	LogD("#####screen on-off time config: %s\n",work->mJsonData.c_str());
	if(work->mJsonData.size() <= 2)
	{
		LogE("Screen on-off config no content!\n");
		return;
	}

	ConfigParser* cfg = mTransManager->getLCDController()->getConfig();
	if(cfg == NULL)
	{
		LogE("cfg == NULL!\n");
		return;
	}

	Json::ScreenOnOffDetail* onofftime = new Json::ScreenOnOffDetail();
	bool ret = Json::ScreenOnOffDetail::Parse(work->mJsonData.c_str(),onofftime);
	if (!ret)
	{
		LogE("ScreenOnOffDetail parse failed!\n");
		DELETE_ALLOCEDRESOURCE(onofftime);
		return;
	}

	bool bontimechanged = false;
	bool bofftimechanged = false;
	if (onofftime->mScreenOnOff.size() > 0)
	{
		std::vector<Json::ScreenOnOffTime>::iterator iter = onofftime->mScreenOnOff.begin();
		LogD("---------- ontime: %s,   offtime:%s\n",iter->mScreenOnTime.c_str(),iter->mScreenOffTime.c_str());
		if(iter->mScreenOnTime != cfg->mPoweronLcdTime)
		{
			cfg->mPoweronLcdTime = iter->mScreenOnTime;
			bontimechanged = true;
		}
		else
		{
			LogD("--- screen poweron time do not change\n");
		}

		if(iter->mScreenOffTime != cfg->mShutdownLcdTime)
		{
			cfg->mShutdownLcdTime = iter->mScreenOffTime;
			bofftimechanged = true;
		}
		else
		{
			LogD("--- screen shutdowb time do not change\n");
		}

		if(bontimechanged || bofftimechanged)
		{
			cfg->ModifyScreenOnOffTime(cfg->mPoweronLcdTime,cfg->mShutdownLcdTime);
			mTransManager->sendMessage(new Message(ScreenOnOffTimeUpdated));
		}
	}
	else
	{
		LogD("onofftime->mScreenOnOff.size() == 0\n");
	}

	DELETE_ALLOCEDRESOURCE(onofftime);

}


