/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/

#include <config/configparser.h>
#include <fcntl.h>
#include <LCDController.h>
#include <Log.h>
#include <Looper.h>
#include <Message.h>
#include <ntp/NtpDate.h>
#include <pthread.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>

NtpDate::NtpDate(LCDController *lcdcontroller) : mLCDController(lcdcontroller)
{
	const ConfigParser *config = mLCDController->getConfig();

	mServer = config->mNtpServer;
	mPeriod = config->mNtpDatePeriod;
}

void NtpDate::run()
{
	Looper *mlooper = Looper::CreateLooper();
	setLooper(mlooper);

	sendMessage(new Message(NtpPeriod));

	mlooper->loop();
}

bool NtpDate::handleMessage(Message *msg)
{
//	float offset = 0.0f;
//	if (SystemClock::NtpDate(mServer, offset)) {
//		mTvs->sendMessage(new Message(TvsStateManager::NtpDateMsg, reinterpret_cast<int &>(offset)));
//	}
//	else {
//		LogE("Ntp Date Failed!\n");
//	}

	pthread_t tid;
	int ret = pthread_create(&tid,NULL,ntpSyncProc,mLCDController);
	if(ret < 0)
	{
		LogE("Thread for ntpsync created failed.\n");
	}
	else
	{
		pthread_detach(tid);
		LogD("Thread for ntpsync created success.\n");
	}

	//sendMessage(new Message(NtpPeriod), mPeriod * 1000);

	return true;
}

void* NtpDate::ntpSyncProc(void* args)
{

	LCDController *lcdcontroller = (LCDController *)args;
	if(lcdcontroller != NULL)
	{
		bool update = false;
		float offset = 0;
		std::string cmd = "ntpdate ";
		cmd.append(mServer);
		//cmd.append(" &");

		while(1)
		{
			LogE("##### Start execute ntpdate to sync time,ntp server(%s).\n",mServer.c_str());
			FILE *freport = popen(cmd.c_str(), "r");

			int fd = fileno(freport);
			int flags;
			flags = fcntl(fd, F_GETFL, 0);
			flags |= O_NONBLOCK;
			fcntl(fd, F_SETFL, flags);

			if (freport == NULL) {
				LogW("Failed to execute ntpdate.\n");
			}
			else {
				usleep(10*1000*1000);
				char echo[1024];
				if(fgets(echo, sizeof(echo), freport) != NULL)
				{
					char *ntp = strstr(echo, "offset");
					if (ntp != NULL) {
						if (sscanf(ntp, "offset %f sec", &offset) == 1) {
							update = true;

							LogD("Adjust time offset %f second.\n", offset);
							system("hwclock -w");
							lcdcontroller->sendMessage(new Message(LCDController::NtpDateSync, reinterpret_cast<int &>(offset)));
						}
					}

					if (update == false) {
						LogE("Sync time failed: %s\n", echo);
					}
				}

				pclose(freport);
			}
			usleep(mPeriod * 1000 * 1000);
		}
	}

	return NULL;
}
const char *NtpDate::TAG = "NtpDate";
std::string NtpDate::mServer = "192.168.250.192";
int NtpDate::mPeriod = 600;


