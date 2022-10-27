#ifndef BJ_PLAN_INFO_H_
#define BJ_PLAN_INFO_H_

#include <string>
#include <vector>
#include "bj_file_info.h"
#include <memory>

using namespace std;

class bj_plan_info
{
public:	
	string PlanName;
	string PlanID;
	time_t UpdateTime;
	int FileCount;
	vector<shared_ptr<bj_file_info>> Files;

	void Parse(const char* data);
	int Write(char* data);
};

#endif // BJ_PLAYLIST_INFO_H_
