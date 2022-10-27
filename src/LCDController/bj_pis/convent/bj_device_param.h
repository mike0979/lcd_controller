#ifndef BJ_DEVICE_PARAM_H
#define BJ_DEVICE_PARAM_H
#include <vector>
#include <memory>
#include <string>
#include "bj_msg.h"

using namespace std;

class bj_device_param
{	
public:
	bj_device_param(){}
	string code;
	string type;
	string desc;
	string ip;
	string mac;
	string area{"16"};
	string platform;
	time_t update_time;

	static bj_device_param& Instance();
	int Parse(const char* data);
	int Write(char* data);
	static void SetDeviceParam(bj_msg *msg, ServerTcpConnection *conn);
	static void SetScParam(bj_msg *msg, ServerTcpConnection *conn);

	static vector<shared_ptr<bj_device_param>> sc_params;
	static string station_start;
	static string station_end;
};

#endif // BJ_DEVICE_PARAM_H
