#include "tcp_client.h"
#include <log/log_manager.h>

using namespace waxberry;

void Client::OnConnected(const connection_ptr & ptr)
{
	if(Connected)
	{
		Connected(this);
	}
}
