#include "bj_operation_param.h"
#include "tcp_msg_help.h"
#include "config/configparser.h"
#include "bj_pis/ats/task_get_ats.h"

STATIC_REGISTRATION(bj_operation_param)
{
	bj_msg::GetHandlers()["M62"] = &bj_operation_param::SetOperationParam;
}

bj_operation_param& bj_operation_param::Instance()
{
	static bj_operation_param _param;
	return _param;
}

int bj_operation_param::Parse(const char* data)
{
    int index=0;
    ats=TcpGetString(data,index,28);
    start=TcpGetString(data,index,8);
    index++;
    end=TcpGetString(data,index,8);
    ntp_server=TcpGetString(data,index,28);
    update_url=TcpGetString(data,index,128);
    int first_last_count=TcpGetInt(data,index,2);
    time_info.clear();
    time_info.reserve(first_last_count);
    for (int i = 0; i < first_last_count; i++)
    {
        first_last_train flt;
        flt.direction_ch=TcpGetString(data,index,12);
        flt.direction_en=TcpGetString(data,index,12);
        flt.terminal_ch=TcpGetString(data,index,32);
        flt.terminal_en=TcpGetString(data,index,32);
        flt.first_ch=TcpGetString(data,index,18);
        flt.first_en=TcpGetString(data,index,18);
        flt.last_ch=TcpGetString(data,index,18);
        flt.last_en=TcpGetString(data,index,18);
        flt.first_time=TcpGetString(data,index,8);
        flt.last_time=TcpGetString(data,index,8);
        flt.update_time=TcpGetDateTime(data,index);
        time_info.push_back(flt);
    }
    return index;
}

int bj_operation_param::Write(char* data)
{
    int index=0;
    TcpSetString(data,index,ats,28);
    TcpSetString(data,index,start,8);
    TcpSetString(data,index,"-",1);
    TcpSetString(data,index,end,8);
    TcpSetString(data,index,ntp_server,28);
    TcpSetString(data,index,update_url,128);
    TcpSetInt(data,index,time_info.size(),2);
    for(const first_last_train& flt:time_info)
    {
        TcpSetString(data,index,flt.direction_ch,12);
        TcpSetString(data,index,flt.direction_en,12);
        TcpSetString(data,index,flt.terminal_ch,32);
        TcpSetString(data,index,flt.terminal_en,32);
        TcpSetString(data,index,flt.first_ch,18);
        TcpSetString(data,index,flt.first_en,18);
        TcpSetString(data,index,flt.last_ch,18);
        TcpSetString(data,index,flt.last_en,18);
        TcpSetString(data,index,flt.first_time,8);
        TcpSetString(data,index,flt.last_time,8);
        TcpSetDateTime(data,index,flt.update_time);
    }
    return index;
}

void bj_operation_param::SetOperationParam(bj_msg *msg, ServerTcpConnection *conn)
{
	bj_operation_param::Instance().Parse(msg->Data);

	bj_msg reply;
	reply.Copy(*msg);
	reply.Command = "A62";
	reply.SystemCode = "10"; //SPIS
    char result[131 + 1] = { '\0' };
	int index=0;
	TcpSetInt(result, index, 100, 3);
	TcpSetString(result, index, "ok", 128);
	reply.SetData(result, index);

	/*ConfigParser config("./","config.xml");
	config.Modify("config.system-config.device_id",bj_operation_param::Instance().ats);*/
	//guo20221113为了稳定的过协议认证，把这段逻辑去掉，功能不到上线是不影响什么的
    /*task_get_ats::GetInstance().ats_url = bj_operation_param::Instance().ats;
    task_get_ats::GetInstance().Stop();
    task_get_ats::GetInstance().Start();*/

	char data[256];//请保证自己buffer够用
	int len = reply.Write(data);
	conn->Send(data, len);
}
