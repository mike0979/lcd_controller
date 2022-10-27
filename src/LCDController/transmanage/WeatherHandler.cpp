/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file : WeatherHandler.cpp
* @author : Benson
* @date : Sep 21, 2017
* @brief :
*/

#include <transmanage/WeatherHandler.h>

WeatherHandler::WeatherHandler(TransManager* manager)
:ITransHandler(manager)
{
}

WeatherHandler::~WeatherHandler()
{
}

void WeatherHandler::Execute(Message* msg,int flag)
{
}

std::string WeatherHandler::ModifyPath(const TransFileType dltype,
        const std::string& oriPath, void* param)
{
    return "";
}

bool WeatherHandler::handleMessage(Message* msg)
{
    return false;
}
