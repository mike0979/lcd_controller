#include "client_tcp_connection.h"

using namespace waxberry;

ClientTcpConnection::ClientTcpConnection(BasicHost& host) : WaxberryTcpConnection<Message>(host)
{

}

void ClientTcpConnection::OnMessage(Message& msg)
{

}

