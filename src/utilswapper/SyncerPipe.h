#ifndef SYNCERPIPE_H_
#define SYNCERPIPE_H_

class SyncerPipe {
public:
	SyncerPipe();

	int sendSyncerMsg(const char *msg);
	int recvSyncerMsg(char *msg, int max);

	int getSyncerFd();
	int syncer(unsigned timeout = 0);		// timeout in ms
private:
	static const char *TAG;

	int mSyncPipe[2];
};

#endif /* SYNCERPIPE_H_ */
