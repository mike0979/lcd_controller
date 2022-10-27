#include "tcp_client_thread.h"
#include "bj_pis/tcp_client/tcp_client.h"
#include <thread>

TcpClientThread::TcpClientThread(const char* ip, const int port)
{
	client_.Init(ip, port);
}

TcpClientThread::~TcpClientThread()
{
	Dispose();
}

void TcpClientThread::Start()
{
	is_run_ = true;
	client_.Connected=this->Connected;
	thread_ = std::thread([this]()
		{
			client_.Run();
			io_service_.run();
		});
}

void TcpClientThread::PostMessage(const char* data, int size)
{
	client_.Connection()->PostWrite(data, size);
}

void TcpClientThread::Dispose()
{
	client_.Stop();
	ThreadZCG::Dispose();
}

