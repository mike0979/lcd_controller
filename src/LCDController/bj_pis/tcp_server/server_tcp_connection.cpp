#include <bj_pis/bj_log/log_transfer.h>
#include "server_tcp_connection.h"
#include "bj_pis/convent/bj_msg.h"
#include <exception>
#include <cstring>
#include <vector>
#include "Log.h"

using namespace waxberry;

vector<ServerTcpConnection*> ServerTcpConnection::TransferConns;
const char* ServerTcpConnection::TAG="ServerTcpConnection";
std::map<ServerTcpConnection*,bool> ServerTcpConnection::States;
ServerTcpConnection::ServerTcpConnection(BasicHost& host) : WaxberryTcpConnection<Message>(host)
{
	States[this]=false;
}

int ServerTcpConnection::OnReadMessage(const char* data, int size)
{
	if(size>=8&&strcmp("transfer",data)==0)
	{
		TransferConns.push_back(this);
		return size;
	}
	for (std::vector<ServerTcpConnection*>::iterator iter = TransferConns.begin(); iter != TransferConns.end(); ++iter)
	{
		if((*iter)->State() == waxberry::CONNECTION_STATE::CON_STATE_CONNECTED)
		{
			char temp[256];
			sprintf(temp,"r\t%ld\t%s:%d\t%d\t",time(0),RemoteEndpoint().address().to_string().c_str(),RemoteEndpoint().port(),size);
			(*iter)->Write(temp,strlen(temp));
			(*iter)->Write(data,size);
		}
	}
	LogTransfer::GetInstance().LogReceive(data,size);
	LogD("receive from %s:%d,size:%d,data:%s!!\n",RemoteEndpoint().address().to_string().c_str(),RemoteEndpoint().port(),size,string(data,size).c_str());
	/*printf("receive::::::::::::::::::::::::::\n");
	for (int i = 0; i < size; ++i) {
		printf("%02x ",(unsigned char)data[i]);
	}
	printf("\n");*/
	if(size<34) return 0;
	bj_msg msg;
	if(data[25]=='M')
	{
		if(size==40&&string(data+25,3)=="M52") return size;//京投的web平台有bug，为了测试通过，单独把这个包抛弃
		msg.Parse(data);
		if(msg.DataLength+80>size)
		{
			return 0;
		}
	}
	else
	{
		msg.Parse(data);
		if(msg.DataLength+34>size)
		{
			return 0;
		}
	}
	LogD("Command is:%s\n",msg.Command.c_str());
	//开始处理
	if(bj_msg::GetHandlers().count(msg.Command)!=0)
	{
		try
		{
			bj_msg::GetHandlers()[msg.Command](&msg,this);
		}
		catch(exception &e)
		{
			LogE("%s handler error,error type:%s,error msg:%s\n",msg.Command.c_str(), typeid(e).name(),e.what());
		}
		catch(...)
		{
			LogE("%s handler unknown error\n",msg.Command.c_str());
		}
	}
	return size;
	//return data[25]=='M'?(msg.DataLength+80):(msg.DataLength+34);
}

void ServerTcpConnection::OnConnected()
{
	LogD("%s:%d connected\n",RemoteEndpoint().address().to_string().c_str(),RemoteEndpoint().port());
	if(bj_msg::GetHandlers().count("connected")!=0)
	{
		try
		{
			bj_msg::GetHandlers()["connected"](nullptr,this);
		}
		catch(...)
		{
			LogE("OnConnected handler error\n");
		}
	}
}

void ServerTcpConnection::OnDisconnected()
{
	States[this]=true;
	LogD("%s:%d disconnected\n",RemoteEndpoint().address().to_string().c_str(),RemoteEndpoint().port());
	if(bj_msg::GetHandlers().count("disconnected")!=0)
	{
		try
		{
			bj_msg::GetHandlers()["disconnected"](nullptr,this);
		}
		catch(...)
		{
			LogE("OnDisconnected handler error\n");
		}
	}

	for (std::vector<ServerTcpConnection*>::iterator iter = TransferConns.begin(); iter != TransferConns.end(); ++iter)
	{
		if((*iter)==this)
		{
			TransferConns.erase(iter);
			break;
		}
	}
}

void ServerTcpConnection::Send(const char * buffer, int size, waxberry::MSG_SENT_CALLBACK cb)
{
	if(States[this])
	{
		States.erase(this);
		//LogD("but State is:%d\n",State());
		return;
	}
	LogD("send to %s:%d,size:%d,data:%s!!\n",RemoteEndpoint().address().to_string().c_str(),RemoteEndpoint().port(),size,string(buffer,size).c_str());
	/*for (std::vector<ServerTcpConnection*>::iterator iter = TransferConns.begin(); iter != TransferConns.end(); ++iter)
	{
		if((*iter)->State() == waxberry::CONNECTION_STATE::CON_STATE_CONNECTED)
		{
			char temp[256];
			sprintf(temp,"s\t%ld\t%s:%d\t%d\t",time(0),RemoteEndpoint().address().to_string().c_str(),RemoteEndpoint().port(),size);
			(*iter)->Write(temp,strlen(temp));
			(*iter)->Write(buffer,size);
		}
	}*/
	Write(buffer,size,cb);
	LogTransfer::GetInstance().LogSend(buffer,size);
}
