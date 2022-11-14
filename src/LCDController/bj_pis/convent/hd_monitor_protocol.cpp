#include "hd_monitor_protocol.h"
#include "tcp_msg_help.h"
#include <boost/filesystem.hpp>
#include "bj_pis/thread/tcp_client_thread.h"
#include "bj_pis/tcp_client/tcp_client.h"

STATIC_REGISTRATION(HdMonirorProtocol)
{
	bj_msg::GetHandlers()["M72"] = &HdMonirorProtocol::OnMonirorProtocol;
}

void HdMonirorProtocol::OnMonirorProtocol(bj_msg* msg, ServerTcpConnection* conn)
{
	int index = 0;
	const string& protocol_address = TcpGetString(msg->Data, index, 32);

	const string& file = GetLatestFile("/home/workspace/log/snapshot/");
	if(file.size()>0)
	{
		auto file_size = boost::filesystem::file_size("/home/workspace/log/snapshot/" + file);
			int total_index = file_size / 1490;
			if (file_size % 1490 != 0)
			{
				total_index++;
			}

			auto index1 = protocol_address.find_last_of("/");
			auto index2 = protocol_address.find_last_of(":");
			const string& ip = protocol_address.substr(index1 + 1, index2 - index1 - 1);
			const string& port = protocol_address.substr(index2 + 1);
			printf("________________________%s_______________%s_____________\n", ip.c_str(), port.c_str());
			printf("________________________%s_______________%lu_____________\n", file.c_str(), file_size);
			TcpClientThread client_thread(ip.c_str(), stoi(port));
			client_thread.Connected=[file,total_index](Client* client)
			{
				FILE* fp = fopen(("/home/workspace/log/snapshot/" + file).c_str(), "rb");
				char send_buf[1498] = { '\0' };
				send_buf[0] = char(0x7F);
				for (int i = 1; i <= total_index; i++)
				{
					*(uint16_t*)(send_buf+2) = i;
					*(uint16_t*)(send_buf+4) = total_index;
					int c = fread(send_buf + 8, 1, 1490, fp);
					*(uint16_t*)(send_buf+6) = c;
					if (i == 1)
					{
						send_buf[1] = char(0);
					}
					else if (i == total_index)
					{
						send_buf[1] = char(2);
					}
					else
					{
						send_buf[1] = char(1);
					}
					if(i == total_index)
					{
						client->Connection()->Write(send_buf, c+8,[client](int status)
						{
							client->Stop();
						});
					}
					else
					{
						client->Connection()->Write(send_buf, c+8);
					}
				}
				fclose(fp);
				//client_thread.Dispose();
			};
			client_thread.Start();
	}

	bj_msg reply;
	reply.Copy(*msg);
	reply.Command = "A72";
	reply.SystemCode = "10"; //SPIS

	reply.SetData("", 12);
	index = 0;
	TcpSetString(reply.Data, index, "jpeg", 12);

	char data[256];//请保证自己buffer够用
	int len = reply.Write(data);
	conn->Send(data, len);
}
