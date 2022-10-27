#include "hd_monitor.h"
#include "tcp_msg_help.h"

//一个cpp里只能写一次，这个会在程序开始时运行。比main函数还早
STATIC_REGISTRATION(hd_monitor)
{
    bj_msg::GetHandlers()["M71"] = &hd_monitor::OnMonitor;
}

void hd_monitor::OnMonitor(bj_msg *msg, ServerTcpConnection *conn)
{
    int index = 0;
    int flag = TcpGetInt(msg->Data, index, 1); //1视频2图片
    string path = TcpGetString(msg->Data, index, 32);

    bj_msg reply;
    reply.Copy(*msg);
    reply.Command = "A71";
    reply.SystemCode = "10"; //SPIS

    reply.SetData("", 33);
    index = 0;
    TcpSetInt(reply.Data, index, flag, 1);
    if(flag == 1)
    {
        TcpSetString(reply.Data, index, "rtsp://10.255.170.221/0", 32);
    }
    else
    {
        string file = GetLatestFile("/home/workspace/log/screenshot/");
        TcpSetString(reply.Data, index, "ftp://192.168.1.1/screenshot/" + file, 32);
    }
    
    char data[256];//请保证自己buffer够用
    int len = reply.Write(data);
    conn->Send(data, len);
}