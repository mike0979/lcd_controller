#include "tcp_server.h"

void Server::Init(const char* ip, const int port)
{
	server.Init(ip, port);
}

void Server::Run()
{
	server.Run();
	io_service_.run();
}

void Server::Stop()
{
	server.Stop();
	io_service_.stop();
}

