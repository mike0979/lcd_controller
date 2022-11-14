#include "bj_device_param.h"
#include "tcp_msg_help.h"
#include "config/configparser.h"
#include "t_command.h"
#include "bj_user_info.h"

STATIC_REGISTRATION(bj_device_param)
{
	bj_msg::GetHandlers()["M61"] = &bj_device_param::SetDeviceParam;
	bj_msg::GetHandlers()["M65"] = &bj_device_param::SetScParam;
}

vector<shared_ptr<bj_device_param>> bj_device_param::sc_params;
string bj_device_param::station_start=string();
string bj_device_param::station_end=string();

bj_device_param& bj_device_param::Instance()
{
	static bj_device_param s_param;
	return s_param;
}

int bj_device_param::Parse(const char* data)
{
	int index = 0;
    code = TcpGetString(data, index, 14);
    type = TcpGetString(data, index, 2);
    desc = TcpGetStringGB2312(data, index, 33);
    ip = TcpGetString(data, index, 26);
    mac = TcpGetString(data, index, 18);
    area = TcpGetString(data, index, 2);
    platform = TcpGetString(data, index, 4);
    update_time = TcpGetDateTime(data, index);
    return index;
}

int bj_device_param::Write(char* data)
{
	int index = 0;
    TcpSetString(data, index, code, 14);
    TcpSetString(data, index, type, 2);
    TcpSetString(data, index, desc, 33);
    TcpSetString(data, index, ip, 26);
    TcpSetString(data, index, mac, 18);
    TcpSetInt(data, index, stoi(area), 2);
    TcpSetString(data, index, platform, 4);
    TcpSetDateTime(data, index, update_time);
    return index;
}

void bj_device_param::SetDeviceParam(bj_msg *msg, ServerTcpConnection *conn)
{
	bj_device_param::Instance().Parse(msg->Data);
	bj_msg reply;
	reply.Copy(*msg);
	reply.Command = "A61";
	reply.SystemCode = "10"; //SPIS
	char result[131 + 1] = { '\0' };
	int index=0;
	TcpSetInt(result, index, 100, 3);
	TcpSetString(result, index, "ok", 128);
	reply.SetData(result, index);

	//为了协议检测不影响功能检测，将修改配置文件的功能去掉
	/*ConfigParser config("./","config.xml");
	config.Modify("config.system-config.device_id",bj_device_param::Instance().code);
	config.mDeviceId=bj_device_param::Instance().code;*/

	char data[256];//请保证自己buffer够用
	int len = reply.Write(data);
	conn->Send(data, len);

	/*SendTCommand("T23", bj_device_param::Instance().code, conn);
	SendTCommand("T24", bj_device_param::Instance().code, conn);
	SendTCommand("T12", bj_device_param::Instance().code, conn);
	SendTCommand("T13", bj_device_param::Instance().code, conn);
	SendTCommand("T21", bj_device_param::Instance().code, conn);*/
}

void bj_device_param::SetScParam(bj_msg *msg, ServerTcpConnection *conn)
{
	int index=0;
	int device_count=TcpGetInt(msg->Data,index,3);//
	for (int i = 0; i < device_count; ++i) {
		shared_ptr<bj_device_param> pDevice=make_shared<bj_device_param>();
		index+=pDevice->Parse(msg->Data+index);
		sc_params.push_back(pDevice);
	}
	int user_count=TcpGetInt(msg->Data,index,3);
	for (int i = 0; i < user_count; ++i) {
		shared_ptr<bj_user_info> pUser=make_shared<bj_user_info>();
		index+=pUser->Parse(msg->Data+index);
		bj_user_info::sc_users.push_back(pUser);
	}
	station_start=TcpGetString(msg->Data,index,8);
	index++;
	station_end=TcpGetString(msg->Data,index,8);

	bj_msg reply;
	reply.Copy(*msg);
	reply.Command = "A65";
	reply.SystemCode = "10"; //SPIS
	char result[131 + 1] = { '\0' };

	index=0;
	TcpSetInt(result, index, 100, 3);
	TcpSetString(result, index, "ok", 128);
	reply.SetData(result, index);

	char data[256];//请保证自己buffer够用
	int len = reply.Write(data);
	conn->Send(data, len);
}
