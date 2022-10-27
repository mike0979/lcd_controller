#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string>

#include "Mutex.h"

class Log {

public:
	enum LogPriority {
    	VERBOSE = 0,
    	DEBUG,
    	INFO,
        WARN,
        ERROR,
        ASSERT,
    };

    static int v(const char *fmt, ...);
    static int d(const char *fmt, ...);
    static int i(const char *fmt, ...);
    static int w(const char *fmt, ...);
    static int e(const char *fmt, ...);
    static int a(const char *fmt, ...);

    static std::string GetLogPathDir();
    static std::string GetLogFile();
    static bool LogInit(std::string logpath,std::string modelname, LogPriority pri = VERBOSE);

    static void setLogReduce(int flag);
    static int getLogReduce();

private:

    static int mLogFd;
    static LogPriority mLogPri;
    static Mutex mLogMutex;
    static std::string mlogfilepathdir;
    static std::string mlogfilename;
    static std::string mmodelname;
    static std::string mLastLogDay;
    Log();

    static int InitLogFd();
    static LogPriority InitLogPri(const char *logpri);

    static int log(const char *msg);
    static int mLogReduce;
};


#define LOG_FORMAT_SIZE		1024

#define LogV(fmt, ...)		do {																	\
								char fmt_[LOG_FORMAT_SIZE];											\
								if(Log::getLogReduce()==0) \
								{                                               \
									snprintf(fmt_, sizeof(fmt_), "%%s %%s : %%s:%%d[%%s] -| %s", fmt);			\
									Log::v(fmt_, "V", TAG, __FILE__, __LINE__, __func__, ##__VA_ARGS__);		\
								}else{                   \
									snprintf(fmt_, sizeof(fmt_), "%%s %s", fmt);			\
									Log::v(fmt_,"V", ##__VA_ARGS__);		\
								} \
							} while (0)

#define LogD(fmt, ...)		do {																	\
								char fmt_[LOG_FORMAT_SIZE];											\
								if(Log::getLogReduce()==0) \
								{                                               \
									snprintf(fmt_, sizeof(fmt_), "%%s %%s : %%s:%%d[%%s] -| %s", fmt);			\
									Log::d(fmt_, "D", TAG, __FILE__, __LINE__, __func__, ##__VA_ARGS__);		\
								}else{                   \
									snprintf(fmt_, sizeof(fmt_), "%%s %s", fmt);			\
									Log::d(fmt_,"D", ##__VA_ARGS__);		\
								} \
							} while (0)

#define LogI(fmt, ...)		do {																	\
								char fmt_[LOG_FORMAT_SIZE];											\
								if(Log::getLogReduce()==0) \
								{                                               \
									snprintf(fmt_, sizeof(fmt_), "%%s %%s : %%s:%%d[%%s] -| %s", fmt);			\
									Log::i(fmt_, "I", TAG, __FILE__, __LINE__, __func__, ##__VA_ARGS__);		\
								}else{                   \
									snprintf(fmt_, sizeof(fmt_), "%%s %s", fmt);			\
									Log::i(fmt_,"I", ##__VA_ARGS__);		\
								} \
							} while (0)

#define LogW(fmt, ...)		do {																	\
								char fmt_[LOG_FORMAT_SIZE];											\
								if(Log::getLogReduce()==0) \
								{                                               \
									snprintf(fmt_, sizeof(fmt_), "%%s %%s : %%s:%%d[%%s] -| %s", fmt);			\
									Log::w(fmt_, "W", TAG, __FILE__, __LINE__, __func__, ##__VA_ARGS__);		\
								}else{                   \
									snprintf(fmt_, sizeof(fmt_), "%%s %s", fmt);			\
									Log::w(fmt_,"W", ##__VA_ARGS__);		\
								} \
							} while (0)

#define LogE(fmt, ...)		do {																	\
								char fmt_[LOG_FORMAT_SIZE];											\
								if(Log::getLogReduce()==0) \
								{                                               \
									snprintf(fmt_, sizeof(fmt_), "%%s %%s : %%s:%%d[%%s] -| %s", fmt);			\
									Log::e(fmt_, "E", TAG, __FILE__, __LINE__, __func__, ##__VA_ARGS__);		\
								}else{                   \
									snprintf(fmt_, sizeof(fmt_), "%%s %s", fmt);			\
									Log::e(fmt_,"E", ##__VA_ARGS__);		\
								} \
							} while (0)

#define LogA(fmt, ...)		do {																	\
								char fmt_[LOG_FORMAT_SIZE];											\
								if(Log::getLogReduce()==0) \
								{                                               \
									snprintf(fmt_, sizeof(fmt_), "%%s %%s : %%s:%%d[%%s] -| %s", fmt);			\
									Log::a(fmt_, "A", TAG, __FILE__, __LINE__, __func__, ##__VA_ARGS__);		\
								}else{                   \
									snprintf(fmt_, sizeof(fmt_), "%%s %s", fmt);			\
									Log::a(fmt_,"A", ##__VA_ARGS__);		\
								} \
								assert(false);														\
							} while (0)

#endif /* LOG_H_ */
