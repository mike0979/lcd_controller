#ifndef SERVER_TCP_CONNECTION_H_
#define SERVER_TCP_CONNECTION_H_

#include <net/message.h>
#include <net/tcp_connection.h>
#include <vector>
#include <map>

class ServerTcpConnection :public waxberry::WaxberryTcpConnection<waxberry::Message>
{
public:
	explicit ServerTcpConnection(waxberry::BasicHost& host);
	int OnReadMessage(const char* data, int size) override;
	void OnMessage(waxberry::Message& msg) override {};
	void OnConnected() override;
    void OnDisconnected() override;

    void Send(const char * buffer, int size, waxberry::MSG_SENT_CALLBACK cb = 0);

    static vector<ServerTcpConnection*> TransferConns;
    static const char* TAG;
private:
    static std::map<ServerTcpConnection*,bool> States;
};

#endif
