#include "hd_opm.h"
#include <set>
#include "bj_msg.h"
#include "tcp_msg_help.h"
#include "transmanage/TransHandlerFactory.h"
#include "LCDController.h"
#include "transmanage/OPSHandler.h"
#include "transmanage/TransManager.h"
#include "bj_pis/utils/string_ext.h"
#include <boost/filesystem.hpp>

STATIC_REGISTRATION(HdOpm)
{
	bj_msg::GetHandlers()["M11"] = &HdOpm::PublishOpm;
	bj_msg::GetHandlers()["M21"] = &HdOpm::CancelOpm;
}

static int bj_opm_global_index = 1;
FILE* HdOpm::fp_ = nullptr;
map<int, OpmMsg> HdOpm::msgs_;

void HdOpm::SendProcess(OpmMsg& opm_msg)
{
	int id = bj_opm_global_index++;
	msgs_[id] = opm_msg;
	WriteToFile();
	TransHandlerFactory* factory = TransHandlerFactory::Instance(LCDController::GetInstance()->GetTransManager());
	ITransHandler* loader = factory->GetLoader(NTF_RTOPSMsgUpdated);
	dynamic_cast<OPSHandler*>(loader)->HandleOPSDetailReply(opm_msg, id);
}

void HdOpm::CancelProcess(const string& tag)
{
	auto iter = find_if(msgs_.begin(), msgs_.end(), [&tag](const pair<int, OpmMsg> p)
		{
			return p.second.tag == tag;
		});
	if (iter != msgs_.end())
	{
		int* delete_id = new int(iter->first);
		TransHandlerFactory* factory = TransHandlerFactory::Instance(LCDController::GetInstance()->GetTransManager());
		ITransHandler* loader = factory->GetLoader(NTF_RTOPSMsgUpdated);
		LCDController::GetInstance()->GetTransManager()->sendMessage(new Message(OPSMsgUpdated, (void*)delete_id, (int)OPSUpdateStatus::OPS_deletestatus));
		dynamic_cast<OPSHandler*>(loader)->sendMessage(new Message(UP_OPSReply, iter->first, OPS_PlayFinished)); // report delate status.
		msgs_.erase(iter);
		WriteToFile();
	}
}

void HdOpm::CancelProcess(const char* priorities)
{
	set<int> opm_ids;
	for (size_t i = 0; i < 10; i++)
	{
		if (priorities[i] == '1')
		{
			for (auto iter = msgs_.begin(); iter != msgs_.end();)
			{
				if (iter->second.priority == i + 1)
				{
					opm_ids.insert(iter->first);
					iter = msgs_.erase(iter);
					WriteToFile();
				}
				else
				{
					iter++;
				}
			}
		}
	}
	TransHandlerFactory* factory = TransHandlerFactory::Instance(LCDController::GetInstance()->GetTransManager());
	ITransHandler* loader = factory->GetLoader(NTF_RTOPSMsgUpdated);
	for (auto it = opm_ids.begin(); it != opm_ids.end(); it++)
	{
		//根据优先级撤销
		int* delete_id = new int(*it);
		LCDController::GetInstance()->GetTransManager()->sendMessage(new Message(OPSMsgUpdated, (void*)delete_id, (int)OPSUpdateStatus::OPS_deletestatus));
		dynamic_cast<OPSHandler*>(loader)->sendMessage(new Message(UP_OPSReply, *it, OPS_PlayFinished)); // report delate status.
	}
}

void HdOpm::PublishOpm(bj_msg* msg, ServerTcpConnection* conn)
{
	int pos = 0;
	OpmMsg opm_msg;
	opm_msg.tag = move(TcpGetString(msg->Data, pos, 14));
	opm_msg.start_time = TcpGetDateTime(msg->Data, pos, 14);
	opm_msg.end_time = TcpGetDateTime(msg->Data, pos, 14);
	opm_msg.priority = TcpGetInt(msg->Data, pos, 2);
	if (opm_msg.priority != 6)
	{
		opm_msg.info_len = TcpGetInt(msg->Data, pos, 4);
		opm_msg.info_text = move(TcpGetStringUTF16(msg->Data, pos, opm_msg.info_len));
		opm_msg.info_text = move(string_replace(opm_msg.info_text,"\r\n",""));
		SendProcess(opm_msg);
	}
	else
	{
		opm_msg.file_info.Parse(msg->Data + pos);
		opm_msg.info_text = opm_msg.file_info.FileName;
		OpmMsg *pOpm=new OpmMsg(opm_msg);
		pOpm->file_info.Download("/home/workspace/media/", [pOpm](bj_file_info* file)
			{
				if(file->HasDownloaded)
				{
					SendProcess(*pOpm);
				}
				delete pOpm;
			});
	}

	bj_msg reply;
	reply.Copy(*msg);
	reply.SystemCode = "10";//SPIS
	reply.Command = "A11";
	reply.SetData("", 145);
	int index=0;
	TcpSetString(reply.Data,index,opm_msg.tag,14);
	TcpSetString(reply.Data,index,"100",3);
	TcpSetString(reply.Data,index,"ok",128);

	char reply_data[256];//请保证自己buffer够用
	int len = reply.Write(reply_data);
	conn->Send(reply_data, len);
}

void HdOpm::CancelOpm(bj_msg* msg, ServerTcpConnection* conn)
{
	int pos = 0;
	const string& tag = TcpGetString(msg->Data, pos, 14);
	char priorities[10 + 1] = { '\0'};
	memcpy(priorities, msg->Data + pos, 10);
	if (!tag.empty())
	{
		CancelProcess(tag);
	}
	else
	{
		CancelProcess(priorities);
	}

	bj_msg reply;
	reply.Copy(*msg);
	reply.SystemCode = "10";//SPIS
	reply.Command = "A21";
	reply.SetData("", 145);
	int index=0;
	TcpSetString(reply.Data,index,tag,14);
	TcpSetString(reply.Data,index,"100",3);
	TcpSetString(reply.Data,index,"ok",128);

	char reply_data[256];//请保证自己buffer够用
	int len = reply.Write(reply_data);
	conn->Send(reply_data, len);
}

void HdOpm::WriteToFile()
{
	fp_ = fopen("/home/workspace/opm.txt", "w");
	for (auto it = msgs_.begin(); it != msgs_.end(); it++)
	{
		fputs((it->second.Variable2String(it->first) + '@').c_str(), fp_);
	}
	fclose(fp_);
}

void HdOpm::ReadFromFile()
{
	fp_ = fopen("/home/workspace/opm.txt", "r");
	if (nullptr == fp_)
	{
		return;
	}
	auto file_size = boost::filesystem::file_size("/home/workspace/opm.txt");
	if (0 == file_size)
	{
		return;
	}
	char buf[file_size + 1];
	fread(buf, 1, file_size, fp_);
	string msgs(buf, file_size - 1);
	printf("msgs is %s\n", msgs.c_str());
	boost::regex ws_re("@");
	vector<string> vec(boost::sregex_token_iterator(msgs.begin(), msgs.end(), ws_re, -1), boost::sregex_token_iterator());
	printf("vec.size() is %d\n", vec.size());
	for (size_t i = 0; i < vec.size(); i++)
	{
		OpmMsg msg;
		int id = msg.String2Variable(vec[i]);
		msgs_[id] = msg;
	}
	fclose(fp_);
}

void HdOpm::LoadLocal()
{
	ReadFromFile();
	if (msgs_.size() > 0)
	{
		bj_opm_global_index = (--msgs_.end())->first + 1;
		printf("bj_opm_global_index is %d\n", bj_opm_global_index);
		for (auto it = msgs_.begin(); it != msgs_.end(); it++)
		{
			TransHandlerFactory* factory = TransHandlerFactory::Instance(LCDController::GetInstance()->GetTransManager());
			ITransHandler* loader = factory->GetLoader(NTF_RTOPSMsgUpdated);
			dynamic_cast<OPSHandler*>(loader)->HandleOPSDetailReply(it->second, it->first);
		}
	}
}

