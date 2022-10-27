#ifndef SRC_SOCKET_H_
#define SRC_SOCKET_H_

#include "Parcel.h"

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

#include <limits.h>

class Socket {
public:
	virtual ~Socket();

	virtual int send(const void* data, size_t len);
	virtual int sendAll(const void* data, size_t len);
	virtual bool sendParcel(const Parcel &parcel);
	virtual int recv(void* data, size_t len);
	virtual int recvAll(void* data, size_t len, int retry = INT_MAX);
	virtual bool recvParcel(Parcel &parcel);

	virtual bool disconnect();

	bool peerConnected() const;

	bool setNonBlocking(bool nonblocking = true);

	// client api
	virtual bool connect(const std::string& addr, unsigned short port, bool nonblocking = true);

	// server api
	virtual bool bind(unsigned short port = 0);
	virtual bool bind(const std::string& addr, unsigned short port = 0);

	virtual bool listen(int max = 8);

	virtual int accept() const;

	int getSocketFd();

	void closeSocket();
public:
	static int recv(int fd, void *data, size_t len);
	static int send(int fd, const void *data, size_t len);

protected:
	static bool assignAddress(const std::string& addr, unsigned short port, sockaddr_in& sockaddrin);

protected:
	enum SocketDomain
	{
		INTERNET = PF_INET,
		UNIX = PF_LOCAL
	};

	enum SocketType
	{
		STREAM = SOCK_STREAM,
		DATAGRAM = SOCK_DGRAM
	};

	Socket(int sockfd);

	Socket(int domain, int type, int protocol);

	int reGenSocketFd();

	int mSocketFd;
	bool mPeerConnected;

	int mSocketDomain;
	int mSocketType;
	int mSocketProtocol;

private:
	static const char *TAG;
};

#endif /* SRC_SOCKET_H_ */
