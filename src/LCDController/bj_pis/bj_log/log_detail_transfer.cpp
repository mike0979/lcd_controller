#include "bj_pis/convent/bj_msg.h"
#include "log_transfer.h"
#include "bj_pis/convent/tcp_msg_help.h"

namespace bj_log
{
	#define LOG_REG_DETAIL(CMD) \
			static void log_trans_##CMD(const char* content,FILE* fp);\
			struct struct_auto_reg_##CMD\
			{\
				struct_auto_reg_##CMD()\
				{\
					LogTransfer::GetInstance().callbacks.insert(make_pair(#CMD,&bj_log::log_trans_##CMD));\
				}\
			};\
			struct struct_auto_reg_##CMD obj_auto_reg_##CMD;\
			static void log_trans_##CMD(const char* content,FILE* fp)

	static void WriteKV(FILE *fp, const char *name, const string &value)
	{
		string key = code_convent(name, strlen(name), "UTF-8", "GB2312");
		fputs(key.c_str(), fp);
		fputc('(', fp);
		fwrite(value.c_str(), 1, value.size(), fp);
		fputc(')', fp);
	}

	static int WriteHeader(const char *content, FILE *fp)
	{
		bj_msg msg;
		msg.Parse(content);
		WriteKV(fp, "发送系统", msg.SystemCode);
		WriteKV(fp, "协议版本", msg.Version);
		WriteKV(fp, "指令", msg.Command);
		if (msg.Command[0] == 'M')
		{
			WriteKV(fp, "任务", msg.TaskID);
			WriteKV(fp, "区域", string(content + 42, 32));
		}
		WriteKV(fp, "长度", to_string(msg.DataLength));
		return msg.Command[0] == 'M' ? 80 : 34;
	}

	static void WriteFileInfo(FILE *fp, const char *content, int &index)
	{
		WriteKV(fp, "文件URL", TcpGetString(content, index, 128));
		WriteKV(fp, "文件名称", TcpGetString(content, index, 80));
		WriteKV(fp, "文件类型", TcpGetString(content, index, 2));
		WriteKV(fp, "Md5校验值", TcpGetString(content, index, 32));
	}

	LOG_REG_DETAIL(M11)
	{
		int headLen = WriteHeader(content, fp);
		WriteKV(fp, "信息tag", TcpGetString(content, headLen, 14));
		WriteKV(fp, "开始时间", TcpGetString(content, headLen, 14));
		WriteKV(fp, "结束时间", TcpGetString(content, headLen, 14));
		int priority = TcpGetInt(content, headLen, 2);
		WriteKV(fp, "优先级", to_string(priority));
		if (priority == 6)
		{
			WriteFileInfo(fp, content, headLen);
		}
		else
		{
			int textLen = TcpGetInt(content, headLen, 4);
			WriteKV(fp, "信息长度", to_string(textLen));
			string s = code_convent(content + headLen, textLen, "UTF-16LE", "GB2312");
			WriteKV(fp, "信息文本", s);
		}
	}

	LOG_REG_DETAIL(M21)
	{
		int headLen = WriteHeader(content, fp);
		WriteKV(fp, "信息tag", TcpGetString(content, headLen, 14));
		WriteKV(fp, "优先级1-10", TcpGetString(content, headLen, 10));
	}

	LOG_REG_DETAIL(M51)
	{
		int headLen = WriteHeader(content, fp);
		WriteKV(fp, "计划名称", TcpGetString(content, headLen, 64));
		WriteKV(fp, "计划ID", TcpGetString(content, headLen, 32));
		WriteKV(fp, "最后更新时间", TcpGetString(content, headLen, 14));
		int FileNumber = TcpGetInt(content, headLen, 3);
		WriteKV(fp, "所含文件数量", to_string(FileNumber));
		for (int i = 0; i < FileNumber ; i++ )
		{
			WriteFileInfo(fp, content,headLen);
			}
	}

LOG_REG_DETAIL(M52)
{
	int headLen = WriteHeader(content, fp);
	WriteKV(fp, "计划名称", TcpGetString(content, headLen, 64));
	WriteKV(fp, "计划ID", TcpGetString(content, headLen, 32));
	WriteKV(fp, "最后更新时间", TcpGetString(content, headLen, 14));
	int FileNumber = TcpGetInt(content, headLen, 4);
	WriteKV(fp, "所含文件数量", to_string(FileNumber));
	for (int i = 0; i < FileNumber; i++)
	{
		WriteFileInfo(fp, content,headLen);
	}
}

	LOG_REG_DETAIL(M45)
	{
		int headLen = WriteHeader(content, fp);
		WriteKV(fp, "计划ID", TcpGetString(content, headLen, 32));
		WriteKV(fp, "命令类型", TcpGetString(content, headLen, 8));
	}

	LOG_REG_DETAIL(M46)
	{
		int headLen = WriteHeader(content, fp);
		WriteKV(fp, "计划ID", TcpGetString(content, headLen, 32));
		WriteKV(fp, "命令类型", TcpGetString(content, headLen, 8));
	}

	LOG_REG_DETAIL(M61)
	{
		int headLen = WriteHeader(content, fp);
		WriteKV(fp, "设备编号", TcpGetString(content, headLen, 14));
		WriteKV(fp, "设备类型", TcpGetString(content, headLen, 2));
		WriteKV(fp, "设备描述", TcpGetString(content, headLen, 32));
		WriteKV(fp, "设备地址", TcpGetString(content, headLen, 26));
		WriteKV(fp, "MAC地址", TcpGetString(content, headLen, 18));
		WriteKV(fp, "所属区域", TcpGetString(content, headLen, 2));
		WriteKV(fp, "站台号", TcpGetString(content, headLen, 4));
		WriteKV(fp, "更新日期", TcpGetString(content, headLen, 14));
	}

LOG_REG_DETAIL(M65)
{
	int headLen = WriteHeader(content, fp);
	int EquNumber = TcpGetInt(content, headLen, 3);
	WriteKV(fp, "设备数量", to_string(EquNumber));
	for (int i = 0; i < EquNumber; i++)
	{
		WriteKV(fp, "设备编号", TcpGetString(content, headLen, 14));
		WriteKV(fp, "设备类型", TcpGetString(content, headLen, 2));
		WriteKV(fp, "设备描述", TcpGetString(content, headLen, 33));
		WriteKV(fp, "设备地址", TcpGetString(content, headLen, 26));
		WriteKV(fp, "MAC地址", TcpGetString(content, headLen, 18));
		WriteKV(fp, "所属区域", TcpGetString(content, headLen, 2));
		WriteKV(fp, "站台号", TcpGetString(content, headLen, 4));
		WriteKV(fp, "更新日期", TcpGetString(content, headLen, 14));
	}
	int UserNumber = TcpGetInt(content, headLen, 3);
	WriteKV(fp, "用户数量", to_string(UserNumber));
	for (int i = 0; i < UserNumber; i++)
	{
		WriteKV(fp, "用户编号", TcpGetString(content, headLen, 12));
		WriteKV(fp, "用户登陆账号", TcpGetString(content, headLen, 12));
		WriteKV(fp, "用户名称", TcpGetString(content, headLen, 32));
		WriteKV(fp, "密码", TcpGetString(content, headLen, 32));
		WriteKV(fp, "所属部门", TcpGetString(content, headLen, 32));
		WriteKV(fp, "电话", TcpGetString(content, headLen, 12));
		WriteKV(fp, "邮箱", TcpGetString(content, headLen, 32));
		WriteKV(fp, "账号状态", TcpGetString(content, headLen, 1));
		WriteKV(fp, "权限", TcpGetString(content, headLen, 32));
		WriteKV(fp, "更新日期", TcpGetString(content, headLen, 14));
	}
	WriteKV(fp, "车站运营时间", TcpGetString(content, headLen, 17));
}

	LOG_REG_DETAIL(M62)
	{
		int headLen = WriteHeader(content, fp);
		WriteKV(fp, "ATS", TcpGetString(content, headLen, 28));
		WriteKV(fp, "设备运营时间", TcpGetString(content, headLen, 17));
		WriteKV(fp, "时钟服务器", TcpGetString(content, headLen, 28));
		WriteKV(fp, "软件更新地址", TcpGetString(content, headLen, 128));
		int BusNumber = TcpGetInt(content, headLen, 3);
			WriteKV(fp, "首末班数量", to_string(BusNumber));
			for (int i = 0; i < BusNumber; i++)
			{
				WriteKV(fp, "方向中文", TcpGetString(content, headLen, 12));
				WriteKV(fp, "方向英文", TcpGetString(content, headLen, 12));
				WriteKV(fp, "终点中文", TcpGetString(content, headLen, 32));
				WriteKV(fp, "终点英文", TcpGetString(content, headLen, 32));
				WriteKV(fp, "首班中文提示", TcpGetString(content, headLen, 18));
				WriteKV(fp, "首班英文提示", TcpGetString(content, headLen, 18));
				WriteKV(fp, "末班中文提示", TcpGetString(content, headLen, 18));
				WriteKV(fp, "末班英文提示", TcpGetString(content, headLen, 18));
				WriteKV(fp, "首班车时间", TcpGetString(content, headLen, 8));
				WriteKV(fp, "末班车时间", TcpGetString(content, headLen, 8));
				WriteKV(fp, "更新日期", TcpGetString(content, headLen, 14));
			}
	}


	LOG_REG_DETAIL(M41)
	{
		int headLen = WriteHeader(content, fp);
		WriteKV(fp, "控制命令", TcpGetString(content, headLen, 2));
		WriteKV(fp, "附加值", TcpGetString(content, headLen, 32));
	}

	LOG_REG_DETAIL(M71)
	{
		int headLen = WriteHeader(content, fp);
		WriteKV(fp, "支持类型", TcpGetString(content, headLen, 1));
		WriteKV(fp, "控制通道URL", TcpGetString(content, headLen, 32));
	}

	LOG_REG_DETAIL(M72)
	{
		int headLen = WriteHeader(content, fp);
		WriteKV(fp, "流/图像序列接收协议及地址", TcpGetString(content, headLen, 32));
	}

	LOG_REG_DETAIL(M73)
	{
		int headLen = WriteHeader(content, fp);
		WriteKV(fp, "播放控制指令", TcpGetString(content, headLen, 4));
	}

	LOG_REG_DETAIL(M31)
	{
		int headLen = WriteHeader(content, fp);
	}

	LOG_REG_DETAIL(M32)
	{
		int headLen = WriteHeader(content, fp);
		WriteKV(fp, "设备编号", TcpGetString(content, headLen, 14));
		WriteKV(fp, "状态发送间隔", TcpGetString(content, headLen, 4));
	}

	LOG_REG_DETAIL(M37)
	{
		int headLen = WriteHeader(content, fp);
		WriteKV(fp, "设备编号", TcpGetString(content, headLen, 14));
		WriteKV(fp, "日志上传地址", TcpGetString(content, headLen, 128));
		WriteKV(fp, "日志更新间隔", TcpGetString(content, headLen, 4));
	}

}

