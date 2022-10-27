#include "bj_layout_info.h"
#include <cstring>
#include "bj_pis/utils/datetime.h"
#include <stdio.h>
#include "tcp_msg_help.h"
#include "bj_pis/utils/string_ext.h"
#include <fstream>

void bj_layout_info::Parse(const char *data)
{
	int index = 0;
	PlanName = TcpGetString(data, index, 64);
	PlanID = TcpGetString(data, index, 32);
	UpdateTime = TcpGetDateTime(data, index, 14);
	int count = TcpGetInt(data, index, 3);
	Files.reserve(count);
	for (int i = 0; i < count; i++)
	{
		shared_ptr<bj_file_info> file = make_shared<bj_file_info>();
		int c = file->Parse(data + index);
		Files.push_back(file);
		index += c;
	}
}

int bj_layout_info::Write(char *data)
{
	int index = 0;
	TcpSetString(data, index, PlanName, 64);
	TcpSetString(data, index, PlanID, 32);
	TcpSetDateTime(data, index, UpdateTime, 14);
	TcpSetInt(data, index, Files.size(), 3);
	for (std::vector<shared_ptr<bj_file_info>>::iterator iter = Files.begin(); iter != Files.end(); ++iter)
	{
		int c = (*iter)->Write(data);
		index += c;
	}
	return index;
}

map<string, shared_ptr<bj_layout_list>> bj_layout_file::layout_list_map;
map<string, shared_ptr<bj_layout>> bj_layout_file::layout_map;
int bj_layout::s_id=0;
int bj_layout_list::s_id=0;

void bj_layout_file::ParseLayoutList(const string &layout_group_name)
{
	string txtLayoutGroup = ReadTextFile("/home/workspace/bj_media/layout/" + layout_group_name);
	JsonObject jsonGroup;
	jsonGroup.Parse(txtLayoutGroup);
	if(bj_layout_file::layout_list_map.count(jsonGroup["id"].ToString())>0)
	{
		shared_ptr<bj_layout_list> lout_list=bj_layout_file::layout_list_map[jsonGroup["id"].ToString()];
		lout_list->updateTime = (string) jsonGroup["updateTime"].ToString();
		lout_list->startTime = (string) jsonGroup["startTime"].ToString();
		lout_list->endTime = (string) jsonGroup["endTime"].ToString();
		return;
	}
	//获取版式列表
	shared_ptr<bj_layout_list> layout_list=make_shared<bj_layout_list>();
	layout_list->enabled=true;
	layout_list->mId=bj_layout_list::GetIntId();
	layout_list->id = jsonGroup["id"].ToString();
	layout_list->name = (string) jsonGroup["name"].ToString();
	layout_list->updateTime = (string) jsonGroup["updateTime"].ToString();
	layout_list->startTime = (string) jsonGroup["startTime"].ToString();
	layout_list->endTime = (string) jsonGroup["endTime"].ToString();
	layout_list->weekFlag = (string) jsonGroup["weekFlag"].ToString();
	layout_list->resolution = (string) jsonGroup["resolution"].ToString();
	layout_list->level = (string) jsonGroup["level"].ToString();

	for (JsonObject &jsonLayout : jsonGroup["layouts"])
	{
		shared_ptr<bj_layout_list_layouts> bj_layout_list_layout=make_shared<bj_layout_list_layouts>();
		bj_layout_list_layout->id = jsonLayout["id"].ToString();
		bj_layout_list_layout->name = jsonLayout["name"].ToString();
		bj_layout_list_layout->updateTime = jsonLayout["updateTime"].ToString();
		bj_layout_list_layout->startTime = jsonLayout["startTime"].ToString();
		bj_layout_list_layout->endTime = jsonLayout["endTime"].ToString();
		bj_layout_list_layout->weekFlag = jsonLayout["weekFlag"].ToString();
		if(layout_list->layout_list_layouts.count(bj_layout_list_layout->id)==0)
		{
			layout_list->layout_list_layouts.insert(make_pair(bj_layout_list_layout->id, bj_layout_list_layout));
		}
	}
	bj_layout_file::layout_list_map.insert(make_pair(layout_list->id, layout_list));

}

void bj_layout_file::ParseLayout(const string &layout_name)
{
	string txtLayout = ReadTextFile("/home/workspace/bj_media/layout/" + layout_name);
	JsonObject json;
	json.Parse(txtLayout);
	if(bj_layout_file::layout_map.count(json["id"].ToString())>0) return;
	shared_ptr<bj_layout> layout=make_shared<bj_layout>();
	layout->mId=bj_layout::GetIntId();
	layout->id = json["id"].ToString();
	layout->name = (string) json["name"];
	layout->chEnSwitch = (string) json["chEnSwitch"];
	layout->resolution = (string) json["resolution"];
	layout->backColor = string_replace((string) json["backColor"],"#","");
	layout->backImage = (string) json["backImage"];
	layout->md5 = (string) json["md5"];
	layout->updateTime = (string) json["updateTime"];

	for (JsonObject &json_partition : json["partitions"])
	{
		shared_ptr<bj_partition> partition=make_shared<bj_partition>();
		partition->id = json_partition["id"];
		partition->mediaType = json_partition["mediaType"];
		partition->rect = json_partition["rect"].ToString();
		partition->backImage = json_partition["backImage"].ToString();
		partition->md5 = json_partition["md5"].ToString();
		partition->backColor = string_replace(json_partition["backColor"].ToString(),"#","");
		partition->zOrder = stoi(json_partition["zOrder"].ToString());

		partition->params = make_shared<bj_partition_media>();
		partition->params->media_id = bj_partition_media::GetNewMediaId();
		partition->params->partition_id = partition->id;
		partition->params->meida_type = (bj_media_type)partition->mediaType;
		partition->params->param = json_partition["params"];
		layout->partitions_map.insert(make_pair(partition->id,partition));
	}

	// 版式emer
	JsonObject &json_layout_emer = json["emer"];
	bj_layout_screen fullScreen;
	fullScreen.backImage = (json_layout_emer["fullScreen"])["backImage"].ToString();
	fullScreen.md5 = (json_layout_emer["fullScreen"])["md5"].ToString();
	fullScreen.backColor =string_replace( (json_layout_emer["fullScreen"])["backColor"].ToString(),"#","");
	fullScreen.emer_font.name = (json_layout_emer["fullScreen"])["font"]["name"].ToString();
	fullScreen.emer_font.size = (int)(json_layout_emer["fullScreen"])["font"]["size"];
	fullScreen.emer_font.bold = (bool)(json_layout_emer["fullScreen"])["font"]["bold"];
	fullScreen.emer_font.italic = (bool)(json_layout_emer["fullScreen"])["font"]["italic"];
	fullScreen.emer_font.textColor = (json_layout_emer["fullScreen"])["font"]["textColor"].ToString();
	fullScreen.emer_font.align = (int)(json_layout_emer["fullScreen"])["font"]["align"];
	fullScreen.emer_font.effect = (int)(json_layout_emer["fullScreen"])["font"]["effect"];
	fullScreen.speed = atoi((json_layout_emer["fullScreen"])["speed"].ToString().c_str());
	fullScreen.textDirection = atoi((json_layout_emer["fullScreen"])["textDirection"].ToString().c_str());

	bj_layout_screen halfScreen;
	halfScreen.rect = (json_layout_emer["halfScreen"])["rect"].ToString();
	halfScreen.backImage = (json_layout_emer["halfScreen"])["backImage"].ToString();
	halfScreen.md5 = (json_layout_emer["halfScreen"])["md5"].ToString();
	halfScreen.backColor = string_replace((json_layout_emer["halfScreen"])["backColor"].ToString(),"#","");
	halfScreen.emer_font.name = (json_layout_emer["halfScreen"])["font"]["name"].ToString();
	halfScreen.emer_font.size = (int)(json_layout_emer["halfScreen"])["font"]["size"];
	halfScreen.emer_font.bold = (bool)(json_layout_emer["halfScreen"])["font"]["bold"];
	halfScreen.emer_font.italic = (bool)(json_layout_emer["halfScreen"])["font"]["italic"];
	halfScreen.emer_font.textColor = (json_layout_emer["halfScreen"])["font"]["textColor"].ToString();
	halfScreen.emer_font.align = (int)(json_layout_emer["halfScreen"])["font"]["align"];
	halfScreen.emer_font.effect = (int)(json_layout_emer["halfScreen"])["font"]["effect"];
	halfScreen.speed = atoi((json_layout_emer["halfScreen"])["speed"].ToString().c_str());
	halfScreen.textDirection = atoi((json_layout_emer["halfScreen"])["textDirection"].ToString().c_str());

	bj_layout_emer emer;
	emer.fullScreen = fullScreen;
	emer.halfScreen = halfScreen;
	layout->emer = emer;
	bj_layout_file::layout_map.insert(make_pair(layout->id, layout));
}

/**
 * 版式列表控制
 * */
void bj_layout_file::Control(const string &plan_id, const string &cmd)
{
	// start(启用)、stop(停用) 、 revoke(撤销)
	if ("start" == cmd)
	{
	}
	else if ("stop" == cmd)
	{
		/* code */
	}
}

map<string,bj_layout_status> bj_layout_status::ALL;

void bj_layout_status::Add(const string& planid,int lvl)
{
	bj_layout_status layout;
	layout.planId=planid;
	layout.publishTime=datetime::Now().ToString("%Y%m%d %H%M%S");
	layout.level=lvl;
	layout.lastOp="下发";
	layout.lastOpTime=layout.publishTime;
	bj_layout_status::ALL.insert(make_pair(planid,layout));
	WriteToFile();
}
void bj_layout_status::Update(const string& planid,const string& publishtime,int lvl,const string& op)
{
	if(bj_layout_status::ALL.count(planid)==0)
	{
		bj_layout_status status;
		bj_layout_status::ALL.insert(make_pair(planid,status));
	}
	bj_layout_status::ALL[planid].publishTime=publishtime;
	bj_layout_status::ALL[planid].level=lvl;
	bj_layout_status::ALL[planid].lastOp=op;
	bj_layout_status::ALL[planid].lastOpTime=datetime::Now().ToString("%Y%m%d %H%M%S");
	WriteToFile();
}

void bj_layout_status::Delete(const string& planid)
{
	bj_layout_status::ALL.erase(planid);
	WriteToFile();
}

void bj_layout_status::WriteToFile()
{
	std::ofstream ofs("/home/workspace/bj_media/layout/layout_list.json");
	if(!ofs) return;
	JsonObject json;
	json.SetArray();
	int i=0;
	for(auto& kv : bj_layout_status::ALL)
	{
		JsonObject& item=json[i++];
		bj_layout_status& layout = kv.second;
		item["planId"]=layout.planId;
		item["publishTime"]=layout.publishTime;
		item["level"]=layout.level;
		item["lastOp"]=layout.lastOp;
		item["lastOpTime"]=layout.lastOpTime;
	}
	ofs<<json.ToString();
	ofs.close();
}

void bj_layout_status::ReadFromFile()
{
	JsonObject json;
	try
	{
		json.FromFile("/home/workspace/bj_media/layout/layout_list.json");
		bj_layout_status::ALL.clear();
		for(JsonObject& item : json)
		{
			bj_layout_status layout;
			layout.planId=(string)item["planId"];
			layout.publishTime=(string)item["publishTime"];
			layout.level=item["level"];
			layout.lastOp=(string)item["lastOp"];
			layout.lastOpTime=(string)item["lastOpTime"];
			bj_layout_status::ALL.insert(make_pair(layout.planId,layout));
		}
	}
	catch(...)
	{
		return;
	}
}
