/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : UpdateListHandler.cpp
 * @author : Benson
 * @date : Sep 6, 2017
 * @brief :
 */
#include <json/UpdateListFileHandler.h>
#include "rapidjson/reader.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include <stdio.h>

using namespace rapidjson;
static const int WriteBufSize = 1024 * 10;

Json::UpdateListFileHandler::UpdateListFileHandler()
{
}

Json::UpdateListFileHandler::UpdateListFileHandler(const std::string& fileName) :
        UpdateListFileBase(fileName)
{
}

Json::UpdateListFileHandler::~UpdateListFileHandler()
{
}

void Json::UpdateListFileHandler::SetFileName(const std::string& fileName)
{
    mFileName = fileName;
}

bool Json::UpdateListFileHandler::UpdateList(const char* jsonStr,
        ScheduleList* data)
{
    if (jsonStr == NULL || data == NULL)
        return false;

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);
    mDoc.CopyFrom(d,mDoc.GetAllocator());

    if (!d.IsArray())
    {
        return false;
    }

    Value array = d.GetArray();
    ScheduleBasic basic;
    for (unsigned i = 0; i < array.Size(); ++i)
    {
        const Value& obj = array[i];

        basic.mId = obj["id"].GetInt();
        basic.mName = obj["name"].GetString();
        basic.mDescrp = obj["description"].GetString();
        basic.mStartTime = obj["start_time"].GetString();
        basic.mEndTime = obj["end_time"].GetString();
        basic.mUpdateTime = obj["update_time"].GetString();
        basic.mPublishTime = obj["publish_time"].GetString();

        data->mSchedules.push_back(basic);
    }

    return true;
}

bool Json::UpdateListFileHandler::CheckUpdateStatus(const ScheduleList& newlist,
        const ScheduleList& oldlist, std::list<int>& keeplist,std::list<int>& updatelist,
        std::list<int>& deletlist)
{
	keeplist.clear();
    updatelist.clear();
    deletlist.clear();

    ScheduleBasic oldSch, newSch;
    bool bOldSchFound = false;
    bool bNewSchFound = false;

    // check new schedule list updated or deleted schedule.
    for (uint i = 0; i < oldlist.mSchedules.size(); i++)
    {
        bOldSchFound = false;

        oldSch = oldlist.mSchedules[i];
        for (uint j = 0; j < newlist.mSchedules.size(); j++)
        {
            newSch = newlist.mSchedules[j];

            if (newSch.mId == oldSch.mId)
            { // found
              // check update time
            	printf("*** newsch publisttime: %s,  oldsch publisttime:%s\n",newSch.mPublishTime.c_str(),oldSch.mPublishTime.c_str());
                if (newSch.mPublishTime > oldSch.mPublishTime)
                {
                	printf("---updated schedule: %d\n",newSch.mId);
                	updatelist.push_back(newSch.mId);
                }
                else if(newSch.mPublishTime == oldSch.mPublishTime)
                {
                	printf("---keeped schedule: %d\n",newSch.mId);
                	keeplist.push_back(newSch.mId);
                }
                bOldSchFound = true;
                break;
            }
        }

        if (!bOldSchFound) // old schedule id not found in new schedule list
            deletlist.push_back(oldSch.mId);
    }

    // check new schedule list added schedule.
    for (uint i = 0; i < newlist.mSchedules.size(); i++)
    {
        bNewSchFound = false;

        newSch = newlist.mSchedules[i];
        for (uint j = 0; j < oldlist.mSchedules.size(); j++)
        {
            oldSch = oldlist.mSchedules[j];

            if (newSch.mId == oldSch.mId)
            { // found
                bNewSchFound = true;
                break;
            }
        }

        if (!bNewSchFound) // new schedule id not found in old schedule list
        {
        	printf("---added schedule: %d\n",newSch.mId);
        	updatelist.push_back(newSch.mId);
        }
//        else
//        	keeplist.push_back(newSch.mId);
    }

    if (updatelist.size() > 0 || deletlist.size() > 0 || keeplist.size() > 0)
        return true;
    else
        return false;
}

bool Json::UpdateListFileHandler::LoadSchlist(const std::string& fileName,
        ScheduleList& schlist)
{
    FILE* fp = fopen(fileName.c_str(), "r");
    if (NULL == fp)
        return false;

    char readBuffer[WriteBufSize];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    Document d;
    d.ParseStream(is);

    fclose(fp);

    if (!d.IsArray())
    {
        return false;
    }

    Value array = d.GetArray();
    ScheduleBasic basic;
    for (unsigned i = 0; i < array.Size(); ++i)
    {
        const Value& obj = array[i];

        basic.mId = obj["id"].GetInt();
        basic.mName = obj["name"].GetString();
        basic.mDescrp = obj["description"].GetString();
        basic.mStartTime = obj["start_time"].GetString();
        basic.mEndTime = obj["end_time"].GetString();
        basic.mUpdateTime = obj["update_time"].GetString();
        basic.mPublishTime = obj["publish_time"].GetString();

        schlist.mSchedules.push_back(basic);
    }

    return true;
}

void Json::UpdateListFileHandler::Save()
{
    FILE* fp = fopen(mFileName.c_str(), "w+");
    if (fp == NULL)
        return;

    char writeBuffer[WriteBufSize];
    FileWriteStream os(fp, writeBuffer, WriteBufSize);
    PrettyWriter<FileWriteStream> writer(os);
    mDoc.Accept(writer);

    fclose(fp);
}
