#include "Message.h"

Message::Message(int what, int arg1, int arg2, void *data) :
		mWhat(what), mArg1(arg1), mArg2(arg2), mData(data)
{
	mHandler = NULL;

	mWhen = 0;
}

Message::Message(int what, void *data, int arg1, int arg2) :
		mWhat(what), mArg1(arg1), mArg2(arg2), mData(data)
{
	mHandler = NULL;

	mWhen = 0;
}

Message::~Message()
{
}

const char *Message::TAG = "Message";
