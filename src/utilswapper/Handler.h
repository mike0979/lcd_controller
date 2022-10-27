#ifndef HANDLER_H_
#define HANDLER_H_

#include <stdlib.h>
#include <stdio.h>
#include "Message.h"

class Looper;

class Handler {
public:
	Handler(Looper *looper = NULL);
	virtual ~Handler();

	bool setLooper(Looper *looper);
	Looper *getLooper() const;

	bool sendMessage(Message *msg, unsigned delay = 0);
	int removeMessage(int what);
	int hasMessage(int what);

public:
	bool handle(Message *msg);

protected:
	virtual bool handleMessage(Message *msg);

private:
	static const char *TAG;

	Looper *mLooper;

	friend class Looper;
};

#endif /* HANDLER_H_ */
