/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : ScheduleHandler.cpp
 * @author : Benson
 * @date : Sep 6, 2017
 * @brief :
 */

#include <json/ScheduleFileHandler.h>
#include <json/ScheduleObjs.h>
#include <Log.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stream.h>
#include <stdio.h>
#include <iterator>
#include <string>
#include <utility>

static const int WriteBufSize = 1024 * 40;
const char * Json::ScheduleFileHandler::TAG = "ScheduleFileHandler";

using namespace rapidjson;

Json::ScheduleFileHandler::ScheduleFileHandler()
{
}

Json::ScheduleFileHandler::ScheduleFileHandler(const std::string& fileName) :
        ScheduleFileBase(fileName)
{
}

Json::ScheduleFileHandler::~ScheduleFileHandler()
{
    for (unsigned i = 0; i < mDocs.size(); i++)
    {
        if (mDocs[i])
            delete mDocs[i];
    }

    mDocs.clear();
}

void Json::ScheduleFileHandler::SetFileName(const std::string& fileName)
{
    mFileName = fileName;
}

bool Json::ScheduleFileHandler::UpdateScheduleDetail(const char* jsonStr,
        ScheduleDetail* data)
{
    if (jsonStr == NULL || data == NULL)
        return false;

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);

    Document* copyDoc = new Document();
    copyDoc->CopyFrom(d, copyDoc->GetAllocator());
    mDocs.push_back(copyDoc);

    if(!d.IsObject())
    {
        LogE("Json parse error!");
        return false;
    }

    Value obj = d.GetObject();
    return GetScheduleDetail(obj, data);
}

bool Json::ScheduleFileHandler::LoadScheduleDetail(const std::string& fileName,
        std::map<int, ScheduleDetail>& allSchDetail)
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
        LogE("Json parse error!");
        return false;
    }

    Value array = d.GetArray();
    for (unsigned i = 0; i < array.Size(); i++)
    {
        ScheduleDetail detail;
        Value obj = array[i].GetObject();
        GetScheduleDetail(obj, &detail);

        allSchDetail.insert(std::make_pair(detail.mScheduleBasic.mId, detail));
    }

    return true;
}

bool Json::ScheduleFileHandler::LoadScheduleDetail(const std::string& fileName,
        std::map<int, ScheduleDetail*>& allSchDetail)
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
        LogE("Json parse error!");
        return false;
    }

    Value array = d.GetArray();
    for (unsigned i = 0; i < array.Size(); i++)
    {
        ScheduleDetail* detail = new ScheduleDetail();
        Value obj = array[i].GetObject();
        GetScheduleDetail(obj, detail);
        allSchDetail.insert(std::make_pair(detail->mScheduleBasic.mId, detail));
    }

    return true;
}

void Json::ScheduleFileHandler::Save()
{
    FILE* fp = fopen(mFileName.c_str(), "w+");
    if (fp == NULL)
        return;

    Document all;
    all.SetArray();

    for (unsigned i = 0; i < mDocs.size(); i++)
    {
        Value copy(*(mDocs[i]), all.GetAllocator());
//        all.PushBack(*(mDocs[i]), all.GetAllocator());
        all.PushBack(copy, all.GetAllocator());
    }

    char writeBuffer[WriteBufSize];
    FileWriteStream os(fp, writeBuffer, WriteBufSize);
    PrettyWriter<FileWriteStream> writer(os);
    all.Accept(writer);

    fclose(fp);

    LogD("\t\t ========== Json::ScheduleFileHandler::Save() ==========\n");
    mDocs.clear();
}

bool Json::ScheduleFileHandler::Find(int id, ScheduleDetail* data)
{
    if (NULL == data)
        return false;

    Document theDocCopy;
    for (unsigned i = 0; i < mDocs.size(); i++)
    {
        theDocCopy.CopyFrom(*(mDocs[i]), theDocCopy.GetAllocator());

        if (id == theDocCopy["id"].GetInt())
        {
            Value obj = theDocCopy.GetObject();
            return GetScheduleDetail(obj, data);
        }
    }

    return false;
}

bool Json::ScheduleFileHandler::Delete(int id)
{
    std::vector<Document*>::iterator itor;

    for (itor = mDocs.begin(); itor != mDocs.end(); itor++)
    {
        if (*itor != NULL && id == (**itor)["id"].GetInt())
        {
            delete (*itor);
            mDocs.erase(itor);

            return true;
        }
    }

    return false;
}

bool Json::ScheduleFileHandler::GetScheduleDetail(Value& obj, ScheduleDetail* data)
{
    if(NULL == data)
        return false;

    // Get schedule basic.
    if(obj.HasMember("id"))
    	data->mScheduleBasic.mId = obj["id"].GetInt();

    if(obj.HasMember("description"))
    	data->mScheduleBasic.mDescrp = obj["description"].GetString();

    if(obj.HasMember("start_time"))
    	data->mScheduleBasic.mStartTime = obj["start_time"].GetString();

    if(obj.HasMember("end_time"))
    	data->mScheduleBasic.mEndTime = obj["end_time"].GetString();

    if(obj.HasMember("publish_time"))
    	data->mScheduleBasic.mPublishTime = obj["publish_time"].GetString();

    if(obj.HasMember("update_time"))
    	data->mScheduleBasic.mUpdateTime = obj["update_time"].GetString();

    if(obj.HasMember("server_level"))
    	data->mScheduleBasic.mServerLevel = obj["server_level"].GetInt();

    if(obj.HasMember("priority"))
    	data->mScheduleBasic.mPriority = obj["priority"].GetInt();

    // Get layout groups.
    SchLayoutGroupBasic layoutGroupBsc;

    if(obj.HasMember("layout_group"))
    {
    	Value groups = obj["layout_group"].GetArray();
		for(unsigned i=0; i<groups.Size(); ++i )
		{
			layoutGroupBsc.mId = groups[i]["id"].GetInt();
			layoutGroupBsc.mDscrp = groups[i]["description"].GetString();
			layoutGroupBsc.mStartTime = groups[i]["start_time"].GetString();
			layoutGroupBsc.mEndTime = groups[i]["end_time"].GetString();
			layoutGroupBsc.mSwitchTime = groups[i]["switch_time"].GetInt();
			layoutGroupBsc.mUpdatedTime = groups[i]["update_time"].GetString();

			// Get partition medias.
			layoutGroupBsc.mPartionMedias.clear();
			Value partitionMedias = groups[i]["partition_medias"].GetArray();
			for(unsigned j=0; j< partitionMedias.Size(); ++j)
			{
				PartitionMedias partiton;
				partiton.mPartitionId= partitionMedias[j]["partition_id"].GetInt();

				// Get medias.
				Value medias = partitionMedias[j]["medias"].GetArray();
				partiton.mMediaIds.clear();
				for( unsigned k=0; k<medias.Size(); ++k )
				{
					int mediaId = medias[k].GetInt();
					partiton.mMediaIds.push_back(mediaId);
				}// end of for(medias).

				layoutGroupBsc.mPartionMedias.push_back(partiton);
			}// end of for(partitionMedias)
			data->mLayoutGroups.push_back(layoutGroupBsc);
		}// end of for(groups)

    }

    return true;
}

