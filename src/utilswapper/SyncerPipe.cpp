#include "SyncerPipe.h"
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

SyncerPipe::SyncerPipe()
{
	int ret = pipe(mSyncPipe);
	if (ret != 0) {
		mSyncPipe[0] = -1;
		mSyncPipe[1] = -1;

		printf("Create Pipe failed.\n");
	}

	 ret = fcntl(mSyncPipe[0], F_SETFL, O_NONBLOCK);
	 if (ret != 0) {
		 printf("Could not make syncer read pipe non-blocking.\n");
	 }

	 ret = fcntl(mSyncPipe[1], F_SETFL, O_NONBLOCK);
	 if (ret != 0) {
		 printf("Could not make syncer write pipe non-blocking.\n");
	 }
}

int SyncerPipe::sendSyncerMsg(const char *msg)
{
	return write(mSyncPipe[1], msg, strlen(msg));
}

int SyncerPipe::recvSyncerMsg(char *msg, int max)
{
	return read(mSyncPipe[0], msg, max);
}

int SyncerPipe::getSyncerFd()
{
	return mSyncPipe[0];
}

int SyncerPipe::syncer(unsigned timeout)
{
	fd_set readFdSet;
	struct timeval *tout = NULL;
	if (timeout != 0) {
		tout = new struct timeval;
		tout->tv_sec = timeout / 1000;
		tout->tv_usec = (timeout % 1000) * 1000;
	}

	FD_ZERO(&readFdSet);
	FD_SET(mSyncPipe[0], &readFdSet);

	return select(mSyncPipe[0] + 1, &readFdSet, NULL, NULL, tout);
}

const char *SyncerPipe::TAG = "SyncerPipe";
