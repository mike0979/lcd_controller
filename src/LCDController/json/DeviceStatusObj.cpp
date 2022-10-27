/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : DeviceStatusObj.cpp
 * @author : Benson
 * @date : Nov 11, 2017
 * @brief :
 */

#include <FileSysUtils.h>
#include <json/DeviceStatusObj.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include "Log.h"

using namespace rapidjson;

Json::HardwareStatus::HardwareStatus() :
        mStatus(S_OFF_LINE), mCpuRadio(0.0f)
{
    mMemoryInfo.mMemoryRatio = 0.0f;
    mMemoryInfo.mMemorySize = 0;
}

Json::HardwareStatus::~HardwareStatus()
{
}

Json::SoftwareStatus::SoftwareStatus() :
        mSysVersion(""), mStatus(S_NotRunning), mMode(M_Local), mCpuRatio(0.0f)
{
    mHddInfo.mHddRatio = 0.0f;
    mHddInfo.mHddSize = 0;

    mMemoryInfo.mMemoryRatio = 0.0f;
    mMemoryInfo.mMemorySize = 0;
}

Json::SoftwareStatus::~SoftwareStatus()
{
}

/**
 * A line info is "PID    USER     PR  NI    VIRT    RES    SHR     S   %CPU   %MEM     TIME+      COMMAND"
 */
/**
     * Get local device software status.
     * include:
     *          1. this software used cpu ration.
     *          2. this software used memory ration.
     * @param[out]
     */
bool Json::SoftwareStatus::GetBasicStatus(const std::string& processName,
        SoftwareStatus& basicInfo)
{
    if (processName.size() < 1)
        return false;

    bool ret = false;

    std::string cmd("top -n 1 -d 1 -b | grep ");
    cmd.append("\"");
    cmd.append(processName);
    cmd.append("\"");
    cmd.append(" &");

    //LogD("popen command - %s\n",cmd.c_str());

    FILE *freport = popen(cmd.c_str(), "r");
    if (freport == NULL)
    {
        return false;
    }

    char linebuf[1024];
    usleep(1000 * 1000 * 2);

    if (fgets(linebuf, sizeof(linebuf), freport) != NULL)
    {
        //std::cout << linebuf << std::endl;
        char* token = strtok(linebuf, " ");
        std::vector<std::string> buflist;
        buflist.clear();
        while (token != NULL)
        {
            buflist.push_back(token);
            token = strtok(NULL, " ");
        }

        if(buflist.size() == 12 )
        {
            std::stringstream cpu_sstrm(buflist[8]);
            // get the cpu ratio.
            cpu_sstrm >> basicInfo.mCpuRatio;

            // get the memory ratio.
            std::stringstream mem_sstrm(buflist[9]);
            mem_sstrm >> basicInfo.mMemoryInfo.mMemoryRatio;
            //std::cout << "MemoryRatio "<< basicInfo.mMemoryInfo.mMemoryRatio<<std::endl;

            ret = true;
        }
    }
    else
    {
        ret = false;
    }

    pclose(freport);
    return ret;
}

Json::DeviceStatus::DeviceStatus()
{
}

Json::DeviceStatus::~DeviceStatus()
{
}

bool Json::DeviceStatus::ToJson(const DeviceStatus& status,
        std::string& jsonStr)
{
    Document doc;
    doc.SetObject();

    Document::AllocatorType &allocator = doc.GetAllocator();

    // hardware
    rapidjson::Value hdObj(rapidjson::kObjectType);
    hdObj.AddMember("system_status", status.mHardware.mStatus, allocator);
    hdObj.AddMember("cpu_ratio", status.mHardware.mCpuRadio, allocator);

    rapidjson::Value hddInfoArray(rapidjson::kArrayType);
    for (unsigned i = 0; i < status.mHardware.mHddInfoVec.size(); ++i)
    {
        HddInfo tempHddInfo = status.mHardware.mHddInfoVec[i];

        rapidjson::Value tempHddObj(rapidjson::kObjectType);
        tempHddObj.AddMember("hdd_ratio", tempHddInfo.mHddRatio, allocator);
        tempHddObj.AddMember("hdd_size", tempHddInfo.mHddSize, allocator);
        hddInfoArray.PushBack(tempHddObj, allocator);
    }
    hdObj.AddMember("hdds", hddInfoArray, allocator);

    hdObj.AddMember("memory_ratio", status.mHardware.mMemoryInfo.mMemoryRatio,
             allocator);
    hdObj.AddMember("memory_size", status.mHardware.mMemoryInfo.mMemorySize,
             allocator);
    doc.AddMember("hardware", hdObj, allocator);

    // software
    rapidjson::Value sftObj(rapidjson::kObjectType);
    sftObj.AddMember("system_version",
            Value(status.mSoftware.mSysVersion.c_str(), allocator), allocator);
    sftObj.AddMember("system_status", status.mSoftware.mStatus, allocator);

    sftObj.AddMember("work_mode", status.mSoftware.mMode, allocator);
    sftObj.AddMember("cpu_ratio", status.mSoftware.mCpuRatio, allocator);

    sftObj.AddMember("hdd_ratio", status.mSoftware.mHddInfo.mHddRatio,
            allocator);
    sftObj.AddMember("hdd_size", status.mSoftware.mHddInfo.mHddSize, allocator);

    sftObj.AddMember("memory_ratio", status.mHardware.mMemoryInfo.mMemoryRatio,
            allocator);
    sftObj.AddMember("memory_size", status.mHardware.mMemoryInfo.mMemorySize,
            allocator);

    doc.AddMember("software", sftObj, allocator);

    StringBuffer buffer;
    PrettyWriter<StringBuffer> pretty_writer(buffer);
    doc.Accept(pretty_writer);

    jsonStr = buffer.GetString();

    return true;
}

const char *Json::HardwareStatus::TAG = "HardwareStatus";
const char *Json::SoftwareStatus::TAG = "SoftwareStatus";
const char *Json::DeviceStatus::TAG = "DeviceStatus";
