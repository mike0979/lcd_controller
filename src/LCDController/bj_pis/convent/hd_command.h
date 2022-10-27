#ifndef HD_COMMAND_H_
#define HD_COMMAND_H_
#include "bj_msg.h"
#include "CommonDef.h"

class hd_command
{
public:
	hd_command();
	~hd_command();
	
	static void OnCommand(bj_msg *msg, ServerTcpConnection *conn);

	static int Exec(const std::string& cmd,const std::string& cmdParam);
};


#endif // HD_COMMAND_H_
