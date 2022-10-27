#ifndef SYSTEMCLOCK_H_
#define SYSTEMCLOCK_H_

#include <stdint.h>
#include <string>

#define SystemClockTMFormat			"%Y%m%d %H%M%S"
#define SystemClockTMFormatNoSpace	"%Y%m%d%H%M%S"
#define SystemClockTMFormatDpyLog	"%m/%d/%Y %H:%M:%S"
#define SystemClockDateFormat		"%Y%m%d"
#define SystemClockTimeFormat		"%H%M%S"

class SystemClockTM {
public:
	int mSec;
	int mMin;
	int mHour;
	int mDay;
	int mMon;
	int mYear;

public:
	SystemClockTM();
	void print();

private:
	static const char *TAG;
};

class SystemClock {
public:
    enum ClockID {
        SYSTEM_TIME_REALTIME = 0,
        SYSTEM_TIME_MONOTONIC = 1,
        SYSTEM_TIME_PROCESS = 2,
        SYSTEM_TIME_THREAD = 3
    };

    // retrieve the time of the specified clock, in nanoseconds
	static uint64_t SystemTime(ClockID clock);
	// retrieve monotonic clock, in milliseconds
	static uint64_t UptimeMillis();

	// convert string to SystemClockTM, return true if success
	static bool StrToTM(const std::string &str, SystemClockTM &tm, const char *fmt = SystemClockTMFormat);
	// calculate difference between SystemClockTM and monotonic clock, return true if tm is after monotonic clock
	static bool TMToUptimeMillis(const SystemClockTM &tm, uint64_t &uptime);
	// calculate difference between string and monotonic clock, return true if tm is after monotonic clock
	static bool StrToUptimeMillis(const std::string &str, uint64_t &uptime, const char *fmt = SystemClockTMFormat);

	// get date/time string
	static std::string Today(const char *fmt = SystemClockDateFormat);

	// get current datetime
	static struct tm *CurrentDateTime();

	// returns difference between the time as the number of seconds since the Epoch and parameter sec
	static uint64_t EpochSecPassed(uint64_t sec);

	// sleep, in milliseconds
	static void Sleep(uint64_t ms);

	// ntp date
	static bool NtpDate(const std::string &ntpserver, float &offset);

private:
	static const char *TAG;
};

#endif /* SYSTEMCLOCK_H_ */
