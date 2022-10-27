#include "hd_layout.h"
#include "tcp_msg_help.h"
#include "bj_layout_info.h"
#include "t_command.h"
#include <stdexcept>
#include "Log.h"
#include "transmanage/TransHandlerFactory.h"
#include "transmanage/ScheduleHandler.h"
#include "json/ScheduleObjs.h"
#include "LCDController.h"
#include "bj_pis/utils/datetime.h"
#include "bj_pis/utils/string_ext.h"

//一个cpp里只能写一次，这个会在程序开始时运行。比main函数还早
STATIC_REGISTRATION(hd_layout)
{
  bj_msg::GetHandlers()["M51"] = &hd_layout::LayoutListSend;
  bj_msg::GetHandlers()["M45"] = &hd_layout::LayoutControlSend;
}

map<string, shared_ptr<bj_layout_info>> hd_layout::Layouts;
map<string,int> hd_layout::map_Plan_Sch;
const char* hd_layout::TAG="hd_layout";
/**
 * 版式列表发送
 * */
void hd_layout::LayoutListSend(bj_msg *msg, ServerTcpConnection *conn)
{
  //先处理数据
   shared_ptr<bj_layout_info> info=make_shared<bj_layout_info>();
   info->Parse(msg->Data);
   hd_layout::Layouts.insert(make_pair(info->PlanID, info));
   string planId = info->PlanID;
   string taskId=msg->TaskID;
   int* pSuccCount=new int(0);
   int* pCount=new int(0);
   int total=info->Files.size();
      for (std::vector<shared_ptr<bj_file_info>>::iterator iter = info->Files.begin(); iter != info->Files.end(); ++iter)
      {
    	  string downloadDir="/home/workspace/bj_media/layout/";
    	  if((*iter)->Type!=4)
    	  {
    		  downloadDir="/home/workspace/media/";
    	  }
   	   (*iter)->Download(downloadDir, [taskId,planId,pSuccCount,pCount,total,conn](bj_file_info* file)
		   {
			   if(file->HasDownloaded)
				{
				   (*pSuccCount)++;
				}
			   (*pCount)++;
			   if(*pCount==total)
			   {
				   if(*pSuccCount==*pCount)
				   {
					   SendDownloadReport(taskId, 2, conn, 2);//下载完毕
					   try
					   {
						   DealNewLayout(planId);
					   }
					   catch(std::runtime_error& e)
					   {
						   LogE("DealNewLayout %s error:%s.\n", planId.c_str(), e.what());
					   }
				   }
				   else
				   {
					   SendDownloadReport(taskId, 3, conn, 0);//下载失败
				   }
				   delete pSuccCount;
				   delete pCount;
			   }
			   else
			   {
				   int pct = *pCount * 100 / total;
				   SendDownloadReport(taskId, 1, conn, pct);//下载中
			   }
		   });
      }

  //再回复
  bj_msg reply;
  reply.Copy(*msg);
  reply.Command = "A51";
  reply.SystemCode = "10"; //SPIS
  reply.SetData("", 163);
  int index=0;
  TcpSetString(reply.Data, index, info->PlanID, 32);
  TcpSetString(reply.Data, index, "100", 3);
  TcpSetString(reply.Data, index, "ok", 128);
  char data[256]; //请保证自己buffer够用
  int len = reply.Write(data);
  conn->Send(data, len);
}

void hd_layout::DealNewLayout(const string& layoutGroupId)
{
	bj_layout_file::ParseLayoutList(layoutGroupId+".json");
	for(auto& layout : bj_layout_file::layout_list_map[layoutGroupId]->layout_list_layouts)
	{
		if(bj_layout_file::layout_map.count(layout.first)==0)
		{
			bj_layout_file::ParseLayout(layout.first+".json");
		}
	}

	LoadScheduleDetail(layoutGroupId);
	TransManager* transManager=LCDController::GetInstance()->GetTransManager();
	ScheduleHandler* handler = (ScheduleHandler*)TransHandlerFactory::Instance(transManager)->GetLoader(NotifyMessageCode::NTF_ScheduleUpdated);
	handler->sendMessage(new Message(NT_UpdateSchedule));
}

static Json::MediaBasic::MediaType bj_media_type_to_hq(int bj_media_type)
{
	switch(bj_media_type)
	{
		case TEXT:
			return Json::MediaBasic::MediaType::Text;
		case IMAGE://
				return Json::MediaBasic::MediaType::Image;
		case VIDEO://直播或视频
				return Json::MediaBasic::MediaType::Live;
		case FIRST_LAST_INFO:
		case STATION_ATS:
				return Json::MediaBasic::MediaType::ArrivalMsg;
		case DATE:
		case TIME:
				return Json::MediaBasic::MediaType::DigitalClock;
	}
	throw runtime_error("unknown bj media type"+to_string(bj_media_type));
}

bool hd_layout::LoadScheduleDetail(const std::string& layoutGroupId,bool onlyLoad)
{
	shared_ptr<bj_layout_list> list = bj_layout_file::layout_list_map[layoutGroupId];
	TransManager* transManager=LCDController::GetInstance()->GetTransManager();
	ScheduleHandler* handler = (ScheduleHandler*)TransHandlerFactory::Instance(transManager)->GetLoader(NotifyMessageCode::NTF_ScheduleUpdated);
	if(map_Plan_Sch.count(layoutGroupId)>0)
	{
		Json::ScheduleDetail* pSchedule=handler->mScheduleForPlay[map_Plan_Sch[layoutGroupId]];
		pSchedule->mScheduleBasic.mStartTime = datetime::Parse(list->startTime).ToString("%Y%m%d %H%M%S");
		pSchedule->mScheduleBasic.mEndTime =  datetime::Parse(list->endTime).ToString("%Y%m%d %H%M%S");
		pSchedule->mScheduleBasic.mPublishTime =  datetime::Now().ToString("%Y%m%d %H%M%S");
		pSchedule->mScheduleBasic.mUpdateTime =  datetime::Parse(list->updateTime).ToString("%Y%m%d %H%M%S");
		pSchedule->mScheduleBasic.mPriority=stoi(list->level);
		bj_layout_status::Update(layoutGroupId,pSchedule->mScheduleBasic.mPublishTime,pSchedule->mScheduleBasic.mPriority,"再次下发");
		return true;
	}
	Json::ScheduleDetail* scheduleDetail = new Json::ScheduleDetail();
	scheduleDetail->mScheduleBasic.mId=list->mId;
	scheduleDetail->mScheduleBasic.mDescrp = list->name;
	scheduleDetail->mScheduleBasic.mStartTime = datetime::Parse(list->startTime).ToString("%Y%m%d %H%M%S");
	scheduleDetail->mScheduleBasic.mEndTime =  datetime::Parse(list->endTime).ToString("%Y%m%d %H%M%S");
	scheduleDetail->mScheduleBasic.mPublishTime =  datetime::Now().ToString("%Y%m%d %H%M%S");
	scheduleDetail->mScheduleBasic.mUpdateTime =  datetime::Parse(list->updateTime).ToString("%Y%m%d %H%M%S");
	scheduleDetail->mScheduleBasic.mServerLevel = 0;
	scheduleDetail->mScheduleBasic.mPriority = stoi(list->level);

	Json::SchLayoutGroupBasic layoutGroupBsc;
	for (auto &kv : list->layout_list_layouts)
	{
		shared_ptr<bj_layout> layout=bj_layout_file::layout_map[kv.first];
		//schedule
		layoutGroupBsc.mId = layout->mId;
		layoutGroupBsc.mDscrp = kv.second->name;
		layoutGroupBsc.mStartTime = kv.second->startTime;
		layoutGroupBsc.mEndTime = kv.second->endTime;
		layoutGroupBsc.mSwitchTime = 86400;
		layoutGroupBsc.mUpdatedTime = datetime::Parse(kv.second->updateTime).ToString("%Y%m%d %H%M%S");
		layoutGroupBsc.mPartionMedias.clear();

		for (auto& part : layout->partitions_map)
		{
			Json::PartitionMedias partitonMedia;
			partitonMedia.mPartitionId = part.second->id;
			partitonMedia.mMediaIds.clear();
			partitonMedia.basMediaId=part.second->params->media_id;
			partitonMedia.mMediaIds.push_back(part.second->params->media_id);
			layoutGroupBsc.mPartionMedias.push_back(partitonMedia);
		}
		scheduleDetail->mLayoutGroups.push_back(layoutGroupBsc);

		//layout-group
		Json::LayoutGroupDetail *groupDetail = new Json::LayoutGroupDetail();
		groupDetail->mId = layout->mId;
		groupDetail->mDscrp = layout->name;
		groupDetail->mResolution = string_replace(layout->resolution,string(","),string("*"),0);
		//layout-layout
		Json::LayoutDetail layoutDetail;
		layoutDetail.mId = layout->mId;
		layoutDetail.mName = layout->name;
		layoutDetail.mDescription = layout->name;
		layoutDetail.mUpdateTime = datetime::Parse(layout->updateTime).ToString("%Y%m%d %H%M%S");
		layoutDetail.emer_ = layout->emer;
		layoutDetail.ch_en_switch_ = stoi(layout->chEnSwitch);
		layoutDetail.back_image_ = layout->backImage;

		//layout-partition
		Json::PartitionDetail partitionDetail;
		/*给加一个背景partition*/
		partitionDetail.mId=0;
		partitionDetail.mMediaType=0;
		partitionDetail.mBkgroudFile=layout->backImage;
		partitionDetail.mZorder=0;
		partitionDetail.mXpos=0;
		partitionDetail.mYpos=0;
		vector<string> resolution=string_split(layout->resolution,",");
		partitionDetail.mWidth=stoi(resolution[0]);
		partitionDetail.mHeight=stoi(resolution[1]);
		partitionDetail.mIsMaster=false;
		partitionDetail.mOPSflag=false;
		partitionDetail.mIsTransparent = false;
		layoutDetail.mPartitions.push_back(partitionDetail);
		/*其它partition*/
		for (auto& partition : layout->partitions_map)
		{
			partitionDetail.mId = partition.second->id;
			partitionDetail.mMediaType = bj_media_type_to_hq(partition.second->mediaType);
			partitionDetail.mIsSoundable = partitionDetail.mMediaType == Json::MediaBasic::MediaType::Live;
			vector<string> rect=string_split(partition.second->rect,",");
			partitionDetail.mXpos = stoi(rect[0]);
			partitionDetail.mYpos = stoi(rect[1]);
			partitionDetail.mWidth = stoi(rect[2]);
			partitionDetail.mHeight = stoi(rect[3]);
			partitionDetail.mZorder = partition.second->zOrder+1;
			partitionDetail.mIsTransparent = false;
			partitionDetail.mIsMaster = partitionDetail.mMediaType == Json::MediaBasic::MediaType::Live;
			partitionDetail.mBkgroudFile = partition.second->backImage;
			partitionDetail.mOPSflag = partitionDetail.mId == 301;
			partitionDetail.mUpdateTime = datetime::Now().ToString("%Y%m%d %H%M%S");
			layoutDetail.mPartitions.push_back(partitionDetail);
		}

		groupDetail->mLayoutDetails.push_back(layoutDetail);
		handler->mLayoutGroupForPlay.insert(std::make_pair(groupDetail->mId, groupDetail));

		//media
		for (auto& part : layout->partitions_map)
		{
			vector<Json::MediaBasic*> medias=GetMedia(part.second->params,part.second->rect);
			for(auto& pMedia:medias)
			{
				handler->mContentListForPlay.insert(std::make_pair(pMedia->mId, pMedia));
			}
		}
	}

	handler->mScheduleForPlay.insert(make_pair(scheduleDetail->mScheduleBasic.mId, scheduleDetail));
	map_Plan_Sch.insert(make_pair(layoutGroupId,scheduleDetail->mScheduleBasic.mId));
	if(!onlyLoad)
	{
		bj_layout_status::Add(layoutGroupId,scheduleDetail->mScheduleBasic.mPriority);
	}
	return true;
}

inline void SetFontByJson(Json::FontInfo& font,JsonObject& json)
{
	font.mName=(string)json["name"];
	font.mSize=(int)((int)json["size"]*0.65);
	font.mIsBold=json["bold"];
	font.mIsItalic=json["italic"];
}

inline Json::TextInfo::Align bj_align_to_hq(int align)
{
	switch(align)
	{
	case 2:
		return Json::TextInfo::Align::Right;
	case 4:
		return Json::TextInfo::Align::Center;
	}
	return Json::TextInfo::Align::Left;
}

inline Json::TextInfo::Effect bj_effect_to_hq(int effecf)
{
	switch(effecf)
	{
	case 0://left
		return Json::TextInfo::Effect::LeftScroll;
	case 1://right
		return Json::TextInfo::Effect::RightScroll;
	case 2://up
		return Json::TextInfo::Effect::UpScroll;
	}
	return Json::TextInfo::Effect::LeftScroll;
}

vector<Json::MediaBasic*> hd_layout::GetMedia(shared_ptr<bj_partition_media> param,const string& partRect)
{
	vector<Json::MediaBasic*> medias;
	switch(param->meida_type)
	{
		case bj_media_type::TEXT:
		{
			JsonObject& jsonText=param->param["text"];
			Json::MediaText* text=new Json::MediaText();
			text->mId=param->media_id;
			text->mType=bj_media_type_to_hq(param->meida_type);
			text->mParams.mContent=(string)jsonText["contentCh"];
			text->mParams.mContentEn = (string)jsonText["contentEn"];
			if(stoi(param->param["speed"].ToString())>0&&text->mParams.mContentEn.size()>0)
			{
				text->mParams.mContent+="\n";
				text->mParams.mContent+=text->mParams.mContentEn;
			}
			SetFontByJson(text->mParams.mFont, jsonText["fontCh"]);
			SetFontByJson(text->mParams.mFontEn, jsonText["fontEn"]);
			text->mParams.mForeColor=string_replace((string)jsonText["fontCh"]["textColor"],"#","");
			text->mParams.mAlign=bj_align_to_hq((int)jsonText["fontCh"]["align"]);
			text->mParams.mEffect=stoi(param->param["speed"].ToString())==0?Json::TextInfo::Effect::Static:bj_effect_to_hq((int)param->param["textDirection"]);
			//text->mParams.mSpeed=(Json::TextInfo::Speed)(4-std::min(stoi(param->param["speed"].ToString())/2, 4));
			text->mParams.mPixelPerSecond=stoi(param->param["speed"].ToString());
			medias.push_back(text);
			break;
		}
		case bj_media_type::IMAGE:
		{
			JsonObject& jsonImages=param->param["image"];
			for(JsonObject& json:jsonImages)
			{
				Json::MediaCommon2* image = new Json::MediaCommon2();
				image->mId=param->media_id;
				image->mType=bj_media_type_to_hq(param->meida_type);
				image->mFile=(string)json["fileName"];
				image->mDuration=param->param["switch"];
				medias.push_back(image);
			}
			break;
		}
		case bj_media_type::VIDEO:
		{
			Json::MediaCommon2* video = new Json::MediaCommon2();
			video->mId=param->media_id;
			video->mType=bj_media_type_to_hq(param->meida_type);
			string liveChannel=param->param["video"]["liveChannel"];
			if(!liveChannel.empty())
			{
				video->mUrl="udp://"+liveChannel;
			}
			video->mFile=(string)param->param["video"]["media"][0]["fileName"];
			//video->mDuration=286;
			medias.push_back(video);
			break;
		}
		case bj_media_type::DATE:
		{
			Json::MediaCommon1* date=new Json::MediaCommon1();
			date->mId=param->media_id;
			date->mType=bj_media_type_to_hq(param->meida_type);
			Json::LabelInfo label;
			label.mRect="0,0"+partRect.substr(partRect.find(',',partRect.find(',')+1));
			label.mVarText.mText.mContent="{0}";
			label.mVarText.mText.mContentEn = "{1}";
			SetFontByJson(label.mVarText.mText.mFont, param->param["date"]["fontCh"]);
			SetFontByJson(label.mVarText.mText.mFontEn, param->param["date"]["fontEn"]);
			label.mVarText.mText.mAlign=bj_align_to_hq((int)param->param["date"]["fontCh"]["align"]);
			label.mVarText.mText.mForeColor=string_replace((string)param->param["date"]["fontCh"]["textColor"],"#","");
			label.mVarText.mText.mEffect=Json::TextInfo::Effect::Static;
			label.mVarText.mText.mSpeed=Json::TextInfo::Speed::Normal;
			Json::VaribleInfo var;
			var.mTypeStr="system_time";
			var.mLanguage="CHS";
			var.mTimeFormat=(string)param->param["date"]["formatCh"];
			var.mTimeFormat=string_replace(var.mTimeFormat,"yyyy","%Y");//yyyy
			var.mTimeFormat=string_replace(var.mTimeFormat,"YYYY","%Y");
			var.mTimeFormat=string_replace(var.mTimeFormat,"MM","%m");
			var.mTimeFormat=string_replace(var.mTimeFormat,"dd","%d");
			var.mTimeFormat=string_replace(var.mTimeFormat,"EE","%W");
			label.mVarText.mVaribles.push_back(var);
			var.mTypeStr="system_time";
			var.mLanguage="ENG";
			var.mTimeFormat=(string)param->param["date"]["formatEn"];
			var.mTimeFormat=string_replace(var.mTimeFormat,"YYYY","%Y");//YYYY
			var.mTimeFormat=string_replace(var.mTimeFormat,"mm","%a");
			var.mTimeFormat=string_replace(var.mTimeFormat,"MM","%a");
			var.mTimeFormat=string_replace(var.mTimeFormat,"DD","%d");
			var.mTimeFormat=string_replace(var.mTimeFormat,"ee","%w");
			var.mTimeFormat=string_replace(var.mTimeFormat,"EE","%w");
			label.mVarText.mVaribles.push_back(var);
			date->mParams.push_back(label);
			medias.push_back(date);
			break;
		}
		case bj_media_type::TIME:
		{
			Json::MediaCommon1* time=new Json::MediaCommon1();
			time->mId=param->media_id;
			time->mType=bj_media_type_to_hq(param->meida_type);
			Json::LabelInfo label;
			label.mRect="0,0"+partRect.substr(partRect.find(',',partRect.find(',')+1));
			label.mVarText.mText.mContent="{0}";
			SetFontByJson(label.mVarText.mText.mFont, param->param["time"]["font"]);
			label.mVarText.mText.mAlign=bj_align_to_hq((int)param->param["time"]["font"]["align"]);
			label.mVarText.mText.mForeColor=string_replace((string)param->param["time"]["font"]["textColor"],"#","");
			label.mVarText.mText.mEffect=Json::TextInfo::Effect::Static;
			label.mVarText.mText.mSpeed=Json::TextInfo::Speed::Normal;
			Json::VaribleInfo var;
			var.mTypeStr="system_time";
			var.mLanguage="CHS";
			var.mTimeFormat=(string)param->param["time"]["format"];
			var.mTimeFormat=string_replace(var.mTimeFormat,"HH","%H");
			var.mTimeFormat=string_replace(var.mTimeFormat,"mm","%M");
			var.mTimeFormat=string_replace(var.mTimeFormat,"ss","%S");
			label.mVarText.mVaribles.push_back(var);
			time->mParams.push_back(label);
			medias.push_back(time);
			break;
		}
		case bj_media_type::STATION_ATS:
		{
			Json::MediaCommon1* ats=new Json::MediaCommon1();
			ats->mId=param->media_id;
			ats->mType=bj_media_type_to_hq(param->meida_type);
			for(int i=1;i<=3;i++)
			{
				if(param->param["ATS"].HasMember("train"+to_string(i)))
				{
					JsonObject& trainJson=param->param["ATS"]["train"+to_string(i)];
					if(trainJson.HasMember("dest"))
					{
						Json::LabelInfo label;
						label.mRect=(string)trainJson["dest"]["rect"];
						label.mVarText.mText.mContent=(string)trainJson["dest"]["tipCh"];
						label.mVarText.mText.mContentEn = (string)trainJson["dest"]["tipEn"];
						SetFontByJson(label.mVarText.mText.mFont, trainJson["dest"]["fontCh"]);
						SetFontByJson(label.mVarText.mText.mFontEn, trainJson["dest"]["fontEn"]);
						label.mVarText.mText.mAlign=bj_align_to_hq((int)trainJson["dest"]["fontCh"]["align"]);
						label.mVarText.mText.mForeColor=string_replace((string)trainJson["dest"]["fontCh"]["textColor"],"#","");
						label.mVarText.mText.mEffect=Json::TextInfo::Effect::Static;
						label.mVarText.mText.mSpeed=Json::TextInfo::Speed::Normal;
						ats->mParams.push_back(label);
						JsonObject& jsonContent=trainJson["dest"]["content"];
						label.mRect=(string)jsonContent["rect"];
						label.mVarText.mText.mContent="{0}";
						label.mVarText.mText.mContentEn="{1}";
						SetFontByJson(label.mVarText.mText.mFont, jsonContent["fontCh"]);
						SetFontByJson(label.mVarText.mText.mFontEn, jsonContent["fontEn"]);
						label.mVarText.mText.mAlign=bj_align_to_hq((int)jsonContent["fontCh"]["align"]);
						label.mVarText.mText.mForeColor=string_replace((string)jsonContent["fontCh"]["textColor"],"#","");
						label.mVarText.mText.mEffect=Json::TextInfo::Effect::Static;
						label.mVarText.mText.mSpeed=Json::TextInfo::Speed::Normal;
						Json::VaribleInfo var;
						var.mTypeStr="destCh";
						var.mLanguage="CHS";
						var.mTrainIndex=i-1;
						label.mVarText.mVaribles.push_back(var);
						var.mTypeStr="destEn";
						var.mLanguage="ENG";
						var.mTrainIndex=i-1;
						label.mVarText.mVaribles.push_back(var);
						ats->mParams.push_back(label);
					}
					if(trainJson.HasMember("trainTip"))
					{
						Json::LabelInfo label;
						label.mRect=(string)trainJson["trainTip"]["rect"];
						label.mVarText.mText.mContent=(string)trainJson["trainTip"]["tipCh"];
						label.mVarText.mText.mContentEn = (string)trainJson["trainTip"]["tipEn"];
						SetFontByJson(label.mVarText.mText.mFont, trainJson["trainTip"]["fontCh"]);
						SetFontByJson(label.mVarText.mText.mFontEn, trainJson["trainTip"]["fontEn"]);
						label.mVarText.mText.mAlign=bj_align_to_hq((int)trainJson["trainTip"]["fontCh"]["align"]);
						label.mVarText.mText.mForeColor=string_replace((string)trainJson["trainTip"]["fontCh"]["textColor"],"#","");
						label.mVarText.mText.mEffect=Json::TextInfo::Effect::Static;
						label.mVarText.mText.mSpeed=Json::TextInfo::Speed::Normal;
						ats->mParams.push_back(label);

						if (trainJson["trainTip"].HasMember("content"))
						{
							JsonObject &jsonContent = trainJson["trainTip"]["content"];

							label.mRect = (string) jsonContent["status"]["rect"];
							label.mVarText.mText.mContent = "{0}";
							label.mVarText.mText.mContentEn = "{1}";
							SetFontByJson(label.mVarText.mText.mFont, jsonContent["status"]["fontCh"]);
							SetFontByJson(label.mVarText.mText.mFontEn, jsonContent["status"]["fontEn"]);
							label.mVarText.mText.mAlign = bj_align_to_hq((int) jsonContent["status"]["fontCh"]["align"]);
							label.mVarText.mText.mForeColor = string_replace((string) jsonContent["status"]["fontCh"]["textColor"], "#", "");
							label.mVarText.mText.mEffect = Json::TextInfo::Effect::Static;
							label.mVarText.mText.mSpeed = Json::TextInfo::Speed::Normal;
							Json::VaribleInfo var;
							var.mTypeStr = "statusCh";
							var.mLanguage = "CHS";
							var.mTrainIndex = i - 1;
							label.mVarText.mVaribles.push_back(var);
							var.mTypeStr = "statusEn";
							var.mLanguage = "ENG";
							var.mTrainIndex = i - 1;
							label.mVarText.mVaribles.push_back(var);
							ats->mParams.push_back(label);
							label.mVarText.mVaribles.clear();

							label.mRect = (string) jsonContent["time"]["rect"];
							label.mVarText.mText.mContent = "{0}";
							label.mVarText.mText.mContentEn.clear();
							SetFontByJson(label.mVarText.mText.mFont, jsonContent["time"]["fontCh"]);
							label.mVarText.mText.mAlign = bj_align_to_hq((int) jsonContent["time"]["fontCh"]["align"]);
							label.mVarText.mText.mForeColor = string_replace((string) jsonContent["time"]["fontCh"]["textColor"], "#", "");
							label.mVarText.mText.mEffect = Json::TextInfo::Effect::Static;
							label.mVarText.mText.mSpeed = Json::TextInfo::Speed::Normal;
							var.mTypeStr = "time";
							var.mLanguage = "CHS";
							var.mTrainIndex = i - 1;
							label.mVarText.mVaribles.push_back(var);
							ats->mParams.push_back(label);
							label.mVarText.mVaribles.clear();

							label.mRect = (string) jsonContent["min"]["rect"];
							label.mVarText.mText.mContent = "{0}";
							label.mVarText.mText.mContentEn = "{1}";
							SetFontByJson(label.mVarText.mText.mFont, jsonContent["min"]["fontCh"]);
							SetFontByJson(label.mVarText.mText.mFontEn, jsonContent["min"]["fontEn"]);
							label.mVarText.mText.mAlign = bj_align_to_hq((int) jsonContent["min"]["fontCh"]["align"]);
							label.mVarText.mText.mForeColor = string_replace((string) jsonContent["min"]["fontCh"]["textColor"], "#", "");
							label.mVarText.mText.mEffect = Json::TextInfo::Effect::Static;
							label.mVarText.mText.mSpeed = Json::TextInfo::Speed::Normal;
							var.mTypeStr = "timeCh";
							var.mLanguage = "CHS";
							var.mTrainIndex = i - 1;
							label.mVarText.mVaribles.push_back(var);
							var.mTypeStr = "timeEn";
							var.mLanguage = "ENG";
							var.mTrainIndex = i - 1;
							label.mVarText.mVaribles.push_back(var);
							ats->mParams.push_back(label);
						}
					}
				}
			}
			medias.push_back(ats);
			break;
		}
		case bj_media_type::FIRST_LAST_INFO:
		{
			Json::MediaCommon1* atsFL=new Json::MediaCommon1();
			atsFL->mId=param->media_id;
			atsFL->mType=bj_media_type_to_hq(param->meida_type);
			for(JsonObject& jsonPage : param->param["trainFLPage"])
			{
				for(JsonObject& jsonInfo : jsonPage["trainFLInfo"])
				{
					Json::LabelInfo label;
					label.mRect=(string)jsonInfo["rectTip"];
					label.mVarText.mText.mContent=(string)jsonInfo["tipCh"];
					label.mVarText.mText.mContentEn = (string)jsonInfo["tipEn"];
					SetFontByJson(label.mVarText.mText.mFont, jsonInfo["fontTipCh"]);
					SetFontByJson(label.mVarText.mText.mFontEn, jsonInfo["fontTipEn"]);
					label.mVarText.mText.mAlign=bj_align_to_hq((int)jsonInfo["fontTipCh"]["align"]);
					label.mVarText.mText.mForeColor=string_replace((string)jsonInfo["fontTipCh"]["textColor"],"#","");
					label.mVarText.mText.mEffect=Json::TextInfo::Effect::Static;
					label.mVarText.mText.mSpeed=Json::TextInfo::Speed::Normal;
					atsFL->mParams.push_back(label);

					label.mRect=(string)jsonInfo["rectDest"];
					label.mVarText.mText.mContent=(string)jsonInfo["destCh"];
					label.mVarText.mText.mContentEn = (string)jsonInfo["destEn"];
					SetFontByJson(label.mVarText.mText.mFont, jsonInfo["fontDestCh"]);
					SetFontByJson(label.mVarText.mText.mFontEn, jsonInfo["fontDestEn"]);
					label.mVarText.mText.mAlign=bj_align_to_hq((int)jsonInfo["fontDestCh"]["align"]);
					label.mVarText.mText.mForeColor=string_replace((string)jsonInfo["fontDestCh"]["textColor"],"#","");
					label.mVarText.mText.mEffect=Json::TextInfo::Effect::Static;
					label.mVarText.mText.mSpeed=Json::TextInfo::Speed::Normal;
					atsFL->mParams.push_back(label);

					label.mRect=(string)jsonInfo["rectFirst"];
					label.mVarText.mText.mContent=(string)jsonInfo["firstCh"];
					label.mVarText.mText.mContentEn = (string)jsonInfo["firstEn"];
					SetFontByJson(label.mVarText.mText.mFont, jsonInfo["fontFirstCh"]);
					SetFontByJson(label.mVarText.mText.mFontEn, jsonInfo["fontFirstEn"]);
					label.mVarText.mText.mAlign=bj_align_to_hq((int)jsonInfo["fontFirstCh"]["align"]);
					label.mVarText.mText.mForeColor=string_replace((string)jsonInfo["fontFirstCh"]["textColor"],"#","");
					label.mVarText.mText.mEffect=Json::TextInfo::Effect::Static;
					label.mVarText.mText.mSpeed=Json::TextInfo::Speed::Normal;
					atsFL->mParams.push_back(label);

					label.mRect=(string)jsonInfo["rectFirstTime"];
					label.mVarText.mText.mContent=(string)jsonInfo["firstTime"];
					label.mVarText.mText.mContentEn.clear();
					SetFontByJson(label.mVarText.mText.mFont, jsonInfo["fontFirstTime"]);
					label.mVarText.mText.mAlign=bj_align_to_hq((int)jsonInfo["fontFirstTime"]["align"]);
					label.mVarText.mText.mForeColor=string_replace((string)jsonInfo["fontFirstTime"]["textColor"],"#","");
					label.mVarText.mText.mEffect=Json::TextInfo::Effect::Static;
					label.mVarText.mText.mSpeed=Json::TextInfo::Speed::Normal;
					atsFL->mParams.push_back(label);

					label.mRect=(string)jsonInfo["rectLast"];
					label.mVarText.mText.mContent=(string)jsonInfo["lastCh"];
					label.mVarText.mText.mContentEn = (string)jsonInfo["lastEn"];
					SetFontByJson(label.mVarText.mText.mFont, jsonInfo["fontLastCh"]);
					SetFontByJson(label.mVarText.mText.mFontEn, jsonInfo["fontLastEn"]);
					label.mVarText.mText.mAlign=bj_align_to_hq((int)jsonInfo["fontLastCh"]["align"]);
					label.mVarText.mText.mForeColor=string_replace((string)jsonInfo["fontLastCh"]["textColor"],"#","");
					label.mVarText.mText.mEffect=Json::TextInfo::Effect::Static;
					label.mVarText.mText.mSpeed=Json::TextInfo::Speed::Normal;
					atsFL->mParams.push_back(label);

					label.mRect=(string)jsonInfo["rectLastTime"];
					label.mVarText.mText.mContent=(string)jsonInfo["lastTime"];
					label.mVarText.mText.mContentEn.clear();
					SetFontByJson(label.mVarText.mText.mFont, jsonInfo["fontLastTime"]);
					label.mVarText.mText.mAlign=bj_align_to_hq((int)jsonInfo["fontLastTime"]["align"]);
					label.mVarText.mText.mForeColor=string_replace((string)jsonInfo["fontLastTime"]["textColor"],"#","");
					label.mVarText.mText.mEffect=Json::TextInfo::Effect::Static;
					label.mVarText.mText.mSpeed=Json::TextInfo::Speed::Normal;
					atsFL->mParams.push_back(label);
				}
			}
			medias.push_back(atsFL);
			break;
		}
		default:
		{
			break;
		}
	}
	return medias;
}

/**
 * 版式控制发送
 * */
void hd_layout::LayoutControlSend(bj_msg *msg, ServerTcpConnection *conn)
{
  // 计划ID
  int i = 0;
  string planId = TcpGetString(msg->Data, i, 32);
  // 命令类型 start、stop 、 revoke
  string cmd = TcpGetString(msg->Data, i, 8);

	if (map_Plan_Sch.count(planId) > 0)
	{
		TransManager *transManager = LCDController::GetInstance()->GetTransManager();
		ScheduleHandler *handler = (ScheduleHandler*) TransHandlerFactory::Instance(transManager)->GetLoader(NotifyMessageCode::NTF_ScheduleUpdated);
		if (handler->mScheduleForPlay.count(map_Plan_Sch[planId]) > 0)
		{
			Json::ScheduleDetail *sd = handler->mScheduleForPlay[map_Plan_Sch[planId]];
			if (cmd == "start")
			{
				sd->mScheduleBasic.mPublishTime = datetime::Now().ToString("%Y%m%d %H%M%S");
				bj_layout_status::Update(planId,sd->mScheduleBasic.mPublishTime,sd->mScheduleBasic.mPriority,cmd);
			}
			else if (cmd == "stop")
			{
				sd->mScheduleBasic.mPublishTime = "19000101 000000";
				bj_layout_status::Update(planId,sd->mScheduleBasic.mPublishTime,sd->mScheduleBasic.mPriority,cmd);
			}
			else if (cmd == "revoke") //撤销
			{
				sd->mScheduleBasic.mPublishTime = "19000101 000000";
				bj_layout_status::Update(planId, sd->mScheduleBasic.mPublishTime, sd->mScheduleBasic.mPriority, cmd);
				//if(sd!=handler->mCurrentScheduleForPlay)
				//{
				//	//其实是要把整个版式从硬盘中删除的，懒得做了。guorenjing20220809_1450
				//	for(auto&kv_layoutList : bj_layout_file::layout_list_map[planId]->layout_list_layouts)
				//	{
				//		for(auto& kv_part : bj_layout_file::layout_map[kv_layoutList.first]->partitions_map)
				//		{
				//			delete handler->mContentListForPlay[kv_part.second->params->media_id];
				//			handler->mContentListForPlay.erase(kv_part.second->params->media_id);
				//		}
				//	}
				//	for(auto& group:sd->mLayoutGroups)
				//	{
				//		delete handler->mLayoutGroupForPlay[group.mId];
				//		handler->mLayoutGroupForPlay.erase(group.mId);
				//	}
				//	delete sd;
				//	sd=nullptr;
				//	handler->mScheduleForPlay.erase(map_Plan_Sch[planId]);
				//	map_Plan_Sch.erase(planId);
				//	bj_layout_status::Delete(planId);
				//}
			}
			else if (cmd == "switch") //强制切换至此版式
			{
				sd->mScheduleBasic.mPriority = 999;
				sd->mScheduleBasic.mPublishTime = datetime::Now().ToString("%Y%m%d %H%M%S");
				bj_layout_status::Update(planId,sd->mScheduleBasic.mPublishTime,sd->mScheduleBasic.mPriority,cmd);
			}
			handler->sendMessage(new Message(NT_UpdateSchedule));
		}
	}

	bj_msg reply;
	reply.Copy(*msg);
	reply.Command = "A45";
	reply.SystemCode = "10"; //SPIS
	reply.SetData("", 163);
	int index=0;
	TcpSetString(reply.Data, index, planId, 32);
	TcpSetString(reply.Data, index, "100", 3);
	TcpSetString(reply.Data, index, "ok", 128);
	char data[256]; //请保证自己buffer够用
	int len = reply.Write(data);
	conn->Send(data, len);
}

void hd_layout::LoadLocal()
{
	TransManager* transManager=LCDController::GetInstance()->GetTransManager();
	ScheduleHandler* handler = (ScheduleHandler*)TransHandlerFactory::Instance(transManager)->GetLoader(NotifyMessageCode::NTF_ScheduleUpdated);
	bj_layout_status::ReadFromFile();
	for(auto& layoutStatus : bj_layout_status::ALL)
	{
		bj_layout_file::ParseLayoutList(layoutStatus.first+".json");
		for(auto& layout : bj_layout_file::layout_list_map[layoutStatus.first]->layout_list_layouts)
		{
			if(bj_layout_file::layout_map.count(layout.first)==0)
			{
				bj_layout_file::ParseLayout(layout.first+".json");
			}
		}
		LoadScheduleDetail(layoutStatus.first,true);
		if(map_Plan_Sch.count(layoutStatus.first)>0)
		{
			if(handler->mScheduleForPlay.count(map_Plan_Sch[layoutStatus.first]))
			{
				handler->mScheduleForPlay[map_Plan_Sch[layoutStatus.first]]->mScheduleBasic.mPriority=layoutStatus.second.level;
				handler->mScheduleForPlay[map_Plan_Sch[layoutStatus.first]]->mScheduleBasic.mPublishTime=layoutStatus.second.publishTime;
			}
		}
	}
}
