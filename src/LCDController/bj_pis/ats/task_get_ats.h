#pragma once
#include "udp_multicast_client.h"

class task_get_ats
{
public:
	void Start();
	void Stop();
	string ats_url;
	static task_get_ats& GetInstance();
	static const char* TAG;
private:
	task_get_ats();
	bool run_flag;
	bool running;
	void Run();
	void DealATS(char* s, int l);
	udp_multicast_client m_client;
};
