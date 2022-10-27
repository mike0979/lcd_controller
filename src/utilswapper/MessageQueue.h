#ifndef MESSAGEQUEUE_H_
#define MESSAGEQUEUE_H_

#include "Message.h"
#include "Mutex.h"
#include <stdio.h>
#include <list>

class Looper;

class MessageQueue {
public:
	MessageQueue(Looper *looper);
	~MessageQueue();

	Message *next(int &nextMsgTimeout);

	bool enqueueMessage(Message *msg, unsigned delay = 0);
	int hasMessages(int what);
	int removeMessages(int what);
private:
	static const char *TAG;

	Looper *mLooper;
	std::list<Message *> mMessages;

	Mutex mMessagesMutex;
};

#endif /* MESSAGEQUEUE_H_ */
