#include "bj_pis/bj_log/log_transfer.h"
#include "bj_pis/utils/datetime.h"
#include <algorithm>
#include <string.h>

using namespace std;

LogTransfer::LogTransfer()
{
	string path="/home/log/lcd_player/lcd_";
	path+=datetime::Now().ToString("%Y%m%d")+".log";
	fp_ = fopen(path.c_str(), "a");
}

LogTransfer::~LogTransfer()
{
	fclose(fp_);
}
LogTransfer& LogTransfer::GetInstance()
{
	static LogTransfer log_tranfer_;
	return log_tranfer_;
}

#define LOGHEAD(LEVEL,TYPE) \
		fputc('[',fp_);fputs(LEVEL, fp_);fputc(']',fp_);\
		fputc('[',fp_);fputs(TYPE, fp_);fputc(']',fp_);\
		fputc('[',fp_);fwrite(datetime::Now().ToString("%Y-%m-%d %H:%M:%S").c_str(),1,19, fp_);fputc(']',fp_);
#define LOGRAWCONTENT(CONTENT,SIZE) fputc('<',fp_);fwrite(CONTENT,1,SIZE, fp_);fputs(">\r\n",fp_);fflush(fp_);
#define LOGCONTENT(CONTENT) fputc('[',fp_);fputs(CONTENT, fp_);fputs("]\r\n",fp_);fflush(fp_);

//因为京投实验室的软件在检查日志时无法识别（消息内容中的）换行符，所以换成空格
void LogTransfer::RemoveCRLF(char* content,int size)
{
	if(size>1)
		{
			for(int i=1;i<size;i++)
			{
				if((content[i]=='\r'||content[i]=='\n')&&content[i-1]==0)//仅适用于gb2312，肯定不适用于utf-8，也许适用于utf16
				{
					content[i]=' ';//把\r\n换成空格
				}
			}
		}
}

void LogTransfer::LogReceive(const char* src_content, int size) const
{
	char content[size+1];
	content[size]='\0';
	memcpy(content,src_content,(unsigned long int)size);
	RemoveCRLF(content,size);
	LOGHEAD("Info   ","RAW");
	LOGRAWCONTENT(content,size);
	LogTrans(content, size);
}

void LogTransfer::LogSend(const char* src_content, int size) const
{
	char content[size+1];
	content[size]='\0';
	memcpy(content,src_content,(unsigned long int)size);
	RemoveCRLF(content,size);
	LOGHEAD("Info   ","SED");
	LOGRAWCONTENT(content,size);
}

void LogTransfer::LogSystem(const char* src_content,const char* level) const
{
	int size=strlen(src_content);
	char content[size+1];
	content[size]='\0';
	memcpy(content,src_content,(unsigned long int)size);
	RemoveCRLF(content,size);
	char extendLevel[8]={' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
	memcpy(extendLevel,level,strlen(level));
	LOGHEAD(extendLevel,"SYS");
	LOGCONTENT(content);
}

void LogTransfer::LogTrans(const char* src_content, int size) const
{
	if(size<28) return;
	string cmd(src_content+25,3);
	std::map<std::string,pFuncTranslog>::const_iterator iter = callbacks.find(cmd);
	if(iter!=callbacks.end())
	{
		LOGHEAD("Info   ","TRA");
		fputc('[',fp_);
		iter->second(src_content, fp_);
		fputs("]\r\n",fp_);
		fflush(fp_);
	}
}
