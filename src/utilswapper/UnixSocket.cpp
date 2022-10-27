#include "UnixSocket.h"
#include "Log.h"

#include <errno.h>
#include <unistd.h>
#include <sys/un.h>

UnixSocket::UnixSocket(int type, int protocol) : Socket(UNIX, type, protocol)
{

}

UnixSocket::UnixSocket(int fd) : Socket(fd)
{

}

bool UnixSocket::connect(const std::string& addr)
{
	reGenSocketFd();

	struct sockaddr_un un;
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, addr.c_str());

	if (::connect(mSocketFd, (struct sockaddr *)&un, sizeof(un)) < 0) {
		mPeerConnected = false;

		LogE("Connect failed[%s]. %s\n", addr.c_str(), strerror(errno));
	}
	else {
		mPeerConnected = true;
	}

	return peerConnected();
}

bool UnixSocket::bind(const std::string& addr)
{
	struct sockaddr_un un;
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, addr.c_str());

	::unlink(addr.c_str());

	if(::bind(mSocketFd, (sockaddr*) &un, sizeof(un)) < 0) {
		LogE("Set of local path failed (bind)[%s]. %s\n", addr.c_str(), strerror(errno));

		return false;
	}

	return true;
}

const char *UnixSocket::TAG = "UnixSocket";
