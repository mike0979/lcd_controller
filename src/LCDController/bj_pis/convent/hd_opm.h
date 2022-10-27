#ifndef HD_OPM_H_
#define HD_OPM_H_

#include <time.h>
#include <string>
#include "bj_pis/convent/bj_file_info.h"
#include <boost/regex.hpp>

class bj_msg;
class ServerTcpConnection;

struct OpmMsg
{
	time_t start_time;
	time_t end_time;
	int priority;
	int info_len;
	std::string info_text;
	string tag;
	bj_file_info file_info;
	//char priorities[10 + 1];

	string Variable2String(int id)
	{
		return to_string(id) + '#'
			+ to_string(start_time) + '#'
			+ to_string(end_time) + '#'
			+ to_string(priority) + '#'
			+ to_string(info_len) + '#'
			+ info_text + '#'
			+ tag;
	}

	int String2Variable(const string& msg)
	{
		if (msg.empty())
		{
			return -1;
		}
		boost::regex ws_re("#");
		vector<string> vec(boost::sregex_token_iterator(msg.begin(), msg.end(), ws_re, -1), boost::sregex_token_iterator());
		printf("msg is %s\n", msg.c_str());
		start_time = stol(vec[1]);
		end_time = stol(vec[2]);
		priority = stoi(vec[3]);
		info_len = stoi(vec[4]);
		info_text = vec[5];
		tag = vec[6];
		return stoi(vec[0]);
	}
};

class HdOpm
{
public:
	static void PublishOpm(bj_msg* msg, ServerTcpConnection* conn);
	static void CancelOpm(bj_msg* msg, ServerTcpConnection* conn);
	static void SendProcess(OpmMsg& opm_msg);
	static void CancelProcess(const string& tag);
	static void CancelProcess(const char* priorities);
	static void WriteToFile();
	static void ReadFromFile();
	static void LoadLocal();
private:
	static FILE* fp_;
	static map<int, OpmMsg> msgs_;
};
#endif
