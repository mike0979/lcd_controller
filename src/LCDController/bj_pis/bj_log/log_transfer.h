#pragma once
#include <string>
#include <map>

typedef void(*pFuncTranslog)(const char* content,FILE* fp);

class LogTransfer
{
public:
	LogTransfer();
	~LogTransfer();
	static LogTransfer& GetInstance();
	void LogReceive(const char* src_content, int size) const;
	void LogSend(const char* src_content, int size) const;
	void LogSystem(const char* src_content,const char* level) const;
	std::map<std::string,pFuncTranslog> callbacks;
private:
	FILE* fp_;
	void LogTrans(const char* src_content, int size) const;
	static void RemoveCRLF(char* content,int size);
};
