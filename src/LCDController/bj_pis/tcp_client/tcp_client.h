#ifndef TCP_CLIENT
#define TCP_CLIENT

#include <net/tcp_client.hpp>
#include "client_tcp_connection.h"

class Client : public waxberry::TcpClient<ClientTcpConnection>
{
public:
	explicit Client(boost::asio::io_service & io_service,const string & name)
		:waxberry::TcpClient<ClientTcpConnection>(io_service,name){};
	void OnConnected(const connection_ptr &) override;
	std::function<void(Client*)> Connected;
private:
};

#endif
