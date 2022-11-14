#include "bj_msg.h"
#include <cstring>
#include "bj_pis/utils/datetime.h"
#include <stdio.h>
#include "tcp_msg_help.h"

//map<string,pBjHandler> bj_msg::Handlers;
int bj_msg::_sqn_for_send = 1;
bj_msg::bj_msg():SQN(1),TimeStamp(0),DataLength(0)
{
	Data=nullptr;
}

bj_msg::~bj_msg()
{
	if(Data!=nullptr)
	{
		delete [] Data;
		Data = nullptr;
	}
}

map<string,pBjHandler>& bj_msg::GetHandlers()
{
	static map<string,pBjHandler> _handlers;
	return _handlers;
}

void bj_msg::Parse(const char* data)
{
	int i=0;
	SQN=TcpGetInt(data,i,4);
	Version=TcpGetString(data,i,5);
	SystemCode=TcpGetString(data,i,2);
	TimeStamp=TcpGetDateTime(data,i,14);
	Command=TcpGetString(data,i,3);
	if(Command.at(0)=='M')
	{
		TaskID=TcpGetString(data,i,14);//获取任务ID
		/*char ss[33];
		ss[32]='\0';
		memcpy(ss,data+i,32);*/
		i+=32;
		/*int areaIndex=0;
		//获取线路
		AreaLine=TcpGetString(ss,areaIndex,2);
		TcpGetString(ss,areaIndex,1);//车站列车标识
		//获取车站开始
		string station=TcpGetString(ss,areaIndex,10);
		long long llstation=std::stoll(station);
		long long s=1;
		for(int j=0;j<37;j++)
		{
			if((s&llstation)!=0)
			{
				AreaStation.push_back(j+1);
			}
			s=s<<1;
		}
		//获取车站结束

		//获取区域开始(上下行站台等)
		string region=TcpGetString(ss,areaIndex,5);
		int iregion=std::stoi(region);
		int iv=1;

		for(int j=0;j<16;j++)
		{
			if((iv&iregion)!=0)
			{
				AreaRegion.push_back(j+1);
			}
			iv=iv<<1;
		}*/
		//获取区域结束(上下行站台等)
	}

	DataLength=std::stoi(TcpGetString(data,i,6));
	Data=new char[DataLength];
	memcpy(Data,data+i,DataLength);
}

int bj_msg::Write(char* data)
{
	int index=0;
	TcpSetInt(data, index, SQN, 4);
	TcpSetString(data, index, Version, 5);
	TcpSetString(data, index, SystemCode, 2);
	TcpSetDateTime(data, index, TimeStamp);
	TcpSetString(data, index, Command, 3);
	if(Command.at(0)=='M')
	{
		TcpSetString(data,index,TaskID,14);
		TcpSetString(data,index,AreaLine,2);
		data[index++]=0;//车站列车标识
		//车站10位
		long long lstation=0;
		for (std::vector<int>::iterator iter = AreaStation.begin(); iter != AreaStation.end(); ++iter)
		{
			long long ll=1;
			ll=ll<<(*iter-1);
			lstation+=ll;
		}
		sprintf(data+index, "%010lld", lstation);
		index+=10;
		//区域5位
		int iregion=0;
		for (std::vector<int>::iterator iter = AreaRegion.begin(); iter != AreaRegion.end(); ++iter)
		{
			iregion+=(1<<(*iter-1));
		}
		TcpSetInt(data,index,iregion,5);

		index+=14;//文档写的是13位,但是其实2+1+10+5+13=31，整体长度是32位，所以这里写了14。
	}
	
	TcpSetInt(data,index,DataLength,6);
	memcpy(data+index,Data,DataLength);
	return index+DataLength;
}

void bj_msg::Copy(bj_msg& msg)
{
	SQN=msg.SQN;
	Version=msg.Version;
	SystemCode=msg.SystemCode;
	TimeStamp=datetime::Now().get_time();
	Command=msg.Command;
	if(msg.Command.at(0)=='M')
	{
		TaskID=msg.TaskID;
		AreaLine=msg.AreaLine;
		AreaStation.assign(msg.AreaStation.begin(), msg.AreaStation.end());
		AreaRegion.assign(msg.AreaRegion.begin(), msg.AreaRegion.end());
	}
}

void bj_msg::SetData(const char* data,int len)
{
	if(Data!=nullptr)
	{
		delete [] Data;
	}
	DataLength = len;
	Data=new char[len]{'\0'};
	int l=std::min(len,(int)strlen(data));
	memcpy(Data, data, l);
}

int bj_msg::GetSendSQN()
{
	if(_sqn_for_send > 9990)
	{
		_sqn_for_send = 1;
	}
	return _sqn_for_send++;
}
