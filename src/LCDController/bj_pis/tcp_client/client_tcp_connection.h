#ifndef CLIENT_TCP_CONNECTION_H_
#define CLIENT_TCP_CONNECTION_H_

#include <net/message.h>
#include <net/tcp_connection.h>

class ClientTcpConnection :public waxberry::WaxberryTcpConnection<waxberry::Message>
{
public:
	explicit ClientTcpConnection(waxberry::BasicHost& host);
	void OnMessage(waxberry::Message& msg) override;
	void OnConnected() override
	{
		printf("####################connect success\n");
	};
};


#endif
