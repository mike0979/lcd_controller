#include "hd_command.h"
#include "tcp_msg_help.h"
#include "config/configparser.h"

//一个cpp里只能写一次，这个会在程序开始时运行。比main函数还早
STATIC_REGISTRATION(hd_command)
{
    bj_msg::GetHandlers()["M41"] = &hd_command::OnCommand;
}

void hd_command::OnCommand(bj_msg *msg, ServerTcpConnection *conn)
{
	int index = 0;
    string control = TcpGetString(msg->Data, index, 2); //控制指令
    string attach = TcpGetString(msg->Data, index, 32);//附加值
	bj_msg reply;
    reply.Copy(*msg);
    reply.Command = "A41";
    reply.SetData("", 131);
    index = 0;
    TcpSetString(reply.Data, index, "100", 3);
	TcpSetString(reply.Data, index, "ok", 128);

    char data[256];//请保证自己buffer够用
    int len = reply.Write(data);
    conn->Send(data, len);
    Exec(control,attach);

}

int hd_command::Exec(const std::string& cmd,const std::string& cmdParam)
{
	int cmdInt=stoi(cmd);
	switch (cmdInt) {
		case 1://startup
			return 0;
		case 2://shutdown
			system("sleep 3&&shutdown -h now");
			return 1;
		case 3://reboot
			system("sleep 3&&reboot -f");
			return 1;
		case 4://setVolumn
			{
				int volumn=stoi(cmdParam);
				volumn=volumn*65536/100;
				system((string("amixer sset Master ")+to_string(volumn)).c_str());
				return 1;
			}
		case 5://mute
			system("amixer sset Master mute");
			return 1;
		case 6://unmute
			system("amixer sset Master unmute");
			return 1;
		case 8://open port
		case 9://close port
		case 10://open all port
		case 11://close all port
			return 0;
		case 12://open screen
			return 0;
		case 13://close screen
			return 0;
		case 14://set brightness
			return 0;
		default:
			return 0;
	}
	return 0;
}
