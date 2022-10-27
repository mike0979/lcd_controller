/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : IDownLoader.cpp
 * @author : Benson
 * @date : Sep 22, 2017
 * @brief :
 */

#include <config/configparser.h>
#include <Looper.h>
#include <transmanage/ITransHandler.h>
#include <transmanage/TransManager.h>

ITransHandler::ITransHandler(TransManager* manager) :
        mTransManager(manager)
{
    Handler::setLooper(manager->getLooper());
}

ITransHandler::~ITransHandler()
{
}

std::string ITransHandler::ModifyPath(const TransFileType dltype,
        const std::string& oriPath, void* param)
{
    return oriPath;
}

const TransManager* ITransHandler::GetManager()
{
    return mTransManager;
}

