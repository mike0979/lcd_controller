#include "bj_playlist_info.h"
#include <fstream>
#include "bj_pis/utils/datetime.h"
#include "bj_pis/utils/JsonObject.h"

map<string,shared_ptr<bj_playlist_info>> bj_playlist_info::PlayLists;

bj_playlist_info::bj_playlist_info()
{
	Enabled=true;
	Level=0;
}

map<string,bj_playlist_status> bj_playlist_status::ALL;

void bj_playlist_status::Add(const string& id)
{
	bj_playlist_status status;
	status.ID=id;
	status.Enabled=true;
	status.PublishTime=datetime::Now().ToString("%Y%m%d %H%M%S");
	status.lastOp="下发";
	status.lastOpTime=status.PublishTime;
	bj_playlist_status::ALL[id] = status;
	WriteToFile();
}

void bj_playlist_status::Update(const string& id, bool enable, const string& publishTime, const string& op)
{
	if(bj_playlist_status::ALL.count(id)==0)
	{
		bj_playlist_status ps;
		bj_playlist_status::ALL.insert(make_pair(id,ps));
	}
	bj_playlist_status& status=bj_playlist_status::ALL[id];
	status.Enabled=enable;
	status.PublishTime=publishTime;
	status.lastOp=op;
	status.lastOpTime=datetime::Now().ToString("%Y%m%d %H%M%S");
	WriteToFile();
}

void bj_playlist_status::Delete(const string& id)
{
	bj_playlist_status::ALL.erase(id);
	WriteToFile();
}

void bj_playlist_status::WriteToFile()
{
	std::ofstream ofs("/home/workspace/bj_media/plan/allplaylist.json");
	if(!ofs) return;
	JsonObject json;
	json.SetArray();
	int i=0;
	for(auto& kv : bj_playlist_status::ALL)
	{
		JsonObject& item=json[i++];
		bj_playlist_status& status = kv.second;
		item["planId"]=status.ID;
		item["publishTime"]=status.PublishTime;
		item["Enabled"]=status.Enabled;
		item["lastOp"]=status.lastOp;
		item["lastOpTime"]=status.lastOpTime;
	}
	ofs<<json.ToString();
	ofs.close();
}

void bj_playlist_status::ReadFromFile()
{
	JsonObject json;
		try
		{
			json.FromFile("/home/workspace/bj_media/plan/allplaylist.json");
			bj_playlist_status::ALL.clear();
			for(JsonObject& item : json)
			{
				bj_playlist_status status;
				status.ID=(string)item["planId"];
				status.PublishTime=(string)item["publishTime"];
				status.Enabled=item["Enabled"];
				status.lastOp=(string)item["lastOp"];
				status.lastOpTime=(string)item["lastOpTime"];
				bj_playlist_status::ALL.insert(make_pair(status.ID,status));
			}
		}
		catch(...)
		{
			return;
		}
}
