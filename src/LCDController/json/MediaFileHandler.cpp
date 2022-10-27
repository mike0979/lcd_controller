/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : MediaHandler.cpp
 * @author : Benson
 * @date : Sep 7, 2017
 * @brief :
 */

#include <json/MediaFileHandler.h>
#include "rapidjson/reader.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include <stdio.h>

static const int WriteBufSize = 1024 * 40;

using namespace rapidjson;

Json::MediaFileHandler::MediaFileHandler()
{
}

Json::MediaFileHandler::MediaFileHandler(const std::string& fileName) :
        MediaFileBase(fileName)
{
}

Json::MediaFileHandler::~MediaFileHandler()
{
    for (unsigned i = 0; i < mDocs.size(); i++)
    {
        if (mDocs[i])
            delete mDocs[i];
    }

    mDocs.clear();
}

void Json::MediaFileHandler::SetFileName(const std::string& fileName)
{
    mFileName = fileName;
}

bool Json::MediaFileHandler::UpdateMediaDetail(const char* jsonStr,
        MediaBasic::MediaType& type, MediaBasic** ppData)
{
    if (NULL == jsonStr || NULL == ppData)
        return false;

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);

    Document* copyDoc = new Document();
    copyDoc->CopyFrom(d, copyDoc->GetAllocator());
    mDocs.push_back(copyDoc);

    if( !d.IsObject() )
    {
        return false;
    }

    Value obj = d.GetObject();

    return GetMediaDetail(obj, type, ppData);
}

bool Json::MediaFileHandler::LoadMediaDetail(const std::string& fileName,
        std::map<int, MediaBasic*>& allMedias)
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
    MediaBasic* tempVal = NULL;
    MediaBasic::MediaType tempType = MediaBasic::MediaType::MediaTypeUnknown;

    for (unsigned i = 0; i < array.Size(); i++)
    {
        Value obj = array[i].GetObject();

        GetMediaDetail(obj, tempType, &tempVal);
        allMedias.insert(std::make_pair(tempVal->mId, tempVal));
    }

    return true;
}

bool Json::MediaFileHandler::Find(int id, MediaBasic::MediaType& type,
        MediaBasic** data)
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
            return GetMediaDetail(obj, type, data);
        }
    }

    return false;
}

bool Json::MediaFileHandler::Delete(int id)
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

void Json::MediaFileHandler::Save()
{
    FILE* fp = fopen(mFileName.c_str(), "w+");
    if (fp == NULL)
        return;

    Document all;
    all.SetArray();

    for (unsigned i = 0; i < mDocs.size(); i++)
    {
        Value copy(*(mDocs[i]),all.GetAllocator());
//        all.PushBack(*(mDocs[i]), all.GetAllocator());
        all.PushBack(copy,all.GetAllocator());    }

    char writeBuffer[WriteBufSize];
    FileWriteStream os(fp, writeBuffer, WriteBufSize);
    PrettyWriter<FileWriteStream> writer(os);
    all.Accept(writer);

    fclose(fp);

    mDocs.clear();
}

bool Json::MediaFileHandler::GetMediaDetail(rapidjson::Value& obj,
        MediaBasic::MediaType& type, MediaBasic** ppData)
{
    if (ppData == NULL || !obj.IsObject())
        return false;

    int ret = true;

    if(!obj.IsObject())
    {
        return false;
    }

    // get the basic information
    //int val = obj["media_type"].GetInt();
    int val = obj["type"].GetInt();

    Value paramsObj = obj["params"].GetObject();

    type = (MediaBasic::MediaType) val;
    switch (type)
    {
    case MediaBasic::MediaType::Text:
    {
        MediaText* text = new MediaText();
        if (NULL == text)
            return false;

        if(paramsObj.HasMember("text"))
        {
			Value textObj = paramsObj["text"].GetObject();
			ret = GetTextInfo(textObj, &text->mParams);
        }
        *ppData = text;

        break;
    }
    case MediaBasic::MediaType::ArrivalMsg:
    case MediaBasic::MediaType::DigitalClock:
    case MediaBasic::MediaType::Weather:
    {
        MediaCommon1* mediaCommon1 = new MediaCommon1();
        if (NULL == mediaCommon1)
            return false;

        // get labels
        if(paramsObj.HasMember("labels"))
        {
        	Value labelArr = paramsObj["labels"].GetArray();

			for (unsigned i = 0; i < labelArr.Size(); i++)
			{
				LabelInfo lblInfo;
				GetLabelInfo(labelArr[i], &lblInfo);
				mediaCommon1->mParams.push_back(lblInfo);
			}
        }

        *ppData = mediaCommon1;
        break;
    }
    case MediaBasic::MediaType::Image:
    case MediaBasic::MediaType::Video:
    case MediaBasic::MediaType::AnalogClock:
    case MediaBasic::MediaType::Flash:
    {
        MediaCommon2* mediaCommon2 = new MediaCommon2();
        if (NULL == mediaCommon2)
            return false;

        if(paramsObj.HasMember("file"))
        	mediaCommon2->mFile = paramsObj["file"].GetString();
        else
        	mediaCommon2->mFile = "";

        *ppData = mediaCommon2;
        break;
    }
    case MediaBasic::MediaType::Live:
    {
        MediaCommon2* mediaCommon2 = new MediaCommon2();
        if (NULL == mediaCommon2)
            return false;

        if(paramsObj.HasMember("url"))
        	mediaCommon2->mUrl = paramsObj["url"].GetString();
        else
        	mediaCommon2->mUrl = "";

        if(paramsObj.HasMember("file"))
        	mediaCommon2->mFile = paramsObj["file"].GetString();
        else
        	mediaCommon2->mFile = "";

        *ppData = mediaCommon2;
        break;
    }
    default:
        type = MediaBasic::MediaType::MediaTypeUnknown;
        *ppData = NULL;
        return false;
    }

    // get the basic info.
    (*ppData)->mId = obj["id"].GetInt();
    (*ppData)->mType = type;
    (*ppData)->mUpdateTime = obj["updated_time"].GetString();
    (*ppData)->mDuration = obj["duration"].GetFloat();

    return ret;
}
