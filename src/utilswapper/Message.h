#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <stdlib.h>
#include <stdint.h>

#include <string>

class Handler;

class Message {
public:
	Message(int what, int arg1 = 0, int arg2 = 0, void *data = NULL);
	Message(int what, void *data, int arg1 = 0, int arg2 = 0);
	~Message();

public:
	int mWhat;
	int mArg1, mArg2;
	std::string mStr;
	void *mData;
private:
	static const char *TAG;

	Handler *mHandler;
	uint64_t mWhen;

	friend class Looper;
	friend class Handler;
	friend class MessageQueue;
};

#endif /* MESSAGE_H_ */
