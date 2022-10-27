#ifndef UNIXSOCKET_H_
#define UNIXSOCKET_H_

#include "Socket.h"

class UnixSocket  : public Socket {
public:
	UnixSocket(int type, int protocol);
	UnixSocket(int fd);

	virtual bool connect(const std::string& addr);
	virtual bool bind(const std::string& addr);

private:
	static const char *TAG;
};

#endif /* UNIXSOCKET_H_ */
