#include "udp_multicast_client.h"
#include <sstream>
#include <iomanip>
#include <unistd.h>

udp_multicast_client::udp_multicast_client()
{
	m_buffer = nullptr;
}

udp_multicast_client::~udp_multicast_client()
{
	if (m_buffer)
	{
		delete[] m_buffer;
		m_buffer = nullptr;
	}
}

void udp_multicast_client::Init(const string &group_ip, int group_port)
{
	strcpy(m_group_ip, group_ip.c_str());
	m_group_port = group_port;

	m_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_socket_fd < 0)
	{
		WriteLog("socket multicast failed!");
		return;
	}

	u_int yes;
	//允许端口复用
	if (setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
	{
		WriteLog("Reusing ADDR failed!");
		return;
	}

	//初始化组播地址
	memset(&m_group_addr, 0, sizeof(struct sockaddr_in));
	m_group_addr.sin_family = AF_INET;
	m_group_addr.sin_port = htons(m_group_port);
	m_group_addr.sin_addr.s_addr = inet_addr(m_group_ip);
	m_group_addr_len = sizeof(m_group_addr);

	//初始化本地地址
	struct sockaddr_in local_addr;
	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(m_group_port);              //this port must be the group port

	//绑定本地地址和端口
	if (::bind(m_socket_fd, (struct sockaddr*) &local_addr, sizeof(local_addr)) == -1)
	{
		WriteLog("Binding the multicast failed!");
		return;
	}

	//加入组播组
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(m_group_ip);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(m_socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
	{
		WriteLog("setsockopt multicast failed!");
		return;
	}
}

void udp_multicast_client::PrintOriginalData(const char *data, int size)
{
	string blank = " ";
	std::ostringstream ss;
	ss << "recv multicast,len=" << size << ",data:";
	ss << setiosflags(ios::uppercase) << hex;
	ss.fill('0');
	for (int i = 0; i < size; ++i)
	{
		unsigned char c = data[i];
		ss << setw(2) << (int) c;
		ss << setw(1) << blank;
	}
	WriteLog(ss.str());
}

#define BUFFER_SIZE 1024
void udp_multicast_client::Start()
{
	if (m_buffer)
	{
		m_buffer = new char[BUFFER_SIZE];
	}
	while (1)
	{
		bzero(m_buffer, BUFFER_SIZE);
		int n;
		//接收
		n = recvfrom(m_socket_fd, m_buffer, BUFFER_SIZE, 0, (struct sockaddr*) &m_group_addr, &m_group_addr_len);
		if (n < 0)
		{
			WriteLog("recvfrom err in udp talk!");
		}
		else if (n == 0)
		{
			WriteLog("recvfrom 0 after shutdown!");
		}
		else
		{
			PrintOriginalData(m_buffer, n);
			if (OnMessage)
			{
				OnMessage(m_buffer, n);
			}
		}

	}
}

int udp_multicast_client::ReadWait(char *s, int l)
{
	int n = recvfrom(m_socket_fd, s, l, 0, (struct sockaddr*) &m_group_addr, &m_group_addr_len);
	if (n < 0)
	{
		WriteLog("recvfrom err in udp talk!");
	}
	else if (n == 0)
	{
		WriteLog("recvfrom 0 after shutdown");
	}
	else
	{
		PrintOriginalData(s, n);
	}
	return n;
}

void udp_multicast_client::Stop()
{
	shutdown(m_socket_fd, SHUT_RDWR);
	close(m_socket_fd);
}

void udp_multicast_client::WriteLog(const string &log)
{
	if (OnLog)
	{
		OnLog(log);
	}
}
