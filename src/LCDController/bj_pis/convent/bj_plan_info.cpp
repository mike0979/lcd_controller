#include "bj_plan_info.h"
#include "tcp_msg_help.h"

void bj_plan_info::Parse(const char* data)
{
	int index=0;
	PlanName=TcpGetStringGB2312(data,index,64);
	PlanID=TcpGetString(data,index,32);
	UpdateTime=TcpGetDateTime(data,index,14);
	FileCount=TcpGetInt(data,index,4);
	Files.reserve(FileCount);
	for(int i=0;i<FileCount;i++)
	{
		shared_ptr<bj_file_info> file = make_shared<bj_file_info>();
		int c = file->Parse(data+index);
		Files.push_back(file);
		index+=c;
	}
}

int bj_plan_info::Write(char* data)
{
	int index=0;
	TcpSetString(data,index,PlanName,64);
	TcpSetString(data,index,PlanID,32);
	TcpSetDateTime(data,index,UpdateTime,14);
	TcpSetInt(data,index,FileCount,4);
	for (std::vector<shared_ptr<bj_file_info>>::iterator iter = Files.begin(); iter != Files.end(); ++iter)
	{
		int c = (*iter)->Write(data);
		index+=c;
	}
	return index;
}
