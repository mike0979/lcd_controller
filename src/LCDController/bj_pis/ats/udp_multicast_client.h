#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <iostream>
#include <functional>

using namespace std;

class udp_multicast_client
{
public:
	udp_multicast_client();
	~udp_multicast_client();
	void Init(const string &group_ip, int group_port);
	int ReadWait(char *s, int l);
	void Start();
	void Stop();
	function<void(const char *buffer, int n)> OnMessage;
	function<void(const string &log)> OnLog;

private:
	//组播的IP
	char m_group_ip[16];
	//组播的端口
	int m_group_port = 0;
	//接收消息的缓存
	char *m_buffer;
	//组播地址
	struct sockaddr_in m_group_addr;
	socklen_t m_group_addr_len;

	//socket_id
	int m_socket_fd;

	void WriteLog(const string &log);
	void PrintOriginalData(const char *data, int size);
};
