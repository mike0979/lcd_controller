#ifndef BJ_USER_INFO_H
#define BJ_USER_INFO_H
#include <string>
#include <vector>
#include <memory>

using namespace std;

class bj_user_info
{
public:
	string user_code;
	string login_account;
	string name;
	string password;
	string department;
	string telephone;
	string email;
	int status;//0正常1冻结2无效
	string permission;//1设备开关,2信息发布取消,3日志操作
	time_t update_time;

	string station_code;

	int Parse(const char* data);
	int Write(char* data);

	static vector<shared_ptr<bj_user_info>> sc_users;
	static vector<shared_ptr<bj_user_info>> GetUsers(const string& station_code);
	
};

#endif // BJ_USER_INFO_H
