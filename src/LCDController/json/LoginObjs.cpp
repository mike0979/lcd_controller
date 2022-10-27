/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : LoginObjs.cpp
 * @author : Benson
 * @date : Oct 11, 2017
 * @brief :
 */

#include <json/LoginObjs.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stream.h>
#include <rapidjson/stringbuffer.h>

using namespace rapidjson;

namespace Json
{

std::string LoginReq::ToJson(const std::string& account, const std::string& pwd)
{
    Document doc;
    doc.SetObject();

    Document::AllocatorType &allocator = doc.GetAllocator();

    doc.AddMember("account", Value(account.c_str(), allocator), allocator);
    doc.AddMember("password", Value(pwd.c_str(), allocator), allocator);

    StringBuffer buffer;
    PrettyWriter<StringBuffer> pretty_writer(buffer);
    doc.Accept(pretty_writer);

    return buffer.GetString();
}

}

Json::LoginReply::LoginReply() :
        mToken(""), mExpireTime(600)
{
}

Json::LoginReply::~LoginReply()
{
}

bool Json::LoginReply::Parse(const char* jsonStr, LoginReply* data)
{
    if (NULL == jsonStr || NULL == data)
        return false;

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);

    // update temperature
    if(d.HasMember("token"))
    	data->mToken = d["token"].GetString();
    else
    	data->mToken = "";

    if(d.HasMember("expired_in"))
    	data->mExpireTime = d["expired_in"].GetInt();
    else
    	data->mExpireTime = -1;

    return true;
}
