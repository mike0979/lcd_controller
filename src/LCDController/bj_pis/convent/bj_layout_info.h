#ifndef BJ_LAYOUT_INFO_H_
#define BJ_LAYOUT_INFO_H_
#include <string>
#include <vector>
#include "bj_pis/tcp_server/server_tcp_connection.h"
#include "bj_plan_info.h"
#include "bj_pis/utils/JsonObject.h"
#include <map>
#include "bj_media_info.h"
#include <memory>

class bj_layout_info
{
public:
	string PlanName;
	string PlanID;
	time_t UpdateTime;
	vector<shared_ptr<bj_file_info>> Files;

	void Parse(const char *data);
	int Write(char *data);
};

/**
 * 
 * */
struct bj_partition
{
	int id;
	int mediaType;
	string rect;
	string backImage;
	string md5;
	string backColor;
	int zOrder;
	shared_ptr<bj_partition_media> params;
};

//-----------------
struct bj_emer_font
{
	string name;
	int size;
	bool bold;
	bool italic;
	string textColor;
	int align;
	int effect;
};

struct bj_layout_screen
{
	string rect;
	string backImage;
	string md5;
	string backColor;
	bj_emer_font emer_font;
	int speed;
	int textDirection;
};

struct bj_layout_emer
{
	bj_layout_screen fullScreen;
	bj_layout_screen halfScreen;
};

struct bj_layout
{
	int mId;
	string id;
	string name;
	string chEnSwitch;
	string resolution;
	string backColor;
	string backImage;
	string md5;
	string updateTime;
	map<int, shared_ptr<bj_partition>> partitions_map;
	bj_layout_emer emer;
	static int GetIntId(){return ++s_id;}
private:
	static int s_id;
};

struct bj_layout_list_layouts
{
	/* data */
	string id;
	string name;
	string updateTime;
	string startTime;
	string endTime;
	string weekFlag;
};

struct bj_layout_list
{
	int mId;
	string id;
	string name;
	string updateTime;
	string startTime;
	string endTime;
	string weekFlag;
	string resolution;
	string level;
	bool enabled;
	map<string, shared_ptr<bj_layout_list_layouts>> layout_list_layouts;
	static int GetIntId(){return ++s_id;}
private:
	static int s_id;
};

class bj_layout_status
{
public:
	string planId;//ID
	string publishTime;//发布时间
	int level;//优先级
	string lastOp;//最后的操作
	string lastOpTime;//最后操作时间
	static void Add(const string& planid,int lvl);
	static void Update(const string& planid,const string& publishtime,int lvl,const string& op);
	static void Delete(const string& planid);
	static map<string,bj_layout_status> ALL;
	static void WriteToFile();
	static void ReadFromFile();
};

class bj_layout_file
{
public:
	static map<string, shared_ptr<bj_layout_list>> layout_list_map; //保持版式列表
	static map<string, shared_ptr<bj_layout>> layout_map;           //版式map

	static void ParseLayoutList(const string &layout_group_name);
	static void ParseLayout(const string &layout_name);
	static void Control(const string &plan_id, const string &cmd);
};

#endif
