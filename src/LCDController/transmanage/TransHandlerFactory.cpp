/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : TransHandlerFactory.cpp
 * @author : Benson
 * @date : Sep 20, 2017
 * @brief :
 */

#include <transmanage/ArrMsgHandler.h>
#include <transmanage/CmdHandler.h>
#include <transmanage/OPSHandler.h>
#include <transmanage/ScheduleHandler.h>
#include <transmanage/TransHandlerFactory.h>
#include <transmanage/TransHandlerFactory.h>
#include <transmanage/WeatherHandler.h>
#include <transmanage/TrainTimeHandler.h>
#include <transmanage/ScreenOnOffHandler.h>
#include <transmanage/LiveSwitchHandler.h>
#include <string>

std::map<NotifyMessageCode,ITransHandler*> TransHandlerFactory::mDownLoaders;
TransHandlerFactory* TransHandlerFactory::mInstance = NULL;
Mutex TransHandlerFactory::mMutex;

TransHandlerFactory* TransHandlerFactory::Instance(TransManager* manager)
{
    if(NULL == mInstance)
    {
        mMutex.lock();
        if(NULL == mInstance)
            mInstance = new TransHandlerFactory(manager);
        mMutex.unlock();
    }

    return mInstance;
}

TransHandlerFactory::TransHandlerFactory(TransManager* manager):mDldManager(NULL)
{
    mDownLoaders[NTF_CommandUpdated] = new CmdHandler(manager);
    mDownLoaders[NTF_RTOPSMsgUpdated] = new OPSHandler(manager);
    mDownLoaders[NTF_ScheduleUpdated] = new ScheduleHandler(manager);
    mDownLoaders[NTF_RTWeatherUpdated] = new WeatherHandler(manager);
    mDownLoaders[NTF_RTArrMsgUpdated] = new ArrMsgHandler(manager);
    //mDownLoaders[NTF_TrainTimeUpdated] = new TrainTimeHandler(manager);
    mDownLoaders[NTF_ScreenOnOffUpdated] = new ScreenOnOffHandler(manager);
    mDownLoaders[NTF_LiveSourceUpdated] = new LiveSwitchHandler(manager);
}

void TransHandlerFactory::Destory()
{
    if(mInstance != NULL)
    {
        delete mInstance;
        mInstance = NULL;
    }
}

TransHandlerFactory::~TransHandlerFactory()
{
    std::map<NotifyMessageCode, ITransHandler*>::iterator itor =
            mDownLoaders.begin();
    do
    {
        if (itor->second != NULL)
            delete itor->second;
        itor++;
    } while (itor != mDownLoaders.end());

    mDownLoaders.clear();
}

ITransHandler* TransHandlerFactory::GetLoader(const NotifyMessageCode code)
{
    ITransHandler* ret = NULL;
    switch (code)
    {
    case NTF_CommandUpdated:
    case NTF_RTOPSMsgUpdated:
    case NTF_ScheduleUpdated:
    case NTF_RTWeatherUpdated:
    case NTF_RTArrMsgUpdated:
    //case NTF_TrainTimeUpdated:
    case NTF_ScreenOnOffUpdated:
    case NTF_LiveSourceUpdated:
        ret = mDownLoaders[code];
        break;
    default:
        break;
    }

    return ret;
}
