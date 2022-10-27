#ifndef SRC_IPCALL_H_
#define SRC_IPCALL_H_

#include "UnixSocket.h"
#include "Looper.h"
#include "Parcel.h"

#define FIRST_USER_IP_CALL		32

class IPCall : public UnixSocket, public FdRunnable {
public:
	IPCall();
	IPCall(int fd,const std::string& callerName);

public:
	bool ping(uint64_t timeout);
	virtual bool connect(const std::string &addr, const std::string &ID);
	std::string getCallerName() const;
protected:
	bool call(const Parcel &parcel);
	bool sync(Parcel &parcel);
	virtual bool answer(Parcel &parcel) = 0;

	static const int PING;
	static const int PING_REPLY;

	std::string mCallerName;
private:
	virtual void run();

	virtual void pingAction();
	virtual void pingReplyAction();

	virtual void timeout();
	virtual void disconnected() = 0;

private:
	static const char *TAG;

	uint64_t mLastPingReplyTimestamp;
};

#endif /* SRC_IPCALL_H_ */
