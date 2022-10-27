#include "tcp_server_thread.h"

TcpServerThread::TcpServerThread(const char* ip, int port)
{
	server_.Init(ip, port);
}

void TcpServerThread::Start()
{
	thread_ = std::thread([this]()
		{
			server_.Run();
		});
}

void TcpServerThread::Dispose()
{
	server_.Stop();
	ThreadZCG::Dispose();
}

