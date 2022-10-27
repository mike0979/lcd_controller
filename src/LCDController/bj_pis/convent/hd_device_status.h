#ifndef HD_DEVICE_STATUS_H_
#define HD_DEVICE_STATUS_H_

#include <net/tcp_connection.h>
#include <vector>
#include "bj_msg.h"

using boost::asio::ip::tcp;

typedef struct st_subscriber_ {
	tcp::endpoint remote;
	bj_msg reply;
	string device_code;
	string log_path;
	time_t last;
	int duration;
	ServerTcpConnection* conn;
} st_subscriber;

class hd_device_status
{
public:
	hd_device_status();
	~hd_device_status();
	static void OnHeartBeat(bj_msg *msg, ServerTcpConnection *conn);
	static void OnSubscribeStatus(bj_msg *msg, ServerTcpConnection *conn);
	static void SendStatus(st_subscriber &suber);
	static void OnSubscribeLog(bj_msg *msg, ServerTcpConnection *conn);
	static void SendLog(st_subscriber &suber);
	static void OnConnected(bj_msg *msg, ServerTcpConnection *conn);
	static void OnDisConnected(bj_msg *msg, ServerTcpConnection *conn);

	static vector<st_subscriber> StatusSubers;
	static vector<st_subscriber> LogSubers;

	static const char* TAG;
};


#endif // HD_DEVICE_STATUS_H_
