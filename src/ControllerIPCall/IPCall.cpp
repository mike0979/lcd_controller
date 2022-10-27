#include "IPCall.h"
#include "SystemClock.h"
#include "Log.h"

#include <string.h>

IPCall::IPCall() : UnixSocket(STREAM, 0)
{
	mLastPingReplyTimestamp = 0;
}

IPCall::IPCall(int fd,const std::string& callerName) : UnixSocket(fd),mCallerName(callerName)
{
	mLastPingReplyTimestamp = 0;
}

bool IPCall::ping(uint64_t timeout)
{
	uint64_t uptime = SystemClock::UptimeMillis();

	if (mLastPingReplyTimestamp == 0) {
		mLastPingReplyTimestamp = uptime;
	}

	if (uptime - mLastPingReplyTimestamp > timeout) {
		return false;
	}
	else {
		Parcel pingcmd;
		pingcmd.writeInt32(PING);

		call(pingcmd);
		return true;
	}
}

bool IPCall::connect(const std::string &addr, const std::string &ID)
{
	bool conn = UnixSocket::connect(addr);
	if (conn == true) {
		size_t size = ID.length() + 1;
		size_t *data = (size_t *)alloca(size + sizeof(size));

		*data = size;
		memcpy(data + 1, ID.c_str(), size);
		conn = sendAll(data, size + sizeof(size)) == (int)(size + sizeof(size));
	}

	return conn;
}
std::string IPCall::getCallerName() const
{
    return mCallerName;
}
bool IPCall::call(const Parcel &parcel)
{
	return sendParcel(parcel);
}

bool IPCall::sync(Parcel &parcel)
{
	bool rp = recvParcel(parcel);
	if (rp == false) {
		LogE("Connection lost.\n");

		disconnected();
	}

	return rp;
}

bool IPCall::answer(Parcel &parcel)
{
	LogD("Do your answer actions.\n");
	return true;
}

void IPCall::run()
{
	Parcel parcel;
	if (sync(parcel) == true) {
		int32_t callid;
		parcel.readInt32(&callid);

		if (callid == PING) {
			pingAction();

			Parcel reply;
			reply.writeInt32(PING_REPLY);

			call(reply);
		}
		else if (callid == PING_REPLY) {
			mLastPingReplyTimestamp = SystemClock::UptimeMillis();

			pingReplyAction();
		}
		else {
			parcel.seek(0);
			answer(parcel);
		}
	}
}

void IPCall::pingAction()
{
	LogD("Do your ping actions.\n");
}

void IPCall::pingReplyAction()
{
	LogD("Do your ping reply actions.\n");
}

void IPCall::timeout()
{
	LogD("Do your timeout actions.\n");
}

const int IPCall::PING = 0;
const int IPCall::PING_REPLY = 1;

const char *IPCall::TAG = "IPCall";
