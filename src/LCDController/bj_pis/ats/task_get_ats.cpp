#include "task_get_ats.h"
#include <thread>
#include "Log.h"
#include <unistd.h>
#include "bj_ats_info.h"

task_get_ats::task_get_ats()
{}
const char* task_get_ats::TAG="task_get_ats";
task_get_ats& task_get_ats::GetInstance()
{
	static task_get_ats _task;
	return _task;
}

void task_get_ats::Start()
{
	run_flag=true;
	thread t(std::bind(&task_get_ats::Run, this));
	t.detach();
}

void task_get_ats::Run()
{
	sleep(3);

	LogD("task get ats--start\n");
	string url;
	if(ats_url.empty())
	{
		url="udp://224.0.0.1:9099";
	}
	else
	{
		url=ats_url;
	}

	int index1=url.find("//");
	int index2=url.find(":",index1);
	m_client.Init(url.substr(index1+2,index2-index1-2),stoi(url.substr(index2+1)));
	LogD("listen to %s\n",url.c_str());

	m_client.OnLog=[](const string& log){
		LogD("%s\n",log.c_str());
	};
	running=true;
	char buffer[10240];
	while(run_flag)
	{
		int count = m_client.ReadWait(buffer,10240);
		if(count<0)
		{
			LogE("udp 组播接收失败,run_flag:%d\n",run_flag);
		}
		else if(count>0)
		{
			try
			{
				bj_ats_info::Instance().DealATS(buffer,count);
			}
			catch(exception &e)
			{
				LogE("task get ats error:%s, detail:%s\n", typeid(e).name(), e.what());
			}
		}
	}
	running=false;
	LogD("task get ats--end\n");
}


void task_get_ats::Stop()
{
	run_flag=false;
	m_client.Stop();
	while(running) {
		usleep(10);
	}
}

