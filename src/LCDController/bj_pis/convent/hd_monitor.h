#ifndef HD_MONITOR_H_
#define HD_MONITOR_H_
#include "bj_msg.h"

class hd_monitor
{
public:
	hd_monitor();
	~hd_monitor();
	
	static void OnMonitor(bj_msg *msg, ServerTcpConnection *conn);
	

};

#endif // HD_MONITOR_H_
