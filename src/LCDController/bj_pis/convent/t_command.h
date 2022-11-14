#ifndef T_COMMAND_H_
#define T_COMMAND_H_

#include "bj_msg.h"
#include "bj_pis/utils/datetime.h"
#include "hd_device_status.h"
#include "tcp_msg_help.h"

//适用于 T11,T12,T13,T21,T22,T23
static void SendTCommand(const string &command, const string &device_code, ServerTcpConnection *conn)
{
    bj_msg tmsg;
    tmsg.SQN = bj_msg::GetSendSQN();
    tmsg.Version = "1.0.2";
    tmsg.SystemCode = "10";
    tmsg.TimeStamp = datetime::Now().get_time();
    tmsg.Command = command;
    //tmsg.SetData(device_code.c_str(), device_code.size());
    tmsg.SetData(device_code.c_str(), 14);

    char data[128];
    int len = tmsg.Write(data);
    conn->Send(data, len);
}

static void SendTDeviceStatus(ServerTcpConnection *conn)
{
    st_subscriber suber;
    suber.reply.SQN = bj_msg::GetSendSQN();
    suber.reply.Version = "1.0.2";
    suber.reply.SystemCode = "10";
    suber.reply.TimeStamp = datetime::Now().get_time();
    suber.reply.Command = "T31";
    suber.conn=conn;
    hd_device_status::SendStatus(suber);
}

//status 0未下载 1下载中 2下载完毕 3下载失败
//percent 0无意义 1百分比 2完毕 3失败代号
static void SendDownloadReport(const string& taskId, int status, ServerTcpConnection *conn, int percent)
{
    bj_msg tmsg;
    tmsg.SQN = bj_msg::GetSendSQN();
    tmsg.Version = "1.0.2";
    tmsg.SystemCode = "10";
    tmsg.TimeStamp = datetime::Now().get_time();
    tmsg.Command = "T35";
    tmsg.SetData("", 18);
    int index = 0;
    TcpSetString(tmsg.Data, index, taskId, 14);
    TcpSetInt(tmsg.Data, index, status, 1);
    TcpSetInt(tmsg.Data, index, percent, 3);
    
    char data[128];
    int len = tmsg.Write(data);
    conn->Send(data, len);
}

#endif // T_COMMAND_H_
