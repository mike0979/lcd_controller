#include "MessageQueue.h"
#include "Looper.h"
#include "SystemClock.h"

MessageQueue::MessageQueue(Looper *looper) : mLooper(looper)
{

}

MessageQueue::~MessageQueue()
{
	synchronized (mMessagesMutex) {
		for (std::list<Message *>::iterator i = mMessages.begin(); i != mMessages.end(); i++) {
			delete (*i);
		}
	}
}

Message *MessageQueue::next(int &nextMsgTimeout)
{
	Message *msg = NULL;

	synchronized (mMessagesMutex) {
		if (mMessages.empty()) {
			nextMsgTimeout = -1;
		}
		else {
			msg = mMessages.front();

			uint64_t now = SystemClock::UptimeMillis();
			if (msg->mWhen <= now) {
				mMessages.pop_front();

				if (mMessages.empty()) {
					nextMsgTimeout = -1;
				}
				else {
					Message *msgNext = mMessages.front();

					nextMsgTimeout = (int)(msgNext->mWhen - now);
					if (nextMsgTimeout < 0) {
						nextMsgTimeout = 0;
					}
				}
			}
			else {
				nextMsgTimeout = msg->mWhen - now;

				msg = NULL;
			}
		}
	}

	return msg;
}

bool MessageQueue::enqueueMessage(Message *msg, unsigned delay)
{
	if (mLooper->isQuit()) {
		printf("Sending message to a handler on dead looper.\n");

		return false;
	}


	uint64_t now = SystemClock::UptimeMillis();
	msg->mWhen = now + delay;

	synchronized (mMessagesMutex) {
		if (mMessages.empty()) {
			mMessages.push_front(msg);
		}
		else {
			std::list<Message *>::iterator i = mMessages.begin();
			while (i != mMessages.end()) {
				if ((*i)->mWhen > msg->mWhen) {
					mMessages.insert(i, msg);
					break;
				}

				i++;
			}
			if (i == mMessages.end()) {
				mMessages.push_back(msg);
			}
		}
	}

	mLooper->wake();
	return true;
}

int MessageQueue::hasMessages(int what)
{
	int counter = 0;

	synchronized (mMessagesMutex) {
		for (std::list<Message *>::iterator i = mMessages.begin(); i != mMessages.end(); i++) {
			if ((*i)->mWhat == what) {
				counter++;
			}
		}
	}

	return counter;
}

int MessageQueue::removeMessages(int what)
{
	int counter = 0;

	synchronized (mMessagesMutex) {
		std::list<Message *>::iterator i = mMessages.begin();
		while (i != mMessages.end()) {
			if ((*i)->mWhat == what) {
				i = mMessages.erase(i);

				counter++;
			}
			else {
				i++;
			}
		}
	}

	return counter;
}

const char *MessageQueue::TAG = "MessageQueue";
