#ifndef TCP_SERVER_THREAD_H_
#define TCP_SERVER_THREAD_H_

#include "bj_pis/thread/thread_zcg.h"
#include "bj_pis/tcp_server/tcp_server.h"

class TcpServerThread : public ThreadZCG
{
public:
	TcpServerThread(const char* ip, int port);
	void Start();
	void Dispose() override;
private:
	Server server_;
};

#endif
