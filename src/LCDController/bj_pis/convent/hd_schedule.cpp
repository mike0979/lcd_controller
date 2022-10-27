#include "hd_schedule.h"
#include "tcp_msg_help.h"
#include "bj_pis/utils/JsonObject.h"
#include "bj_pis/utils/datetime.h"
#include "t_command.h"
#include "Log.h"
#include "bj_media_info.h"
#include "transmanage/TransHandlerFactory.h"
#include "LCDController.h"

//一个cpp里只能写一次，这个会在程序开始时运行。比main函数还早
STATIC_REGISTRATION(hd_schedule)
{
	bj_msg::GetHandlers()["M52"] = &hd_schedule::PlayListSend;
	bj_msg::GetHandlers()["M46"] = &hd_schedule::PlayListControl;
}

map<string, shared_ptr<bj_plan_info>> hd_schedule::Plans;

const char *hd_schedule::TAG = "hd_schedule";

void hd_schedule::PlayListSend(bj_msg *msg, ServerTcpConnection *conn)
{
	shared_ptr<bj_plan_info> info = make_shared<bj_plan_info>();
	info->Parse(msg->Data);
	string planId = info->PlanID;
	hd_schedule::Plans.insert(make_pair(info->PlanID, info));
	int *pSuccCount = new int(0);
	int *pCount = new int(0);
	int total = info->Files.size();
	for (std::vector<shared_ptr<bj_file_info>>::iterator iter = info->Files.begin(); iter != info->Files.end(); ++iter)
	{
		string taskId = msg->TaskID;
		string downloadDir = "/home/workspace/bj_media/plan/";
		if ((*iter)->Type != 4)
		{
			downloadDir = "/home/workspace/media/";
		}
		(*iter)->Download(downloadDir, [taskId, pSuccCount, pCount, total, conn, planId](bj_file_info *file)
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
					SendDownloadReport(taskId, 2, conn, 2); //下载完毕
				try
				{
					DealPlayList(planId);
				}
				catch(runtime_error& e)
				{
					LogE("DealPlayList %s error:%s.\n", planId.c_str(), e.what());
				}
			}
			else
			{
				SendDownloadReport(taskId, 3, conn, 0); //下载失败
			}
			delete pSuccCount;
			delete pCount;
		}
		else
		{
			int pct = *pCount * 100 / total;
			SendDownloadReport(taskId, 1, conn, pct); //下载中
		}
	});
	}

	bj_msg reply;
	reply.Copy(*msg);
	reply.Command = "A52";
	reply.SystemCode = "10"; //SPIS
	char result[163 + 1] = { '\0' };
	int index = 0;
	TcpSetString(result, index, info->PlanID, 32);
	TcpSetString(result, index, "100", 3);
	TcpSetString(result, index, "ok", 128);
	reply.SetData(result, index);

	char data[256]; //请保证自己buffer够用
	int len = reply.Write(data);
	conn->Send(data, len);
}

void hd_schedule::PlayListControl(bj_msg *msg, ServerTcpConnection *conn)
{
	// 计划ID
	int i = 0;
	string planId = TcpGetString(msg->Data, i, 32);
	// 命令类型 start、stop 、 revoke
	string cmd = TcpGetString(msg->Data, i, 8);
	if(bj_playlist_info::PlayLists.count(planId)>0)
	{
		if(cmd=="start")
		{
			bj_playlist_info::PlayLists[planId]->Enabled=true;
			bj_playlist_info::PlayLists[planId]->PublishTime=datetime::Now().ToString("%Y%m%d %H%M%S");
			bj_playlist_status::Update(planId,true,bj_playlist_info::PlayLists[planId]->PublishTime,cmd);
		}
		else if(cmd=="stop")
		{
			bj_playlist_info::PlayLists[planId]->Enabled=false;
			bj_playlist_status::Update(planId,false,bj_playlist_info::PlayLists[planId]->PublishTime,cmd);
		}
		else if(cmd=="revoke")
		{
			bj_playlist_info::PlayLists.erase(planId);
			bj_playlist_status::Delete(planId);
		}
		TransManager *transManager = LCDController::GetInstance()->GetTransManager();
		ScheduleHandler *handler = (ScheduleHandler*) TransHandlerFactory::Instance(transManager)->GetLoader(NotifyMessageCode::NTF_ScheduleUpdated);
		handler->sendMessage(new Message(NT_UpdateSchedule));
	}

	bj_msg reply;
	reply.Copy(*msg);
	reply.Command = "A46";
	reply.SystemCode = "10"; //SPIS
	reply.SetData("", 163);
	int index = 0;
	TcpSetString(reply.Data, index, planId, 32);
	TcpSetString(reply.Data, index, "100", 3);
	TcpSetString(reply.Data, index, "ok", 128);
	char data[256]; //请保证自己buffer够用
	int len = reply.Write(data);
	conn->Send(data, len);
}

void hd_schedule::DealPlayList(const string &planId,bool reloadSchedule)
{
	TransManager *transManager = LCDController::GetInstance()->GetTransManager();
	ScheduleHandler *handler = (ScheduleHandler*) TransHandlerFactory::Instance(transManager)->GetLoader(NotifyMessageCode::NTF_ScheduleUpdated);
	string md5=bj_file_info::GetMd5("/home/workspace/bj_media/plan/" + planId + ".json");
	if (bj_playlist_info::PlayLists.count(planId) > 0)
	{
		if(bj_playlist_info::PlayLists[planId]->Md5==md5)
		{
			handler->sendMessage(new Message(NT_UpdateSchedule));
			return;
		}
	}
	JsonObject json_playlist;
	json_playlist.FromFile("/home/workspace/bj_media/plan/" + planId + ".json");

	shared_ptr<bj_playlist_info> playlist = make_shared<bj_playlist_info>();
	playlist->ID = (string) json_playlist["id"];
	playlist->Name = (string) json_playlist["name"];
	playlist->UpdateTime = datetime::Parse(json_playlist["updatedTime"]).ToString("%Y%m%d %H%M%S");
	playlist->StartTime = datetime::Parse(json_playlist["startTime"]).ToString("%Y%m%d %H%M%S");
	playlist->EndTime = datetime::Parse(json_playlist["endTime"]).ToString("%Y%m%d %H%M%S");
	playlist->PublishTime = datetime::Now().ToString("%Y%m%d %H%M%S");
	playlist->WeekFlag = (string) json_playlist["weekFlag"];
	playlist->Md5=md5;
	if (json_playlist.HasMember("level"))
	{
		playlist->Level = json_playlist["level"];
	}

	JsonObject &jsonProgram = json_playlist["programs"];
	for (JsonObject &jo : jsonProgram)
	{
		if (jo.GetName() == "text")
		{
			vector<shared_ptr<pl_media_info>> vct_pl_media;
			for (JsonObject &arrItem : jo)
			{
				for (JsonObject &item : arrItem["list"])
				{
					Json::MediaText *text = new Json::MediaText();
					text->mId = bj_partition_media::GetNewMediaId();
					text->mType = Json::MediaBasic::MediaType::Text;
					text->mParams.mContent = (string) item["content"];
					shared_ptr<pl_media_info> plMedia = make_shared<pl_media_info>();
					plMedia->partitionID = stoi(arrItem["id"].ToString());
					plMedia->StartTime = (string) item["startTime"];
					plMedia->StartTime.erase(5).erase(2);
					plMedia->EndTime = (string) item["endTime"];
					plMedia->EndTime.erase(5).erase(2);
					plMedia->media = text;
					vct_pl_media.push_back(plMedia);
					handler->mContentListForPlay.insert(make_pair(text->mId, text));
				}
			}
			if (vct_pl_media.size() > 0)
				playlist->MediaList.insert(make_pair((int) Json::MediaBasic::MediaType::Text, vct_pl_media));
		}
		else if (jo.GetName() == "image")
		{
			vector<shared_ptr<pl_media_info>> vct_pl_media;
			for (JsonObject &arrItem : jo)
			{
				for (JsonObject &item : arrItem["list"])
				{
					Json::MediaCommon2 *image = new Json::MediaCommon2();
					image->mId = bj_partition_media::GetNewMediaId();
					image->mType = Json::MediaBasic::MediaType::Image;
					image->mFile = (string) item["fileName"];
					image->mDuration = stoi(arrItem["switch"].ToString());
					shared_ptr<pl_media_info> plMedia = make_shared<pl_media_info>();
					plMedia->partitionID = stoi(arrItem["id"].ToString());
					plMedia->StartTime = (string) item["startTime"];
					plMedia->EndTime = (string) item["endTime"];
					plMedia->media = image;
					vct_pl_media.push_back(plMedia);
					handler->mContentListForPlay.insert(make_pair(image->mId, image));
				}
			}
			if (vct_pl_media.size() > 0)
				playlist->MediaList.insert(make_pair((int) Json::MediaBasic::MediaType::Image, vct_pl_media));
		}
		else if (jo.GetName() == "video")
		{
			vector<shared_ptr<pl_media_info>> vct_pl_media;
			for (JsonObject &arrItem : jo)
			{
				for (JsonObject &item : arrItem["list"])
				{
					Json::MediaCommon2 *video = new Json::MediaCommon2();
					video->mId = bj_partition_media::GetNewMediaId();
					video->mType = Json::MediaBasic::MediaType::Live;
					if ((bool) item["liveFlag"])
					{
						video->mUrl = "udp://" + (string) item["liveChannel"];
					}
					else
					{
						video->mFile = (string) item["fileName"];
					}
					shared_ptr<pl_media_info> plMedia = make_shared<pl_media_info>();
					plMedia->partitionID = stoi(arrItem["id"].ToString());
					plMedia->StartTime = (string) item["startTime"];
					plMedia->EndTime = (string) item["endTime"];
					plMedia->media = video;
					vct_pl_media.push_back(plMedia);
					handler->mContentListForPlay.insert(make_pair(video->mId, video));
				}
			}
			if (vct_pl_media.size() > 0)
				playlist->MediaList.insert(make_pair((int) Json::MediaBasic::MediaType::Live, vct_pl_media));
		}
	}
	bj_playlist_info::PlayLists[planId] = playlist;
	if(reloadSchedule)
	{
		bj_playlist_status::Add(planId);
		handler->sendMessage(new Message(NT_UpdateSchedule));
	}
}

void hd_schedule::SetPlaylistMedia(Json::ScheduleDetail *sch, ScheduleHandler *handler)
{
	if (sch == NULL)
		return;
	for (auto &lg : sch->mLayoutGroups)
	{
		for (auto &pm : lg.mPartionMedias)
		{
			if (pm.basMediaId == 0)
				return;
			pm.mMediaIds.clear();
			GetPlayListMedia(pm, handler);
			if (pm.mMediaIds.size() == 0)
			{
				pm.mMediaIds.push_back(pm.basMediaId);
			}
		}
	}
}

void hd_schedule::GetPlayListMedia(Json::PartitionMedias &pm, ScheduleHandler *handler)
{
	string now = datetime::Now().ToString("%Y%m%d %H%M%S");
	string time=now.substr(9, 6);
	//mediaType : level
	map<int, int> maxLevel =
	{
	{ (int) Json::MediaBasic::MediaType::Text, -1 },
	{ (int) Json::MediaBasic::MediaType::Image, -1 },
	{ (int) Json::MediaBasic::MediaType::Live, -1 } };
	//mediaType : publish_time
	map<int, string> latestPublish =
	{
	{ (int) Json::MediaBasic::MediaType::Text, "19700101 000000" },
	{ (int) Json::MediaBasic::MediaType::Image, "19700101 000000" },
	{ (int) Json::MediaBasic::MediaType::Live, "19700101 000000" } };
	for (auto &playList : bj_playlist_info::PlayLists)
	{
		LogD("playList key=%s PublishTime=%s\n", playList.first.c_str(), playList.second->PublishTime.c_str());
		if (playList.second->Enabled)
		{
			if (playList.second->StartTime <= now && now < playList.second->EndTime)
			{
				for (auto &mediaSomeType : playList.second->MediaList)
				{
					LogD("mediaSomeType.second[0]->partitionID=%d\n", mediaSomeType.second[0]->partitionID);
					if (pm.mPartitionId == (int) mediaSomeType.second[0]->partitionID //partitionID相等
						&& playList.second->Level >= maxLevel[mediaSomeType.first] //优先级
						&& playList.second->PublishTime >= latestPublish[mediaSomeType.first])//发布时间
					{
						LogD("partitionID=%d\n", mediaSomeType.second[0]->partitionID);
						LogD("maxLevel=%d\n", maxLevel[mediaSomeType.first]);
						LogD("latestPublish=%s\n", latestPublish[mediaSomeType.first].c_str());

						//如果没有一个符合播放时间的媒体，则跳过该类型
						bool flagHasAvaliable=false;
						for (auto &plMedia : mediaSomeType.second)
						{
							if(now>=plMedia->StartTime && now<plMedia->EndTime)
							{
								flagHasAvaliable=true;
								break;
							}
						}
						if(!flagHasAvaliable)continue;//如果没有一个符合播放时间的媒体，则跳过该类型

						maxLevel[mediaSomeType.first] = playList.second->Level;
						latestPublish[mediaSomeType.first] = playList.second->PublishTime;

						pm.mMediaIds.clear();
						for (auto &plMedia : mediaSomeType.second)
						{
							pm.mMediaIds.push_back(plMedia->media->mId);
							if (plMedia->media->mType == Json::MediaBasic::MediaType::Text)
							{
								Json::MediaText *baseText = (Json::MediaText*) handler->mContentListForPlay[pm.basMediaId];
								Json::MediaText *playlistText = (Json::MediaText*) plMedia->media;
								playlistText->mParams.mFont = baseText->mParams.mFont;
								playlistText->mParams.mForeColor = baseText->mParams.mForeColor;
								playlistText->mParams.mAlign = baseText->mParams.mAlign;
								playlistText->mParams.mEffect = baseText->mParams.mEffect;
								playlistText->mParams.mPixelPerSecond = baseText->mParams.mPixelPerSecond;
							}
						}
					}
				}
			}
		}
	}
}

void hd_schedule::LoadLocal()
{
	bj_playlist_status::ReadFromFile();
	for(auto& kv : bj_playlist_status::ALL)
	{
		DealPlayList(kv.first,false);
		bj_playlist_info::PlayLists[kv.first]->Enabled=kv.second.Enabled;
		bj_playlist_info::PlayLists[kv.first]->PublishTime=kv.second.PublishTime;
	}
}
