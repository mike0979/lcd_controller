#include "hd_device_status.h"
#include "tcp_msg_help.h"
#include "bj_pis/utils/diagnostic.h"
#include <ftptrans.h>
#include "t_command.h"
#include <thread>
#include "Log.h"
#include "config/configparser.h"
#include <thread>

//一个cpp里只能写一次，这个会在程序开始时运行。比main函数还早
STATIC_REGISTRATION(hd_device_status)
{
    bj_msg::GetHandlers()["M31"] = &hd_device_status::OnHeartBeat;
    bj_msg::GetHandlers()["M32"] = &hd_device_status::OnSubscribeStatus;
    bj_msg::GetHandlers()["M37"] = &hd_device_status::OnSubscribeLog;
    bj_msg::GetHandlers()["connected"] = &hd_device_status::OnConnected;
    bj_msg::GetHandlers()["disconnected"] = &hd_device_status::OnDisConnected;
}

vector<st_subscriber> hd_device_status::StatusSubers;
vector<st_subscriber> hd_device_status::LogSubers;
const char* hd_device_status::TAG="hd_device_status";
static thread* _thread_bj_sub=NULL;
void hd_device_status::OnHeartBeat(bj_msg *msg, ServerTcpConnection *conn)
{
    bj_msg reply;
    reply.Copy(*msg);
    reply.Command = "A31";
    reply.SystemCode = "10"; //SPIS
    char result[3] = { '\0' };
    result[0] = '0';
    result[1] = '1';
    reply.SetData(result, 3);

    char data[256];//请保证自己buffer够用
    int len = reply.Write(data);
    conn->Send(data, len);
}

void hd_device_status::OnSubscribeStatus(bj_msg *msg, ServerTcpConnection *conn)
{
    int index = 0;
    st_subscriber suber;
    suber.device_code = TcpGetString(msg->Data, index, msg->DataLength-4);
    suber.duration = TcpGetInt(msg->Data, index, 4);
    suber.remote = conn->RemoteEndpoint();
    suber.reply.Copy(*msg);
    suber.reply.Command = "A32";
    suber.reply.SystemCode = "10"; //SPIS
    suber.last=time(0)-suber.duration-1;
    suber.conn=conn;

    //清除状态订阅
	for (std::vector<st_subscriber>::iterator iter = StatusSubers.begin(); iter != StatusSubers.end(); ++iter)
	{
		if(conn->RemoteEndpoint() == iter->remote)
		{
			StatusSubers.erase(iter);
			break;
		}
	}
    StatusSubers.push_back(suber);
}

void hd_device_status::SendStatus(st_subscriber &suber)
{
    thread t([](st_subscriber tsuber,ServerTcpConnection *tconn){
        int index = 0;
        char status[1024];
        if(tsuber.reply.Command == "T31")
        {
            TcpSetDateTime(status, index, datetime::Now().get_time(), 14); //任务 ID
            tsuber.reply.SQN = bj_msg::GetSendSQN();
        }
        TcpSetDateTime(status, index, datetime::Now().get_time(), 14); //tag
        TcpSetString(status, index, "DT13", 4); //车站编码
        TcpSetString(status, index, "0001", 4); //设备数量
        {
            TcpSetString(status, index, ConfigParser::Instance().mDeviceId, 14); //设备编号
            TcpSetString(status, index, "006", 3); //采集点数量
            {
            	//通信状态
					TcpSetString(status, index, "001", 3); //采集点类型
					TcpSetString(status, index, "01", 2); //采集点子项数量
					TcpSetStringGB2312(status, index, "通信状态", 20); //采集子项点描述
					TcpSetString(status, index, "2", 1); //采集点状态数据类型
					TcpSetString(status, index, "1", 1); //采集点状态数据值
					//操作系统版本
					TcpSetString(status, index, "003", 3); //采集点类型
					TcpSetString(status, index, "01", 2); //采集点子项数量
					TcpSetStringGB2312(status, index, "操作系统版本", 20); //采集子项点描述
					TcpSetString(status, index, "3", 1); //采集点状态数据类型
					TcpSetString(status, index, Diagnostic::Instance().GetServerVersion(), 256); //采集点状态数据值
                //CPU基本信息
                TcpSetString(status, index, "004", 3); //采集点类型
                TcpSetString(status, index, "01", 2); //采集点子项数量
                TcpSetStringGB2312(status, index, "CPU基本信息", 20); //采集子项点描述
                TcpSetString(status, index, "3", 1); //采集点状态数据类型
                TcpSetString(status, index, Diagnostic::Instance().GetCpuVersion(), 256); //采集点状态数据值
                //CPU百分比
                TcpSetString(status, index, "006", 3); //采集点类型
                TcpSetString(status, index, "01", 2); //采集点子项数量
                TcpSetStringGB2312(status, index, "CPU百分比", 20); //采集子项点描述
                TcpSetString(status, index, "1", 1); //采集点状态数据类型
                double cpu_percent = Diagnostic::Instance().GetCPUUsage();
                TcpSetInt(status, index, (int)(cpu_percent * 100 + 0.5), 8); //采集点状态数据值
                //内存总容量
                TcpSetString(status, index, "007", 3); //采集点类型
                TcpSetString(status, index, "01", 2); //采集点子项数量
                TcpSetStringGB2312(status, index, "内存总容量", 20); //采集子项点描述
                TcpSetString(status, index, "1", 1); //采集点状态数据类型
                TcpSetInt(status, index, (int)(Diagnostic::Instance().GetMemTotal() / 1024), 8); //采集点状态数据值
                //内存占用百分比
                TcpSetString(status, index, "008", 3); //采集点类型
                TcpSetString(status, index, "01", 2); //采集点子项数量
                TcpSetStringGB2312(status, index, "内存占用百分比", 20); //采集子项点描述
                TcpSetString(status, index, "1", 1); //采集点状态数据类型
                double mem_use = Diagnostic::Instance().GetMemUsage();
                TcpSetInt(status, index, (int)(mem_use * 100 + 0.5), 8); //采集点状态数据值
            }
        }
        tsuber.reply.TimeStamp = datetime::Now().get_time();
        tsuber.reply.SetData(status, index);
        char data[1024];//请保证自己buffer够用
        bj_msg& msg = tsuber.reply;
        int len = msg.Write(data);
        tconn->Send(data, len);
        //tsuber.last=time(0);
    },suber,suber.conn);
    suber.last=time(0);
    t.detach();
}


void hd_device_status::OnSubscribeLog(bj_msg *msg, ServerTcpConnection *conn)
{
    int index = 0;
    st_subscriber suber;
    suber.device_code = TcpGetString(msg->Data, index, 14);
    suber.log_path = TcpGetString(msg->Data, index, 128);
    suber.duration = TcpGetInt(msg->Data, index, 4);
    suber.remote = conn->RemoteEndpoint();

    suber.reply.Copy(*msg);
    suber.reply.Command = "A37";
    suber.reply.SystemCode = "10"; //SPIS
    suber.last=time(0)-suber.duration-1;
    suber.conn=conn;
	//清除日志订阅
	for (std::vector<st_subscriber>::iterator iter = LogSubers.begin(); iter != LogSubers.end(); ++iter)
	{
		if(conn->RemoteEndpoint() == iter->remote)
		{
			iter = LogSubers.erase(iter);
			break;
		}
	}
    LogSubers.push_back(suber);
}

void hd_device_status::SendLog(st_subscriber &suber)
{
    string ftpurl = suber.log_path;
    //ftp://tccpis:123@10.255.170.223/PlanFormat
    string userAndPwd="anonymous:";
    	string url="";
    	if (ftpurl.find('@') != string::npos)
    	{
    		int indexUser = ftpurl.find("//") + 2;
    		userAndPwd = ftpurl.substr(indexUser, ftpurl.find('@', indexUser) - indexUser);
    		ftpurl = string("ftp://") + ftpurl.substr(ftpurl.find('@', indexUser) + 1);
    	}

    ftpurl.erase(0, 6);
    size_t idx = ftpurl.find('/');
    string localDir = "/home/log/lcd_player/";
    string logFile = GetLatestFile(localDir);
    LogE("upload %s%s to %s\n",localDir.c_str(),logFile.c_str(),suber.log_path.c_str());
    ftptrans ftp;
    ftp.SetURL(ftpurl.substr(0, idx), ftpurl.substr(idx + 1));
    ftp.SetConnectTimeout(5000);
    ftp.SetLocalPath(localDir);
    ftp.SetUserPassword(userAndPwd.substr(0,userAndPwd.find(':')),userAndPwd.substr(userAndPwd.find(':')+1));
    double d;
    int r = ftp.UploadFile(logFile, logFile, d);
    if(r == FTP_TRANS_Result::OK)
    {
        char log_buff[259];
        int index = 0;
        TcpSetString(log_buff, index, logFile, 128);
        TcpSetString(log_buff, index, "100", 3);
        TcpSetString(log_buff, index, "ok", 128);
        suber.reply.SetData(log_buff, index);
        suber.reply.SQN = bj_msg::GetSendSQN();
        suber.reply.TimeStamp = datetime::Now().get_time();

        char data[1024];//请保证自己buffer够用
        int len = suber.reply.Write(data);
        suber.conn->Send(data, len);
        suber.last=time(0);
    }
    else
    {
        LogD("failed\n");
    }
}

void hd_device_status::OnConnected(bj_msg *msg, ServerTcpConnection *conn)
{
	if(_thread_bj_sub==NULL){
		_thread_bj_sub=new thread([](){
			while(true){
				time_t now = time(0);

				//发送设备运行状态报告
				for (std::vector<st_subscriber>::iterator iter = StatusSubers.begin(); iter != StatusSubers.end(); ++iter)
				{
					int time = now - iter->last;
					if(time >= (iter->duration))
					{
						SendStatus(*iter);
					}
				}

				//发送日志报告
				for (std::vector<st_subscriber>::iterator iter = LogSubers.begin(); iter != LogSubers.end(); ++iter)
				{
					int time = now - iter->last;
					if(time >= (iter->duration))
					{
						SendLog(*iter);
					}
				}
				sleep(1);
			}
		});
		_thread_bj_sub->detach();
	}
    string deviceCode = ConfigParser("./","config.xml").mDeviceId;
    SendTCommand("T11", deviceCode , conn);
    SendTCommand("T12", deviceCode, conn);
    SendTCommand("T13", deviceCode, conn);
    SendTCommand("T21", deviceCode, conn);
    SendTCommand("T23", deviceCode, conn);
    SendTCommand("T24", deviceCode, conn);
    //SendTDeviceStatus(conn);
    /*thread _threadT31([conn](){
    	while(conn->State() == waxberry::CONNECTION_STATE::CON_STATE_CONNECTED)
		{
    		sleep(2);
			SendTDeviceStatus(conn);
		}
    });
    _threadT31.detach();*/
}

void hd_device_status::OnDisConnected(bj_msg *msg, ServerTcpConnection *conn)
{
    //清除状态订阅
    for (std::vector<st_subscriber>::iterator iter = StatusSubers.begin(); iter != StatusSubers.end(); ++iter)
    {
        if(conn->RemoteEndpoint() == iter->remote)
        {
            printf("StatusSuber %s:%d remove\n",iter->remote.address().to_string().c_str(),iter->remote.port());
            StatusSubers.erase(iter);
            break;
        }
    }
    //清除日志订阅
    for (std::vector<st_subscriber>::iterator iter = LogSubers.begin(); iter != LogSubers.end(); ++iter)
    {
        if(conn->RemoteEndpoint() == iter->remote)
        {
            printf("LogSuber %s:%d remove\n",iter->remote.address().to_string().c_str(),iter->remote.port());
            iter = LogSubers.erase(iter);
            break;
        }
    }
}
