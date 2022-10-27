#include "SystemClock.h"

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

SystemClockTM::SystemClockTM()
{
	mSec = 0;
	mMin = 0;
	mHour = 0;
	mDay = 0;
	mMon = 0;
	mYear = 0;
}

void SystemClockTM::print()
{
	printf("%04d%02d%02d %02d%02d%02d\n", mYear, mMon, mDay, mHour, mMin, mSec);
}

uint64_t SystemClock::SystemTime(ClockID clock)
{
    static const clockid_t clocks[] = {
        CLOCK_REALTIME,
        CLOCK_MONOTONIC,
        CLOCK_PROCESS_CPUTIME_ID,
        CLOCK_THREAD_CPUTIME_ID
    };

    struct timespec t;
    clock_gettime(clocks[clock], &t);

    return (t.tv_sec) * 1000000000LL + t.tv_nsec;
}

uint64_t SystemClock::SystemClock::UptimeMillis()
{
	return SystemTime(SYSTEM_TIME_MONOTONIC) / 1000000;
}

bool SystemClock::StrToTM(const std::string &str, SystemClockTM &tm, const char *fmt)
{
	struct tm stm;
	memset(&stm, 0, sizeof(struct tm));

#if 1		// it's NOT a good idea
	std::string strH = str;
	if (strH.size() < 10)
	{
		return false;
	}
	if (strcmp(fmt, SystemClockTMFormat) == 0) {
		if (strH[9] == '2' && strH[10] == '4') {
			strH[10] = '3';
			strH[11] = '5';
			strH[12] = '9';
			strH[13] = '5';
			strH[14] = '9';
		}
	}
	else if (strcmp(fmt, SystemClockTimeFormat) == 0) {
		if (strH[0] == '2' && strH[1] == '4') {
			strH[1] = '3';
			strH[2] = '5';
			strH[3] = '9';
			strH[4] = '5';
			strH[5] = '9';
		}

		time_t t = time(NULL);
		struct tm *pstm = localtime(&t);

		stm.tm_mday = pstm->tm_mday;
		stm.tm_mon = pstm->tm_mon;
		stm.tm_year = pstm->tm_year;
	}

	if (strptime(strH.c_str(), fmt, &stm) != NULL) {
#else
	if (strptime(str.c_str(), fmt, &stm) != NULL) {
#endif
		tm.mSec = stm.tm_sec;
		tm.mMin = stm.tm_min;
		tm.mHour = stm.tm_hour;
		tm.mDay = stm.tm_mday;
		tm.mMon = stm.tm_mon + 1;
		tm.mYear = stm.tm_year + 1900;
		return true;
	}
	else {
		return false;
	}
}

bool SystemClock::TMToUptimeMillis(const SystemClockTM &tm, uint64_t &uptime)
{
	bool monotonic = true;

	struct tm stm;
	memset(&stm, 0, sizeof(struct tm));

	stm.tm_sec = tm.mSec;
	stm.tm_min = tm.mMin;
	stm.tm_hour = tm.mHour;
	stm.tm_mday = tm.mDay;
	stm.tm_mon = tm.mMon - 1;
	stm.tm_year = tm.mYear - 1900;

	time_t currSecs = time(NULL);
	time_t tmSecs = mktime(&stm);

	time_t upSecs = currSecs - UptimeMillis() / 1000;
	uint64_t diff = 0;
	if (upSecs > tmSecs) {
		diff = upSecs - tmSecs;
		monotonic = false;
	}
	else {
		diff = tmSecs - upSecs;
		monotonic = true;
	}

	uptime = diff * 1000;
	return monotonic;
}

bool SystemClock::StrToUptimeMillis(const std::string &str, uint64_t &uptime, const char *fmt)
{
	SystemClockTM tm;

	if (StrToTM(str, tm, fmt) == false) {
		uptime = 0;

		printf("StrToUptimeMillis failed. Bad string.\n");
		return false;
	}

	return TMToUptimeMillis(tm, uptime);
}

std::string SystemClock::Today(const char *fmt)
{
	char today[64];
	today[0] = '\0';

	time_t t = time(NULL);
	strftime(today, sizeof(today), fmt, localtime(&t));


	return today;
}

struct tm *SystemClock::CurrentDateTime()
{
	time_t t = time(NULL);
	return localtime(&t);
}


uint64_t SystemClock::EpochSecPassed(uint64_t sec)
{
	return time(NULL) - sec;
}

void SystemClock::Sleep(uint64_t ms)
{
	struct timespec req, rem;
	req.tv_sec = ms / 1000;
	req.tv_nsec = (ms % 1000) * 1000000;

	while (nanosleep(&req, &rem) != 0) {
		if (errno == EINTR) {
			memcpy(&req, &rem, sizeof(struct timespec));
		}
		else {
			break;
		}
	}
}

bool SystemClock::NtpDate(const std::string &ntpserver, float &offset)
{
	bool update = false;

	std::string cmd = "ntpdate ";
	cmd.append(ntpserver);

	FILE *freport = popen(cmd.c_str(), "r");
	if (freport == NULL) {
		printf("Failed to execute ntpdate.\n");
	}
	else {
		char echo[1024];

		if (fgets(echo, sizeof(echo), freport) != NULL) {
			char *ntp = strstr(echo, "offset");
			if (ntp != NULL) {
				if (sscanf(ntp, "offset %f sec", &offset) == 1) {
					update = true;

					printf("Adjust time offset %f second.\n", offset);
				}
			}

			if (update == false) {
				printf("%s\n", echo);
			}
		}

		pclose(freport);
	}

	return update;
}

const char *SystemClockTM::TAG = "SystemClockTM";
const char *SystemClock::TAG = "SystemClock";
