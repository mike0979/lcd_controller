#ifndef BJ_MSG_HEADER_H_
#define BJ_MSG_HEADER_H_
#include <string>
#include <vector>
#include "bj_pis/tcp_server/server_tcp_connection.h"
#include <map>

using namespace std;
class bj_msg;
typedef void (*pBjHandler) (bj_msg* header,ServerTcpConnection* conn);

class bj_msg
{
public:
	bj_msg();
	virtual ~bj_msg();

	int SQN;
	string Version;
	string SystemCode;
	time_t TimeStamp;
	string Command;
	int DataLength;
	char* Data;

	string TaskID;
	string AreaLine;
	vector<int> AreaStation;//车站次序
	vector<int> AreaRegion;//车站内区域

	void Parse(const char* data);
	int Write(char* data);
	void SetData(const char* data,int len);
	static std::map<string,pBjHandler>& GetHandlers();

	void Copy(bj_msg& msg);

	static int GetSendSQN();
private:
	static int _sqn_for_send;
};

#define MARCROCONCAT_CMPL(x,y) x##y
#define MARCROCONCAT(x,y) MARCROCONCAT_CMPL(x,y)
#define STATIC_REGISTRATION(unique_id)																\
static void MARCROCONCAT(auto_register_function_,unique_id) ();										\
namespace																							\
{																									\
    struct MARCROCONCAT(auto__register__,unique_id)													\
    {																								\
        MARCROCONCAT(auto__register__,unique_id)()													\
        {																							\
            MARCROCONCAT(auto_register_function_,unique_id)();										\
        }																							\
    };																								\
}																									\
static const MARCROCONCAT(auto__register__,unique_id) MARCROCONCAT(id_auto_register__,unique_id);	\
static void MARCROCONCAT(auto_register_function_,unique_id)()

#endif
