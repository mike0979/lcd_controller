/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file : Version.cpp 
* @author : Benson
* @date : Nov 13, 2017
* @brief :
*/
#include "Version.h"

std::string Version::mVersion = "0.0.4";

const std::string& Version::GetVersion()
{
    return mVersion;
}
