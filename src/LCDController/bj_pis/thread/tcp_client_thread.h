#ifndef TCP_CLINET_THREAD_H_
#define TCP_CLINET_THREAD_H_

#include "bj_pis/thread/thread_zcg.h"
#include <functional>
#include "bj_pis/tcp_client/tcp_client.h"
class TcpClientThread : public ThreadZCG
{
public:
	TcpClientThread(const char* ip, const int port);
	~TcpClientThread();
	void Start();
	void PostMessage(const char* data, int size);
	void Dispose() override;
	std::function<void(Client*)> Connected;
public:
	boost::asio::io_service io_service_;
	Client client_{io_service_,"TcpClient"};
};

#endif
