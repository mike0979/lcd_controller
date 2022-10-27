/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : CommandObjs.cpp
 * @author : Benson
 * @date : Sep 18, 2017
 * @brief :
 */

#include "CommandObjs.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <exception>

using namespace rapidjson;

static bool ParseCmdBasic(const Value& jsonObj, Json::CmdBasic* data)
{
    if (NULL == data)
        return false;

    if( !jsonObj.IsObject() )
    {
        return false;
    }

    data->mId = jsonObj.GetObject()["id"].GetInt();
    data->mDscrp = jsonObj.GetObject()["description"].GetString();
    data->mCmd = jsonObj.GetObject()["command"].GetString();
    data->mCmdParm = jsonObj.GetObject()["command_param"].GetString();
    data->mStatus = jsonObj.GetObject()["status"].GetInt();
    data->mStartTime = jsonObj.GetObject()["start_time"].GetString();
    data->mUpdateTime = jsonObj.GetObject()["updated_time"].GetString();

    return true;
}

Json::CmdBasic::CmdBasic() :
        mId(-1), mDscrp(""), mCmd(""), mCmdParm(""), mStatus(-1), mStartTime(
                ""), mUpdateTime("")
{
}

Json::CmdBasic::~CmdBasic()
{
}

Json::CmdList::CmdList()
{
}

Json::CmdList::~CmdList()
{
}

bool Json::CmdList::Parse(const char* jsonStr, CmdList* data)
{
    if (NULL == jsonStr || NULL == data)
        return false;

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);

    data->mCmds.clear();

    if( !d.IsArray() )
    {
        return false;
    }

    // Get obj array
    Value arr = d.GetArray();

    CmdBasic basicObj;
    for (uint i = 0; i < arr.Size(); i++)
    {
        ParseCmdBasic(arr[i], &basicObj);

        data->mCmds.push_back(basicObj);
    }

    return true;
}

Json::CmdDetail::CmdDetail()
{
}

Json::CmdDetail::~CmdDetail()
{
}

bool Json::CmdDetail::Parse(const char* jsonStr, CmdDetail* data)
{
    if (NULL == jsonStr || NULL == data)
        return false;


	StringStream s(jsonStr);
	Document d;
	d.ParseStream(s);

	// Get basic info
	ParseCmdBasic(d,&(data->mBasic));

	// Get Object
	if(d.HasMember("specific_object"))
	{
		Value specArr = d["specific_object"].GetArray();
		for (uint i = 0; i < specArr.Size(); i++)
		{
			Value specObj = specArr[i].GetObject();
			if(specObj.HasMember("master"))
			{
				data->mSpecObj.mMaster = specObj["master"].GetString();
			}
			else
			{
				data->mSpecObj.mMaster = "";
			}

			if(specObj.HasMember("target"))
			{
				Value targetArr = specObj["target"].GetArray();

				for (uint i = 0; i < targetArr.Size(); i++)
				{
					data->mSpecObj.mTargets.push_back(targetArr[i].GetString());
				}
			}
			else
			{
				data->mSpecObj.mTargets.clear();
			}
		}
	}
	else
	{
		printf("No node specific_object!!!!!!!!!\n");
	}


    return true;
}

Json::CmdExeReply::CmdExeReply() :
        mId(-1),mDevice(""), mStatus(-1), mRepTime("")
{
}


Json::CmdExeReply::~CmdExeReply()
{
}

void Json::CmdExeReply::ToJson(const CmdExeReply* obj, std::string& jsonStr)
{
    if (NULL == obj)
        return;

    Document doc;
    doc.SetObject();

    Document::AllocatorType &allocator = doc.GetAllocator();

    doc.AddMember("device", Value(obj->mDevice.c_str(), allocator), allocator);
    doc.AddMember("status", obj->mStatus, allocator);
    doc.AddMember("rep_time", Value(obj->mRepTime.c_str(), allocator),
            allocator);

    StringBuffer buffer;
    PrettyWriter<StringBuffer> pretty_writer(buffer);
    doc.Accept(pretty_writer);

    jsonStr = buffer.GetString();

    return;
}

