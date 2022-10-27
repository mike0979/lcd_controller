#ifndef BJ_ATS_INFO_H_
#define BJ_ATS_INFO_H_
#include <vector>
#include <map>
#include <string>
#include <memory>
#include "bj_pis/utils/read_write_lock.h"

using namespace std;

class bj_ats_info
{
public:
	map<string,string> station_info;
	string GetValue(const string& var, int index=0);
	vector<map<string,string>> trains_info;
	void DealATS(const char* s, int l);
	static bj_ats_info& Instance();
private:
	bj_ats_info(){};
	ReadWriteLock rwlock_trains;
};

#endif /* BJ_ATS_INFO_H_ */
