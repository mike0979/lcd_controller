#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include <net/tcp_server.hpp>
#include "server_tcp_connection.h"

class Server
{
public:
	void Init(const char* ip, const int port);
	void Run();
	void Stop();
private:
	boost::asio::io_service io_service_;
	waxberry::TcpServer<ServerTcpConnection> server{ io_service_, "tcp_server" };
};

#endif
