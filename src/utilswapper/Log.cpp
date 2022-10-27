#include "Log.h"

#include "EnvSetting.h"
#include "FileSysUtils.h"
#include "SystemClock.h"

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/timeb.h>
#define LOG_PREFIX_SIZE		128
#define LOG_MESSAGE_SIZE	2048

#define LogBuilder(fmt, msg)		do {																		\
												time_t t = time(NULL);                                                  \
												struct timeb tb;                                                        \
												ftime(&tb);                                                              \
												struct tm* stm = localtime(&(tb.time));                                  \
												int tlen = strftime(msg, LOG_PREFIX_SIZE, "%04Y-%02m-%02d %02H:%02M:%02S", localtime(&(tb.time)));		\
												int tlen1 = snprintf(msg + tlen, LOG_PREFIX_SIZE - tlen, ".%03d", tb.millitm);		\
												va_list ap;																\
												va_start(ap, fmt);														\
												vsnprintf(msg + strlen(msg), LOG_MESSAGE_SIZE, fmt, ap);				\
												va_end(ap);																\
											} while (0)

int Log::v(const char *fmt, ...)
{
	if (VERBOSE < mLogPri) {
		return 0;
	}

	char msg[LOG_PREFIX_SIZE + LOG_MESSAGE_SIZE];
	LogBuilder(fmt, msg);

	return log(msg);
}

int Log::d(const char *fmt, ...)
{
	if (DEBUG < mLogPri) {
		return 0;
	}

	char msg[LOG_PREFIX_SIZE + LOG_MESSAGE_SIZE];
	LogBuilder(fmt, msg);

	return log(msg);
}

int Log::i(const char *fmt, ...)
{
	if (INFO < mLogPri) {
		return 0;
	}

	char msg[LOG_PREFIX_SIZE + LOG_MESSAGE_SIZE];
	LogBuilder(fmt, msg);

	return log(msg);
}

int Log::w(const char *fmt, ...)
{
	if (WARN < mLogPri) {
		return 0;
	}

	char msg[LOG_PREFIX_SIZE + LOG_MESSAGE_SIZE];
	LogBuilder(fmt, msg);

	return log(msg);
}

int Log::e(const char *fmt, ...)
{
	if (ERROR < mLogPri) {
		return 0;
	}

	char msg[LOG_PREFIX_SIZE + LOG_MESSAGE_SIZE];
	LogBuilder(fmt, msg);

	return log(msg);
}

int Log::a(const char *fmt, ...)
{
	if (ASSERT < mLogPri) {
		return 0;
	}

	char msg[LOG_PREFIX_SIZE + LOG_MESSAGE_SIZE];
	LogBuilder(fmt, msg);

	return log(msg);
}

int Log::log(const char *msg)
{
	int loglen = 0;

	synchronized (mLogMutex) {
		if(SystemClock::Today(SystemClockDateFormat)>mLastLogDay)
		{
			if(mLogFd != 0 && mLogFd != STDOUT_FILENO && mLogFd != STDERR_FILENO)
			{
				close(mLogFd);
			}

			mLogFd = 0;

			mLogFd = InitLogFd();
		}

		loglen = write(mLogFd, msg, strlen(msg));
		//printf("%s\n",msg); //add for test
		mLastLogDay = SystemClock::Today(SystemClockDateFormat);
	}

    return loglen;
}

std::string  Log::mlogfilepathdir = "";
std::string Log::mlogfilename = "";
std::string Log::mmodelname = "";
std::string Log::mLastLogDay = "";
int Log::mLogFd = 0;
int Log::mLogReduce = 0;
Log::LogPriority Log::mLogPri = Log::VERBOSE;

Mutex Log::mLogMutex;

Log::Log()
{

}

// logfilename:    module_yyyymmddhhMMss
bool Log::LogInit(std::string logpath,std::string modename,LogPriority pri)
{
	mlogfilepathdir = logpath;
	mmodelname = modename;
	mLogPri = pri;
	mLastLogDay = SystemClock::Today(SystemClockDateFormat);

	mLogFd = Log::InitLogFd();

	return true;
}

int Log::InitLogFd()
{
	char logfile[128] = { '\0' };

	if(mLogFd == 0)
	{
		if (strcasecmp(mmodelname.c_str(), "stdout") == 0) {
			mLogFd = STDOUT_FILENO;

		}
		else if (strcasecmp(mmodelname.c_str(), "stderr") == 0) {
			mLogFd = STDERR_FILENO;
		}

		if(mLogFd > 0)
			return mLogFd;
	}


	if(mLogFd != 0 && mlogfilename.size() > 0)
	{
		std::size_t namepos = mlogfilename.find_last_of('_');
		if(namepos != std::string::npos)
		{
			mlogfilename = mlogfilename.substr(0,namepos) + "_" + SystemClock::Today(SystemClockTMFormatNoSpace) + ".log";
		}
		else
		{
			mlogfilename = mlogfilename  + "_" + SystemClock::Today(SystemClockTMFormatNoSpace) + ".log";
		}

		strcpy(logfile, mlogfilename.c_str());

	}
	else
	{
		strcpy(logfile, (mmodelname + "_" + SystemClock::Today(SystemClockTMFormatNoSpace) + ".log").c_str());
	}


	char fullname[1024];
	if (logfile[0] != '/') {
		snprintf(fullname, sizeof(fullname), "%s/%s", mlogfilepathdir.c_str(), logfile);
	}
	else {
		strcpy(fullname, logfile);
	}

	printf("#######  %s\n",fullname);
	mlogfilename = logfile;

	char *dirch = rindex(fullname, '/');
	if (dirch != NULL) {
		*dirch = '\0';
		FileSysUtils::MakeDir(fullname);

		*dirch = '/';
	}

	if (FileSysUtils::Accessible(fullname, FileSysUtils::FR_OK)) {
		char newname[1024];
		snprintf(newname, sizeof(newname), "%s-%s", fullname, SystemClock::Today(SystemClockTMFormatNoSpace).c_str());
		rename(fullname, newname);
	}

	mLogFd = open(fullname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (mLogFd < 0) {
		mLogFd = STDOUT_FILENO;

		fprintf(stderr, "Open log file %s failed.\n", fullname);
		fprintf(stderr, "Using stdout for log.\n");
	}


	return mLogFd;
}

std::string Log::GetLogPathDir()
{
	return mlogfilepathdir;
}

std::string Log::GetLogFile()
{
	return mlogfilename;
}

Log::LogPriority Log::InitLogPri(const char *logpri)
{
	//const char *logpri = EnvSetting::Get(LOG_PRIORITY_ENV, "DEBUG");
	if (strcasecmp(logpri, "VERBOSE") == 0) {
		mLogPri = VERBOSE;
	}
	else if (strcasecmp(logpri, "DEBUG") == 0) {
		mLogPri = DEBUG;
	}
	else if (strcasecmp(logpri, "INFO") == 0) {
		mLogPri = INFO;
	}
	else if (strcasecmp(logpri, "WARN") == 0) {
		mLogPri = WARN;
	}
	else if (strcasecmp(logpri, "ERROR") == 0) {
		mLogPri = ERROR;
	}
	else if (strcasecmp(logpri, "ASSERT") == 0) {
		mLogPri = ASSERT;
	}
	else {
		mLogPri = DEBUG;

		fprintf(stderr, "Bad log priority %s.\n", logpri);
		fprintf(stderr, "Using DEBUG priority for log.\n");
	}

	return mLogPri;
}

void Log::setLogReduce(int flag)
{
	mLogReduce = flag;
}
int Log::getLogReduce()
{
	return mLogReduce;
}
