/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : LayoutGroupHandler.cpp
 * @author : Benson
 * @date : Oct 30, 2017
 * @brief :
 */

#include <json/LayoutGroupFileHandler.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stream.h>
#include <cstdio>
#include <iterator>
#include <utility>

static const int BufferSize = 1024 * 40;

using namespace rapidjson;

Json::LayoutGroupFileHandler::LayoutGroupFileHandler()
{
}

Json::LayoutGroupFileHandler::LayoutGroupFileHandler(const std::string& fileName) :
        LayoutGroupFileBase(fileName)
{
}

Json::LayoutGroupFileHandler::~LayoutGroupFileHandler()
{
    for (unsigned i = 0; i < mDocs.size(); i++)
    {
        if (mDocs[i])
            delete mDocs[i];
    }

    mDocs.clear();
}

void Json::LayoutGroupFileHandler::SetFileName(const std::string& fileName)
{
    mFileName = fileName;
}

bool Json::LayoutGroupFileHandler::Find(int id, LayoutGroupDetail* data)
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
            return GetLayoutGroupDetail(obj, data);
        }
    }

    return false;
}

bool Json::LayoutGroupFileHandler::Delete(int id)
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

bool Json::LayoutGroupFileHandler::UpdateLayoutGroupDetail(const char* jsonStr,
        LayoutGroupDetail* data)
{
    if (jsonStr == NULL || data == NULL)
        return false;

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);

    Document* copyDoc = new Document();
    copyDoc->CopyFrom(d, copyDoc->GetAllocator());

    mDocs.push_back(copyDoc);

    Value obj = d.GetObject();
    return GetLayoutGroupDetail(obj, data);
}

bool Json::LayoutGroupFileHandler::LoadLayoutGroupDetail(
        const std::string& fileName,
        std::map<int, LayoutGroupDetail>& allGroupDetail)
{
    FILE* fp = fopen(fileName.c_str(), "r");
    if (NULL == fp)
        return false;

    char readBuffer[BufferSize];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    Document d;
    d.ParseStream(is);

    fclose(fp);

    if (!d.IsArray())
        return false;

    Value array = d.GetArray();
    for (unsigned i = 0; i < array.Size(); i++)
    {
        LayoutGroupDetail detail;
        Value obj = array[i].GetObject();
        GetLayoutGroupDetail(obj, &detail);

        allGroupDetail.insert(std::make_pair(detail.mId, detail));
    }

    return true;
}

bool Json::LayoutGroupFileHandler::LoadLayoutGroupDetail(
        const std::string& fileName,
        std::map<int, LayoutGroupDetail*>& allGroupDetail)
{
    FILE* fp = fopen(fileName.c_str(), "r");
    if (NULL == fp)
        return false;

    char readBuffer[BufferSize];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    Document d;
    d.ParseStream(is);

    fclose(fp);

    if (!d.IsArray())
        return false;

    Value array = d.GetArray();
    for (unsigned i = 0; i < array.Size(); i++)
    {
        LayoutGroupDetail *detail = new LayoutGroupDetail();
        Value obj = array[i].GetObject();
        GetLayoutGroupDetail(obj, detail);
        allGroupDetail.insert(std::make_pair(detail->mId, detail));
    }

    return true;
}

void Json::LayoutGroupFileHandler::Save()
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

    char writeBuffer[BufferSize] =
    { 0 };
    FileWriteStream os(fp, writeBuffer, BufferSize);
    PrettyWriter<FileWriteStream> writer(os);
    all.Accept(writer);

    fclose(fp);

    mDocs.clear();
}

bool Json::LayoutGroupFileHandler::GetLayoutGroupDetail(rapidjson::Value& obj,
        LayoutGroupDetail* data)
{
    if (data == NULL)
        return false;

    data->mId = obj["id"].GetInt();
    data->mDscrp = obj["description"].GetString();
    data->mResolution = obj["resolution"].GetString();

    // parse layout array
    Value layoutArray = obj["layouts"].GetArray();
    LayoutDetail layout;
    for (unsigned i = 0; i < layoutArray.Size(); ++i)
    {
    	layout.mPartitions.clear();

        layout.mId = layoutArray[i]["id"].GetInt();
        layout.mName = layoutArray[i]["name"].GetString();
        layout.mDescription = layoutArray[i]["description"].GetString();
        layout.mUpdateTime = layoutArray[i]["update_time"].GetString();

        // parse partition array
        Value ptArray = layoutArray[i]["partition"].GetArray();
        PartitionDetail partition;
        for (unsigned j = 0; j < ptArray.Size(); ++j)
        {
            partition.mId = ptArray[j]["id"].GetInt();
            partition.mMediaType = ptArray[j]["media_type"].GetInt();
            partition.mIsSoundable = ptArray[j]["sound"].GetBool();
            partition.mXpos = ptArray[j]["x_pos"].GetInt();
            partition.mYpos = ptArray[j]["y_pos"].GetInt();
            partition.mWidth = ptArray[j]["width"].GetInt();
            partition.mHeight = ptArray[j]["height"].GetInt();
            partition.mZorder = ptArray[j]["z_order"].GetInt();
            partition.mIsTransparent = ptArray[j]["transparent"].GetBool();
            partition.mIsMaster = ptArray[j]["master"].GetBool();
            partition.mBkgroudFile = ptArray[j]["background"].GetString();
            partition.mOPSflag = ptArray[j]["ops_flag"].GetBool();
            partition.mUpdateTime = ptArray[j]["update_time"].GetString();

            layout.mPartitions.push_back(partition);
        }// end of for(ptArray)
        data->mLayoutDetails.push_back(layout);
    }// end of for(layoutArray)

    return true;
}
