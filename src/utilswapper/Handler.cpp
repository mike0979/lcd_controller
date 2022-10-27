#include "Handler.h"
#include "Looper.h"

Handler::Handler(Looper *looper) : mLooper(looper)
{
	if (mLooper == NULL) {
		mLooper = Looper::MainLooper();

		if (mLooper == NULL) {
			printf("MainLooper has NOT been created.\n");
		}
	}
}

Handler::~Handler()
{

}

bool Handler::setLooper(Looper *looper)
{
	if (looper != NULL) {
		mLooper = looper;

		return true;
	}
	else {
		return false;
	}
}

Looper *Handler::getLooper() const
{
	return mLooper;
}

bool Handler::sendMessage(Message *msg, unsigned delay)
{
	if (mLooper == NULL) {
		printf("Can NOT send message to a unlooped handler. Setup looper before send message.\n");

		return false;
	}

	msg->mHandler = this;
	return mLooper->getMessageQueue()->enqueueMessage(msg, delay);
}

int Handler::removeMessage(int what)
{
	if (mLooper == NULL) {
		return 0;
	}

	return mLooper->getMessageQueue()->removeMessages(what);
}

int Handler::hasMessage(int what)
{
	if (mLooper == NULL) {
		return 0;
	}

	return mLooper->getMessageQueue()->hasMessages(what);
}

bool Handler::handle(Message *msg)
{
	bool ret = handleMessage(msg);

	if(msg != NULL)
	    delete msg;
	return ret;
}

bool Handler::handleMessage(Message *msg)
{
	printf("Override Handler::handleMessage to handle message.\n");

	return true;
}

const char *Handler::TAG = "Handler";
