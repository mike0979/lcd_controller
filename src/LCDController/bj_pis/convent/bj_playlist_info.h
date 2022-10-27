#ifndef BJ_PLAYLIST_INFO_H_
#define BJ_PLAYLIST_INFO_H_

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <set>
#include "json/ScheduleObjs.h"

using namespace std;

class pl_media_info
{
public:
	int partitionID;
	string StartTime;
	string EndTime;
	Json::MediaBasic* media;
};

class bj_playlist_info
{
public:
	bj_playlist_info();
	string ID;
	string Name;
	string UpdateTime;
	string StartTime;
	string EndTime;
	string WeekFlag;
	int Level;
	bool Enabled;
	string PublishTime;
	string Md5;

	//map<media_type,vector<版式媒體信息>>
	std::map<int,vector<shared_ptr<pl_media_info>>> MediaList;
	//planId,plan info
	static map<string,shared_ptr<bj_playlist_info>> PlayLists;

};

class bj_playlist_status
{
public:
	string ID;
	bool Enabled;
	string PublishTime;
	string lastOp;
	string lastOpTime;
	//planId,bj_playlist_status
	static map<string,bj_playlist_status> ALL;
	static void Add(const string& id);
	static void Update(const string& id, bool enable, const string& publishTime, const string& op);
	static void Delete(const string& id);
	static void WriteToFile();
	static void ReadFromFile();
};

#endif // BJ_PLAYLIST_INFO_H_
