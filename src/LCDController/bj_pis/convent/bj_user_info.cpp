#include "bj_user_info.h"
#include "tcp_msg_help.h"

vector<shared_ptr<bj_user_info>> bj_user_info::sc_users;

int bj_user_info::Parse(const char* data)
{
	int index = 0;
    user_code = TcpGetString(data, index, 12);
    login_account = TcpGetString(data, index, 12);
    name = TcpGetString(data, index, 32);
    password = TcpGetString(data, index, 32);
    department = TcpGetString(data, index, 32);
    telephone = TcpGetString(data, index, 12);
    email = TcpGetString(data, index, 32);
	status = TcpGetInt(data, index, 1);
	permission = TcpGetString(data, index, 32);
    update_time = TcpGetDateTime(data, index);
    return index;
}

int bj_user_info::Write(char* data)
{
	int index = 0;
    TcpSetString(data, index, user_code, 12);
    TcpSetString(data, index, login_account, 12);
    TcpSetString(data, index, name, 32);
    TcpSetString(data, index, password, 32);
    TcpSetString(data, index, department, 32);
    TcpSetString(data, index, telephone, 12);
    TcpSetString(data, index, email, 32);
    TcpSetInt(data, index, status, 1);
    TcpSetString(data, index, permission, 32);
    TcpSetDateTime(data, index, update_time);
    return index;
}

vector<shared_ptr<bj_user_info>> bj_user_info::GetUsers(const string& station_code)
{
	vector<shared_ptr<bj_user_info>> result;
    for (shared_ptr<bj_user_info>& sp : sc_users)
    {
        if(sp->station_code==station_code)
        {
            result.push_back(sp);
        }
    }
    return result;
}
