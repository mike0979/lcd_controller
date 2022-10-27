/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : ScheduleObjs.cpp
 * @author : Benson
 * @date : Aug 17, 2017
 * @brief : JsonParser used for parse Json string to certain data class.
 */
#include "ScheduleObjs.h"
#include "Log.h"

#include "rapidjson/reader.h"
#include "rapidjson/prettywriter.h"
#include <algorithm>
#include <stdio.h>
#include <utility>
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "bj_pis/utils/datetime.h"
#include "bj_pis/convent/hd_opm.h"

using namespace rapidjson;

const char* Json::OPSMsgList::TAG = "OPSMsgList";
const char* Json::OPSMsgDetail::TAG = "OPSMsgDetail";
const char* Json::ArrivalDetail::TAG = "ArrivalDetail";
const char* Json::WeatherDetail::TAG = "WeatherDetail";
const char* Json::TrainTimeDetail::TAG = "TrainTimeDetail";
const char* Json::ScreenOnOffDetail::TAG = "ScreenOnOffDetail";
const char* Json::LiveSwitchDetail::TAG = "LiveSwitchDetail";

Json::ScheduleBasic::ScheduleBasic() :
        mId(-1), mUpdateTime(""), mName(""), mDescrp(""), mStartTime(""), mEndTime(
                ""),mServerLevel(0),mPriority(0)
{
}

Json::ScheduleBasic::~ScheduleBasic()
{
}

Json::ScheduleList::ScheduleList()
{
}

Json::ScheduleList::~ScheduleList()
{
}

Json::ScheduleDetail::ScheduleDetail() :
        mPriority(-1)
{
}

Json::ScheduleDetail::~ScheduleDetail()
{
}

Json::PartitionDetail::PartitionDetail() :
        mId(-1), mMediaType(0), mIsSoundable(false), mXpos(0), mYpos(0), mWidth(
                0), mHeight(0), mZorder(0), mIsTransparent(false), mIsMaster(
                false), mBkgroudFile(""), mOPSflag(false), mUpdateTime("")
{
}

Json::PartitionDetail::~PartitionDetail()
{
}

Json::LayoutDetail::LayoutDetail() :
        mId(-1), mName(""), mDescription(""), mUpdateTime("")
{
}

Json::LayoutDetail::~LayoutDetail()
{
}

void Json::LayoutDetail::sortPartationByZOrder()
{
	std::sort(mPartitions.begin(), mPartitions.end(), cmp_by_value);
}

Json::MediaBasic::MediaBasic() :
        mId(-1), mType(MediaTypeUnknown), mUpdateTime(""), mDuration(-1)
{
}

Json::MediaBasic::~MediaBasic()
{
}

Json::FontInfo::FontInfo() :
        mName(""), mSize(-1), mIsBold(false), mIsItalic(false)
{
}

Json::FontInfo::~FontInfo()
{
}

Json::TextInfo::TextInfo() :
        mContent(""), mForeColor(""), mBackColor(""), mBackImage(""), mEffect(
                Static), mSpeed(Normal), mAlign(Center)
{
}

Json::TextInfo::~TextInfo()
{
}

Json::MediaText::MediaText()
{
}

Json::MediaText::~MediaText()
{
}

Json::VaribleInfo::VaribleInfo() :
        mTypeStr(""), mTrainIndex(-1), mTimeFormat(""), mUseIcon(false)
{
}

Json::VaribleInfo::~VaribleInfo()
{
}

Json::VarTextInfo::VarTextInfo()
{
}

Json::VarTextInfo::~VarTextInfo()
{
}

Json::LabelInfo::LabelInfo()
{
}

Json::LabelInfo::~LabelInfo()
{
}

Json::MediaCommon1::MediaCommon1()
{
}

Json::MediaCommon1::~MediaCommon1()
{
}

Json::MediaCommon2::MediaCommon2()
{
}

Json::MediaCommon2::~MediaCommon2()
{
}

Json::OPSMsgBasic::OPSMsgBasic() :
        mId(-1), mLevelId(-1), mContent(""), mStatus(-1), mStartTime(""), mEndTime(
                ""), mPlayCnt(-1), mCreateTime(""), mUpdateTime("")
{
}

Json::OPSMsgBasic::~OPSMsgBasic()
{
}

Json::OPSMsgList::OPSMsgList()
{
}

Json::OPSMsgList::~OPSMsgList()
{
}

bool Json::OPSMsgList::Parse(const char* jsonStr, OPSMsgList* data)
{
    if (NULL == jsonStr || NULL == data)
        return false;

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);
    data->mOPSMsgs.clear();

    if(!d.IsArray())
    {
        LogE("Json parse error!");
        return false;
    }

    Value arr = d.GetArray();
    OPSMsgBasic msgBsc;
    for (unsigned i = 0; i < arr.Size(); i++)
    {
        msgBsc.mId = arr[i]["id"].GetInt();
        msgBsc.mLevelId = arr[i]["level_id"].GetInt();
        msgBsc.mLevelName = arr[i]["level_name"].GetString();
        msgBsc.mContent = arr[i]["content"].GetString();
        msgBsc.mStatus = arr[i]["status"].GetInt();
        msgBsc.mStartTime = arr[i]["start_time"].GetString();
        msgBsc.mEndTime = arr[i]["end_time"].GetString();
        msgBsc.mPlayCnt = arr[i]["play_count"].GetInt();
        msgBsc.mCreateTime = arr[i]["create_time"].GetString();
        msgBsc.mUpdateTime = arr[i]["updated_time"].GetString();
        data->mOPSMsgs.push_back(msgBsc);
    }

    return true;
}

Json::OPSMsgDetail::OPSMsgDetail() :
        mPriority(0), mPlayMode(0), mDisplayRegion(0), mContent("")
{
}

Json::OPSMsgDetail::~OPSMsgDetail()
{
}

bool Json::OPSMsgDetail::Parse(const char* jsonStr, OPSMsgDetail* data)
{
    if (NULL == jsonStr || NULL == data)
        return false;

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);

    // Get obj array
//    Value arr = d["obj"].GetArray();
//    LogD("######################  size-%d\n",arr.Size());
//    for (uint i = 0; i < arr.Size(); i++)
//    {
//        data->mObjs.push_back(arr[i].GetString());
//    }
    if(!d.IsObject())
    {
        LogE("Json parse error!");
        return false;
    }

    data->mBasic.mId = d["id"].GetInt();
    data->mBasic.mLevelId = d["level_id"].GetInt();
    data->mBasic.mStatus = d["status"].GetInt();
    data->mBasic.mCreateTime = d["create_time"].GetString();
    data->mBasic.mUpdateTime = d["updated_time"].GetString();

    /*if(d.HasMember("priority"))
     data->mPriority = d["priority"].GetInt();*/
    data->mPriority = d["priority"].GetInt();

    data->mBasic.mStartTime = d["start_time"].GetString();
    data->mBasic.mEndTime = d["end_time"].GetString();
    data->mBasic.mPlayCnt = d["play_count"].GetInt();
    //data->mQueueFlag = d["queue_flag"].GetInt();
   // data->mPlayMode = d["play_mode"].GetInt();
    data->mPlayMode = 2;
    data->mDisplayRegion = d["display_region"].GetInt();
    if(data->mDisplayRegion == 3)
    	data->mDisplayRegion = 2;

    data->mContent = d["content"].GetString();

    Value objText = d["text"].GetObject();

    // get text info.
    bool ret = GetTextInfo(objText, &(data->mText));
    if (true == ret)
        data->mBasic.mContent = data->mText.mContent;

    return ret;
}


void Json::OPSMsgDetail::Parse(const OpmMsg& msg, OPSMsgDetail* data, int opm_id)
{
    data->mBasic.mId = opm_id;
    data->mBasic.mStatus = 0;
    data->mBasic.mLevelId = 1/* + GetOffset()*/;
    data->mPriority = msg.priority;
    data->mBasic.mPlayCnt = 0;
	if (data->mPriority >= 8 && data->mPriority <= 10)
	{
        data->mDisplayRegion = 0;
	}
	else if (data->mPriority >= 4 && data->mPriority <= 7)
	{
		data->mDisplayRegion = 2;
	}
	else
	{
		data->mDisplayRegion = 1;
	}
	data->mPlayMode = 2;
	data->mBasic.mStartTime = datetime::from_time(msg.start_time).ToString("%Y%m%d %H%M%S");
	data->mBasic.mEndTime = datetime::from_time(msg.end_time).ToString("%Y%m%d %H%M%S");
	data->mContent = msg.info_text;
	if(data->mPriority==6)
	{
		data->mText.mBackImage = msg.info_text;
	}
}

bool Json::OPSMsgDetail::PriorityHigherThan(const OPSMsgDetail& b) const
{
    return (this->mPriority < b.mPriority);
}

bool Json::OPSMsgDetail::PriorityEqual(const OPSMsgDetail& b) const
{
    return (this->mPriority == b.mPriority);
}

bool Json::OPSMsgDetail::PriorityLowerThan(const OPSMsgDetail& b) const
{
    return (this->mPriority > b.mPriority);
}

Json::TrainInfo::TrainInfo() :
        mTrainCode(""), mCarCount(0), mHoldFlag(false), mSkipFlag(false), mIsFirstTrain(
                true), mIsLastTrain(false), mArrivalStatus(0), mArrivalTime(0), mDepartureTime(
                ""), mTerminal("")
{
}

Json::TrainInfo::~TrainInfo()
{
}

Json::ArrivalDetail::ArrivalDetail()
{
}

Json::ArrivalDetail::~ArrivalDetail()
{
}

bool Json::ArrivalDetail::Parse(const char* jsonStr, ArrivalDetail* data)
{
    if (NULL == jsonStr || NULL == data)
        return false;

    data->mTrains.clear();

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);

    if(d.HasMember("station_code"))
    	data->mStationCode = d["station_code"].GetString();
    else
    	data->mStationCode = "";

    if(d.HasMember("platform_code"))
    	data->mPlatformCode = d["platform_code"].GetString();
    else
    	data->mPlatformCode = "";

    if(d.HasMember("trains"))
    {
        TrainInfo info;
        Value trainArr = d["trains"].GetArray();

        for (unsigned i = 0; i < trainArr.Size(); i++)
        {
            info.mTrainCode = trainArr[i]["train_code"].GetString();
            info.mCarCount = trainArr[i]["car_count"].GetInt();
            info.mHoldFlag = trainArr[i]["hold_flag"].GetBool();
            info.mSkipFlag = trainArr[i]["skip_flag"].GetBool();
            info.mIsFirstTrain = trainArr[i]["first_train"].GetBool();
            info.mIsLastTrain = trainArr[i]["last_train"].GetBool();
            info.mArrivalStatus = trainArr[i]["arrival_status"].GetInt();
            info.mArrivalTime = trainArr[i]["arrival_time"].GetInt();
            //info.mDepartureTime = trainArr[i]["departure_time"].GetString();
            info.mTerminal = trainArr[i]["terminal"].GetString();

            data->mTrains.push_back(info);
        }
    }

    return true;
}

Json::TrainTime::TrainTime() :
		mDirectionCode(""),mFirstTrainTime(""),mLastTrainTime(""),mUpDown("")
{
}

Json::TrainTime::~TrainTime()
{
}

Json::TrainTimeDetail::TrainTimeDetail()
{

}

Json::TrainTimeDetail::~TrainTimeDetail()
{

}

bool Json::TrainTimeDetail::Parse(const char* jsonStr, TrainTimeDetail* data)
{
    if (NULL == jsonStr || NULL == data)
        return false;

    data->mTrainsTime.clear();

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);

    if(!d.IsArray())
    {
        LogE("Json parse error!");
        return false;
    }

    Value array = d.GetArray();
    for (unsigned i = 0; i < array.Size(); ++i)
	{
		const Value& obj = array[i];

		if(obj.HasMember("station_code"))
			data->mStationCode = obj["station_code"].GetString();
		else
			data->mStationCode = "";

		if(obj.HasMember("update_time"))
			data->mUpdateTime = obj["update_time"].GetString();
		else
			data->mUpdateTime = "";

		if(obj.HasMember("up"))
		{
			TrainTime traintime;

			if(obj["up"].HasMember("first_train"))
				traintime.mFirstTrainTime = obj["up"]["first_train"].GetString();
			else
				traintime.mFirstTrainTime = "";

			if(obj["up"].HasMember("last_train"))
				traintime.mLastTrainTime = obj["up"]["last_train"].GetString();
			else
				traintime.mLastTrainTime = "";

			if(obj["up"].HasMember("direction"))
				traintime.mDirectionCode = obj["up"]["direction"].GetString();
			else
				traintime.mDirectionCode = "";

			traintime.mUpDown = "up";

			data->mTrainsTime.push_back(traintime);
		}

		if(obj.HasMember("down"))
		{
			TrainTime traintime;

			if(obj["down"].HasMember("first_train"))
				traintime.mFirstTrainTime = obj["down"]["first_train"].GetString();
			else
				traintime.mFirstTrainTime = "";

			if(obj["down"].HasMember("last_train"))
				traintime.mLastTrainTime = obj["down"]["last_train"].GetString();
			else
				traintime.mLastTrainTime = "";

			if(obj["down"].HasMember("direction"))
				traintime.mDirectionCode = obj["down"]["direction"].GetString();
			else
				traintime.mDirectionCode = "";

			traintime.mUpDown = "down";

			data->mTrainsTime.push_back(traintime);
		}
	}
    return true;
}

bool Json::TrainTimeDetail::Load(const std::string file, TrainTimeDetail* data)
{
    FILE* fp = fopen(file.c_str(), "r");
    if (NULL == fp)
        return false;

    char readBuffer[1024];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    fclose(fp);

    return Parse(readBuffer,data);
}

bool Json::TrainTimeDetail::Save(const std::string file, std::string sdata)
{
    FILE* fp = fopen(file.c_str(), "w+");
    if (fp == NULL)
        return false;

    StringStream s(sdata.c_str());
    Document d;
    d.ParseStream(s);

    char writeBuffer[1024];
    FileWriteStream os(fp, writeBuffer, 1024);
    PrettyWriter<FileWriteStream> writer(os);
    d.Accept(writer);

    fclose(fp);

	return true;
}

Json::ScreenOnOffTime::ScreenOnOffTime() :
		mStationCode(""),mUpdateTime(""),mScreenOnTime(""),mScreenOffTime("")
{
}

Json::ScreenOnOffTime::~ScreenOnOffTime()
{
}

Json::ScreenOnOffDetail::ScreenOnOffDetail()
{

}

Json::ScreenOnOffDetail::~ScreenOnOffDetail()
{

}

bool Json::ScreenOnOffDetail::Parse(const char* jsonStr, ScreenOnOffDetail* data)
{
    if (NULL == jsonStr || NULL == data)
        return false;

    data->mScreenOnOff.clear();

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);

    if(!d.IsArray())
    {
        LogE("Json parse error!");
        return false;
    }

    Value array = d.GetArray();
    for (unsigned i = 0; i < array.Size(); ++i)
	{
		const Value& obj = array[i];

		ScreenOnOffTime onofftime;

		if(obj.HasMember("station_code"))
			onofftime.mStationCode = obj["station_code"].GetString();
		else
			onofftime.mStationCode = "";

		if(obj.HasMember("update_time"))
			onofftime.mUpdateTime = obj["update_time"].GetString();
		else
			onofftime.mUpdateTime = "";

		if(obj.HasMember("screen_on"))
			onofftime.mScreenOnTime = obj["screen_on"].GetString();
		else
			onofftime.mScreenOnTime = "";

		if(obj.HasMember("screen_off"))
			onofftime.mScreenOffTime = obj["screen_off"].GetString();
		else
			onofftime.mScreenOffTime = "";

		data->mScreenOnOff.push_back(onofftime);
	}
    return true;
}

Json::LiveSourceSwitch::LiveSourceSwitch() :
		mStationCode(""),mUpdateTime(""),mPlaySource(LiveSourceSwitchConfig::LIVE_AutoPlay)
{
}

Json::LiveSourceSwitch::~LiveSourceSwitch()
{
}

Json::LiveSwitchDetail::LiveSwitchDetail()
{

}

Json::LiveSwitchDetail::~LiveSwitchDetail()
{

}

bool Json::LiveSwitchDetail::Parse(const char* jsonStr, LiveSwitchDetail* data)
{
    if (NULL == jsonStr || NULL == data)
        return false;

    data->mLiveSource.clear();

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);

    if(!d.IsArray())
    {
        LogE("Json parse error!");
        return false;
    }

    Value array = d.GetArray();
    for (unsigned i = 0; i < array.Size(); ++i)
	{
		const Value& obj = array[i];

		LiveSourceSwitch sourceswitch;

		if(obj.HasMember("station_code"))
			sourceswitch.mStationCode = obj["station_code"].GetString();
		else
			sourceswitch.mStationCode = "";

		if(obj.HasMember("update_time"))
			sourceswitch.mUpdateTime = obj["update_time"].GetString();
		else
			sourceswitch.mUpdateTime = "";

		if(obj.HasMember("play_source"))
			sourceswitch.mPlaySource = obj["play_source"].GetInt();
		else
			sourceswitch.mPlaySource = LiveSourceSwitchConfig::LIVE_AutoPlay;


		data->mLiveSource.push_back(sourceswitch);
	}
    return true;
}

Json::TemperatureInfo::TemperatureInfo() :
        mMax(-1), mMin(-1), mCurrent(-1)
{
}

Json::TemperatureInfo::~TemperatureInfo()
{
}

Json::WindInfo::WindInfo() :
        mDirection(0), mSpeed(0)
{
}

Json::WindInfo::~WindInfo()
{
}

Json::WeatherDetail::WeatherDetail() :
        mAtmosphere(0)
{
}

Json::WeatherDetail::~WeatherDetail()
{
}

bool Json::WeatherDetail::Parse(const char* jsonStr, WeatherDetail* data)
{
    if (NULL == jsonStr || NULL == data)
        return false;

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);

    if(!d.IsObject())
    {
        LogE("Json parse error!");
        return false;
    }

    // update temperature
    Value tmpObj = d["temperature"].GetObject();
    data->mTemperature.mMax = tmpObj["max"].GetInt();
    data->mTemperature.mMin = tmpObj["min"].GetInt();
    data->mTemperature.mCurrent = tmpObj["current"].GetInt();

    // update atmosphere
    data->mAtmosphere = d["atmosphere"].GetInt();

    // update wind
    Value windObj = d["wind"].GetObject();
    data->mWind.mDirection = windObj["direction"].GetInt();
    data->mWind.mSpeed = windObj["speed"].GetInt();

    return true;
}

bool Json::GetTextInfo(rapidjson::Value& obj, TextInfo* textInfo)
{
    if (!obj.IsObject() || textInfo == NULL)
        return false;

    if (obj.HasMember("content"))
        textInfo->mContent = obj["content"].GetString();

    textInfo->mFont.mName = obj["font"]["name"].GetString();
    textInfo->mFont.mSize = obj["font"]["size"].GetInt();
    textInfo->mFont.mIsBold = obj["font"]["bold"].GetBool();
    textInfo->mFont.mIsItalic = obj["font"]["italic"].GetBool();

    textInfo->mForeColor = obj["fore_color"].GetString();
    textInfo->mBackColor = obj["back_color"].GetString();
    textInfo->mBackImage = obj["back_image"].GetString();
    textInfo->mAlign = (TextInfo::Align) obj["align"].GetInt();
    textInfo->mEffect = (TextInfo::Effect) obj["effect"].GetInt();
    textInfo->mSpeed = (TextInfo::Speed) obj["speed"].GetInt();

    return true;
}

bool Json::GetVaribleInfo(rapidjson::Value& obj, VaribleInfo* vblInfo)
{
    if (!obj.IsObject() || vblInfo == NULL)
        return false;

    vblInfo->mTypeStr = obj["type"].GetString();

    if (obj.HasMember("train_index"))
        vblInfo->mTrainIndex = obj["train_index"].GetInt();
    else
        vblInfo->mTrainIndex = -1;

    if (obj.HasMember("time_format"))
        vblInfo->mTimeFormat = obj["time_format"].GetString();
    else
        vblInfo->mTimeFormat = "";

    if (obj.HasMember("use_icon"))
        vblInfo->mUseIcon = obj["use_icon"].GetBool();
    else
        vblInfo->mUseIcon = false;

    if (obj.HasMember("language"))
        vblInfo->mLanguage = obj["language"].GetString();
    else
        vblInfo->mLanguage = "CHS";

    if (obj.HasMember("direction"))
        vblInfo->mDirection = obj["direction"].GetString();
    else
        vblInfo->mDirection = "";

    return true;
}

bool Json::GetVarTextInfo(rapidjson::Value& obj, VarTextInfo* varTextInfo)
{
    if (!obj.IsObject() || varTextInfo == NULL)
        return false;

    Value varTextObj = obj["text"].GetObject();

    if (!GetTextInfo(varTextObj, &(varTextInfo->mText)))
        return false;

    Value varibleArr = obj["varibles"].GetArray();
    for (unsigned i = 0; i < varibleArr.Size(); i++)
    {
        VaribleInfo vblInfo;
        if (!GetVaribleInfo(varibleArr[i], &vblInfo))
            return false;

        varTextInfo->mVaribles.push_back(vblInfo);
    }

    return true;
}

bool Json::GetLabelInfo(rapidjson::Value& obj, LabelInfo* lblInfo)
{
    if (!obj.IsObject() || lblInfo == NULL)
        return false;

    lblInfo->mRect = obj["rect"].GetString();

    Value varTextObj = obj["var_text"].GetObject();
    return GetVarTextInfo(varTextObj, &(lblInfo->mVarText));
}

void Json::LabelInfo::GetRectInfo(int& x, int& y, int& w, int& h)
{
    sscanf(mRect.c_str(), "%d,%d,%d,%d", &x, &y, &w, &h);
}

bool Json::ParseNtCode(const char* jsonStr, NotifyMessageCode& code)
{
    if (NULL == jsonStr)
        return false;

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);

    if(!d.IsObject())
    {
        return false;
    }

    code = (NotifyMessageCode) d["code"].GetInt();

    return true;
}

Json::OPSReply::OPSReply() :
        mId(-1), mDevice(""), mStatus(-1), mRepTime("")
{
}

Json::OPSReply::~OPSReply()
{
}

void Json::OPSReply::ToJson(const OPSReply* obj, std::string& jsonStr)
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

Json::PartitionMedias::PartitionMedias() :
        mPartitionId(-1)
{
}

Json::PartitionMedias::~PartitionMedias()
{
}

Json::SchLayoutGroupBasic::SchLayoutGroupBasic() :
        mId(-1), mDscrp(""), mStartTime(""), mEndTime(""), mSwitchTime(0), mUpdatedTime(
                "")
{
}

Json::SchLayoutGroupBasic::~SchLayoutGroupBasic()
{
}

Json::LayoutGroupDetail::LayoutGroupDetail() :
        mId(-1), mDscrp(""), mResolution("")
{
}

Json::LayoutGroupDetail::~LayoutGroupDetail()
{
}

Json::FileInfo::FileInfo() :
        mFilePath(""), mMD5("")
{
}

Json::FileInfo::~FileInfo()
{
}

bool Json::FileInfo::Parse(const char* jsonStr, FileInfo* data)
{
    if (NULL == jsonStr || NULL == data)
        return false;

    StringStream s(jsonStr);
    Document d;
    d.ParseStream(s);

    if(!d.IsObject())
    {
        return false;
    }

    data->mName = d["name"].GetString();
    data->mFilePath = d["path"].GetString();
    data->mMD5 = d["md5"].GetString();

    return true;
}
