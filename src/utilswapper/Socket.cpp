#include "Socket.h"
#include "SystemClock.h"
#include "Log.h"

#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <netdb.h>
#include <arpa/inet.h>

#include <alloca.h>

Socket::~Socket()
{
	TEMP_FAILURE_RETRY(::close(mSocketFd));
}

int Socket::send(const void* data, size_t len)
{
	if (peerConnected() == false) {
		LogD("Socket is NOT connected.\n");
		return 0;
	}

	int slen = TEMP_FAILURE_RETRY(::send(mSocketFd, data, len, MSG_NOSIGNAL));
	if (slen < 0)
	{
		switch(errno)
		{
		case EBADF:
		case ENOTSOCK:
		case ECONNRESET:
		case ECONNREFUSED:
			mPeerConnected = false;
			LogE("Send error. %s\n", strerror(errno));
			break;

		default:
			LogE("Send failed. %s\n", strerror(errno));
			break;
		}
	}

	return slen;
}

int Socket::sendAll(const void* data, size_t len)
{
	if (peerConnected() == false) {
		LogD("Socket is NOT connected.\n");
		return 0;
	}

	int slen = 0;
	while (slen != (int)len)
	{
		const char* buf = static_cast<const char*>(data) + slen;
		int ret = Socket::send(buf, len - slen);
		if (ret <= 0) {
			return ret;
		}

		slen += ret;
	}

	return slen;
}

bool Socket::sendParcel(const Parcel &parcel)
{
	size_t size = parcel.dataSize();
	size_t *data = (size_t *)alloca(size + sizeof(size));

	*data = size;
	memcpy(data + 1, parcel.data(), size);

	return sendAll(data, size + sizeof(size)) == (int)(size + sizeof(size));
}

int Socket::recv(void* data, size_t len)
{
	if (peerConnected() == false) {
		LogD("Socket is NOT connected.\n");
		return 0;
	}

	int rlen = TEMP_FAILURE_RETRY (::recv(mSocketFd, data, len, 0));
	if (rlen < 0) {
		switch(errno)
		{
		case EAGAIN:
//		case EWOULDBLOCK:
//			LogD("Read Again.\n");
			break;

		case EBADF:
		case ENOTSOCK:
		case ENOTCONN:
		case ECONNREFUSED:
			mPeerConnected = false;
			LogE("Received error. %s\n", strerror(errno));
			break;

		default:
			LogE("Received failed. %s\n", strerror(errno));
			break;
		}
	}
	else if (rlen == 0) {
		LogE("Connection Closed.\n");
	}

	return rlen;
}

int Socket::recvAll(void* data, size_t len, int retry)
{
	if (peerConnected() == false) {
		LogD("Socket is NOT connected.\n");
		return 0;
	}

	int eagain = 0;
	int rlen = 0;
	while (rlen != (int)len) {
		char* buf = static_cast<char*>(data) + rlen;
		int ret = Socket::recv((void *)buf, len - rlen);
		if(ret <= 0) {
			if (ret < 0 && errno == EAGAIN) {
				if (eagain++ < retry) {
					SystemClock::Sleep(50);

					LogD("Read Again.\n");
					continue;
				}
				else {
					break;
				}
			}
			else {
				return ret;
			}
		}

		rlen += ret;
	}

	return rlen;
}

bool Socket::recvParcel(Parcel &parcel)
{
	size_t psize = 0;

	int recvsize = recv(&psize, sizeof(psize));
	if (recvsize == sizeof(psize)) {
		if (psize > 0) {
			uint8_t *data = (uint8_t *)malloc(psize);
			if (data != NULL) {
				int recvsize = recvAll(data, psize);
				if (recvsize == (int)psize) {
					parcel.setData(data, psize);

					return true;
				}
				else {
					free(data);
				}
			}
		}
		else {
			return true;
		}
	}

	return false;
}

bool Socket::disconnect()
{
	if (peerConnected() == true) {
#if 0
		sockaddr addr;
		memset(&addr, 0, sizeof(addr));
		addr.sa_family = AF_UNSPEC;

		if(::connect(mSocketFd, &addr, sizeof(addr)) < 0)
		{
			if (errno != ECONNRESET) {
				LogE("Disconnect failed. %s\n", strerror(errno));
			}
		}
		else {
			mPeerConnected = false;
		}
#else
		TEMP_FAILURE_RETRY(::close(mSocketFd));

		mSocketFd = -1;
		mPeerConnected = false;
#endif
	}

	return peerConnected() == false;
}

bool Socket::peerConnected() const
{
	return mPeerConnected;
}

bool Socket::setNonBlocking(bool nonblocking)
{
	int flags = fcntl(mSocketFd, F_GETFL, 0);
	if (flags == -1) {
		return false;
	}

	if (nonblocking == true) {
		return fcntl(mSocketFd, F_SETFL, flags | O_NONBLOCK) == 0;
	}
	else {
		// todo: ^^^^^^^
	}

	return false;
}

// client api
bool Socket::connect(const std::string& addr, unsigned short port, bool nonblocking)
{
	if (peerConnected() == false) {
		reGenSocketFd();
		setNonBlocking(nonblocking);

		sockaddr_in sockaddrin;
		if (assignAddress(addr, port, sockaddrin)) {
			if (::connect(mSocketFd, (sockaddr *) &sockaddrin, sizeof(sockaddrin)) < 0) {
				mPeerConnected = false;

				LogE("Connect failed[%s:%d]. %s[%d]\n", addr.c_str(), port, strerror(errno), errno);
			}
			else {
				mPeerConnected = true;
			}
		}
	}

	return peerConnected();
}

// server api
bool Socket::bind(unsigned short port)
{
	sockaddr_in sockaddrin;
	sockaddrin.sin_family = AF_INET;
	sockaddrin.sin_addr.s_addr = INADDR_ANY;
	sockaddrin.sin_port = htons(port);

	if(::bind(mSocketFd, (sockaddr*) &sockaddrin, sizeof(sockaddrin)) < 0) {
		LogE("Set of local port failed[%d]. %s\n", port, strerror(errno));
		return false;
	}

	return true;
}

bool Socket::bind(const std::string& addr, unsigned short port)
{
	sockaddr_in sockaddrin;

	if (assignAddress(addr, port, sockaddrin)) {
		if(::bind(mSocketFd, (sockaddr*) &sockaddrin, sizeof(sockaddrin)) < 0) {
			LogE("Set of local address and port failed[%s:%d]. %s\n", addr.c_str(), port, strerror(errno));
			return false;
		}

		return true;
	}

	return false;
}

bool Socket::listen(int max)
{
	int ret = ::listen(mSocketFd, max);
	if (ret < 0) {
		LogE("Listen failed, most likely another socket is already listening on the same port.\n");
		return false;
	}

	return true;
}

int Socket::accept() const
{
	int fd = ::accept(mSocketFd, 0, 0);
	if (fd < 0) {
		LogE("Accept failed. %s\n", strerror(errno));
	}

	return fd;
}

int Socket::getSocketFd()
{
	return mSocketFd;
}

void Socket::closeSocket()
{
    if(mSocketFd > 0)
    {
        LogD("Close the socket fd = %d\n", mSocketFd);
        close(mSocketFd);
        mSocketFd = -1;
        LogD("After close the socket fd = %d\n", mSocketFd);
    }
}

int Socket::recv(int fd, void *data, size_t len)
{
	return TEMP_FAILURE_RETRY (::recv(fd, data, len, 0));
}

int Socket::send(int fd, const void *data, size_t len)
{
	return TEMP_FAILURE_RETRY(::send(fd, data, len, MSG_NOSIGNAL));
}

bool Socket::assignAddress(const std::string& addr, unsigned short port, sockaddr_in& sockaddrin)
{
	sockaddrin.sin_family = AF_INET;
	sockaddrin.sin_port = htons(port);

	if (addr.empty()) {
		sockaddrin.sin_addr.s_addr = htonl(INADDR_ANY);
		return true;
	}

	// Assume we have a simple ipv4 address
	if(inet_aton(addr.c_str(), &sockaddrin.sin_addr)) {
		return true;
	}

	// We need to resolve the address
	hostent* host = gethostbyname(addr.c_str());
	if (host == 0) {
		LogE("Failed to resolve address. %s\n", strerror(errno));
		return false;
	}
	sockaddrin.sin_addr.s_addr = *reinterpret_cast<uint32_t*>(host->h_addr);

	return true;
}

Socket::Socket(int sockfd) : mSocketFd(sockfd)
{
	mPeerConnected = true;

	mSocketDomain = -1;
	mSocketType = -1;
	mSocketProtocol = -1;
}

Socket::Socket(int domain, int type, int protocol)
{
	mSocketFd = -1;

	mSocketDomain = domain;
	mSocketType = type;
	mSocketProtocol = protocol;

	reGenSocketFd();

	mPeerConnected = false;
}

int Socket::reGenSocketFd()
{
	if (mSocketFd < 0) {
		mSocketFd = ::socket(mSocketDomain, mSocketType, mSocketProtocol);
		if (mSocketFd < 0) {
			LogE("Socket creation failed. %s\n", strerror(errno));
		}
		else {
			int flag = 1;

	//		fcntl(mSocketFd, F_SETFL, O_NONBLOCK);
//            LogD("reGenSocketFd!!! fd=%d \n", mSocketFd);
			setsockopt(mSocketFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
		}
	}

	return mSocketFd;
}

const char *Socket::TAG = "Socket";
