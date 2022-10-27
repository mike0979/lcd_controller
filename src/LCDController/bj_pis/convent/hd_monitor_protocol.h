#pragma once

#include "bj_msg.h"
#include "bj_pis/tcp_server/server_tcp_connection.h"

class HdMonirorProtocol
{
public:
	static void OnMonirorProtocol(bj_msg* msg, ServerTcpConnection* conn);
};
