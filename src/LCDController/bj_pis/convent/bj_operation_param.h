#ifndef BJ_OPERATION_PARAM_H_
#define BJ_OPERATION_PARAM_H_
#include <string>
#include <vector>
#include "bj_msg.h"

using namespace std;

class first_last_train
{
public:
    string direction_ch{"开往:"};//例 开往:
    string direction_en{"To:"};//例 To:
    string terminal_ch;//终点站 例 鼓楼
    string terminal_en;//终点站 例 gulou
    string first_ch{"首班"};//例 首班
    string first_en{"First Train"};//例 First Train
    string last_ch{"末班"};//例 末班
    string last_en{"Last Train"};//例 Last Train
    string first_time;//例 05:30:00
    string last_time;//例 23:15:00
    time_t update_time;//更新时间
};

class bj_operation_param
{
public:
	static bj_operation_param& Instance();
    string ats;//ATS服务地址，例udp://239.1.2.3:4567
    string start;//设备运营开始时间,例05:30:00
    string end;//设备运营结束时间,例23:00:00
    string ntp_server;//ntp服务器地址,例NTP://192.168.1.100
    string update_url;//软件更新地址,例FTP://192.168.1.23/softupdata
    vector<first_last_train> time_info;//列车首末班
    int Parse(const char* data);
	int Write(char* data);
	static void SetOperationParam(bj_msg *msg, ServerTcpConnection *conn);
};

#endif // BJ_OPERATION_PARAM_H_
