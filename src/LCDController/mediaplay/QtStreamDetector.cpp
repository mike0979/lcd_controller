/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/

/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/

#include "QtStreamDetector.h"
#include "Log.h"
//#include "TvsStateManager.h"
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

QtStreamDetector::QtStreamDetector(std::string ip,int port,int timeout)
         : mIp(ip),mPort(port),mFd(-1),mbPause(false),mMultiCastTimeout(timeout)
{
	pthread_mutex_init(&mMutex,NULL);
	pthread_cond_init(&mCond, NULL);

	mLiveStreamOK = false;
}

QtStreamDetector::~QtStreamDetector()
{
	if(mFd > 0)
		close(mFd);

	pthread_mutex_destroy(&mMutex);
	pthread_cond_destroy(&mCond);
}

void QtStreamDetector::pauseDetect()
{
	if (!mbPause)
	{
		pthread_mutex_lock(&mMutex);
		mbPause = true;
		LogD(" ------------------ Pause live stream detector! ------------------\n");
		pthread_mutex_unlock(&mMutex);
	}
	else
	{
//		LogD(" ------------------ Live stream detector is already paused! ------------------\n");
	}
}

void QtStreamDetector::resumeDetect()
{
	//LogD("-----------------2  %d\n",mbPause);
	if (mbPause)
	{
		pthread_mutex_lock(&mMutex);
		mbPause = false;
		int ret = pthread_cond_signal(&mCond);
		pthread_mutex_unlock(&mMutex);

		//LogD("------------------ Resume live stream detector! pthread_cond_signal ------------------\n");
	}
	else
	{
//		LogD(" ------------------ Live stream detector is already resumed! ------------------\n");
	}
}

bool QtStreamDetector::getStreamStatus()
{
//	synchronized(mLiveStreamMutex)
//	{
		return mLiveStreamOK;
//	}
}

void QtStreamDetector::setStreamStatus(bool status)
{
//	synchronized(mLiveStreamMutex)
//	{
		mLiveStreamOK = status;
//	}
}

void QtStreamDetector::doInit()
{
	int n;
	u_int flag = 1; /*** MODIFICATION TO ORIGINAL */

	/* create what looks like an ordinary UDP socket */
	if ((mFd=socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		LogE("creat socket failure\n");
		return;
	}

	/**** MODIFICATION TO ORIGINAL */
	/* allow multiple sockets to use the same PORT number */
	if (setsockopt(mFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0){
		LogE("reusing addr failure\n");
		return;
	}
	/*** END OF MODIFICATION TO ORIGINAL */

	/* set up destination address */
	memset(&mAddr,0,sizeof(mAddr));
	mAddr.sin_family = AF_INET;
	mAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* N.B.: differs from sender */
	mAddr.sin_port = htons(mPort);

	/* bind to receive address */
	if (bind(mFd, (struct sockaddr *) &mAddr, sizeof(mAddr)) < 0){
		LogE("bind socket failure\n");
		return;
	}
	mIpPortChanged=false;
	/* use setsockopt() to request that the kernel join a multicast group */
	mMreq.imr_multiaddr.s_addr = inet_addr(mIp.c_str());
	mMreq.imr_interface.s_addr = htonl(INADDR_ANY);

	do
	{
		if (setsockopt(mFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mMreq, sizeof(mMreq)) < 0)
		{
			//LogE("setsockopt() failure\n");
			continue;
		}
		else
		{
			break;
		}
	}while(1);
}
void QtStreamDetector::onPauseStreamDetect()
{
	pauseDetect();
}

void QtStreamDetector::onResumeStreamDetect()
{
	//doInit();       // re-init the socket.
	resumeDetect(); // resume the thread to detect stream.
}

void QtStreamDetector::run()
{
	if(mFd > 0)
	{
		close(mFd);
		mFd = -1;
	}

	int n=0, addrlen = 0;
	char recvBuf[32];

	// first time init.
	doInit();
	/* now just enter a read-print loop */

	fd_set readfds;
	int maxfds = 0;
	int selectRet = -1;
	struct timeval timeout;

	bool bHaveSent = false;

	while (1)
	{
		pthread_mutex_lock(&mMutex);
		if (mbPause)
		{
			LogD("------------------ LiveStreamDetec thread -- start wait.... ------------------\n");
			pthread_cond_wait(&mCond, &mMutex);
			if(mFd > 0)
			{
				close(mFd);
				mFd = -1;
			}
			doInit();
			LogD("------------------ LiveStreamDetec thread -- got conditional signal.... ------------------\n");
		}
		if(mIpPortChanged)
		{
			if(mFd > 0)
			{
				close(mFd);
				mFd = -1;
			}
			doInit();
		}
		pthread_mutex_unlock(&mMutex);

//		LogD("------------------ LiveStreamDetec thread -- detecting.... ------------------\n");

		timeout.tv_sec = mMultiCastTimeout;
		timeout.tv_usec = 0;

		addrlen=sizeof(mAddr);
		FD_ZERO(&readfds);
		FD_SET(mFd,&readfds);

		maxfds = mFd +1;

		selectRet = select(maxfds, &readfds, NULL, NULL, &timeout);
		if( selectRet > 0)
		{
			n = recvfrom(mFd,recvBuf,sizeof(recvBuf),0,(struct sockaddr *)&mAddr,(socklen_t*)&addrlen);
			if(n > 0)
			{
				emit signalProbeStreamReport(true);
				bHaveSent = true;

				setsockopt(mFd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mMreq, sizeof(mMreq)); // quit multicast
				close(mFd); // close socket.
				mFd = -1;
				//doInit(); // re-init the socket otherwise select will always return >0(because readFds is always ready);
				pauseDetect(); // have stream, so pause detect thread.
				continue;
			}
			else
			{
				// Peer side closed
//				LogD("############ LiveStreamDetector recv <=0  ############ \n");
				emit signalProbeStreamReport(false);
				resumeDetect(); // have no stream, resume detect thread.
			}
		}
		else
		{
			// select error or time out

			emit signalProbeStreamReport(false);
			resumeDetect(); // have no stream, resume detect thread.
		}

//		if(mFd > 0)
//		{
//			close(mFd);
//			mFd = -1;
//		}
	}
}

bool QtStreamDetector::SetIpPort(const std::string& ip,int port)
{
	if(ip!=mIp||port!=mPort)
	{
		mIp=ip;
		mPort=port;
		mIpPortChanged=true;
		mLiveStreamOK = false;
		return true;
	}
	return false;
}

const char * QtStreamDetector::TAG = "QtStreamDetector";


