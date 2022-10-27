#include <bj_pis/ats/bj_ats_info.h>
#include "bj_pis/convent/tcp_msg_help.h"
#include "bj_pis/utils/JsonObject.h"
#include "transmanage/TransManager.h"
#include "LCDController.h"

bj_ats_info& bj_ats_info::Instance()
{
	static bj_ats_info s_info_;
	return s_info_;
}

string bj_ats_info::GetValue(const string& var, int index)
{
	rwlock_trains.ReadLock();
	map<string,string>::iterator iter_station =  station_info.find(var);
	if(iter_station!=station_info.end())
	{
		string temp=iter_station->second;
		rwlock_trains.ReadUnlock();
		return temp;
	}

	if(trains_info.size()>index&&index>=0)
	{
		map<string,string>& train = trains_info.at(index);
		map<string,string>::iterator iter_train =  train.find(var);
		if(iter_train!=train.end())
		{
			string temp=iter_train->second;
			rwlock_trains.ReadUnlock();
			return temp;
		}
	}
	rwlock_trains.ReadUnlock();
	//printf("bj_ats_info::GetValue, can't find '%s' with index %d.\n",var.c_str(),index);
	return string();
}

void bj_ats_info::DealATS(const char* s, int l)
{
	string jsonStr=code_convent(s,l,"GB2312","UTF-8");
	printf("\ntask_get_ats::DealATS:%s\n", jsonStr.c_str());
	JsonObject jo;
	jo.Parse(jsonStr);
	rwlock_trains.WriteLock();
	JsonObject& joAts=jo["ATS"];
	for(JsonObject& jsonPlatform : joAts)
	{
		if(jsonPlatform["platformId"].ToString()=="1")
		{
			trains_info.clear();
			JsonObject& jsonAtsInfo=jsonPlatform["atsInfo"];
			for(JsonObject& jsonTrain : jsonAtsInfo)
			{
				map<string,string> temp;
				for(JsonObject& jsonItem : jsonTrain)
				{
					temp.insert(make_pair(jsonItem.GetName(),jsonItem.ToString()));
				}
				if((bool)jsonTrain["showType"])
				{
					temp["time"].clear();
					temp["timeCh"].clear();
					temp["timeEn"].clear();
				}
				else
				{
					temp["statusCh"].clear();
					temp["statusEn"].clear();
				}
				trains_info.push_back(std::move(temp));
			}
			break;
		}
	}
	rwlock_trains.WriteUnlock();
	//LCDController::GetInstance()->removeMessage(ArrivalInfoUpdated);
	LCDController::GetInstance()->GetTransManager()->sendMessage(new Message(ArrivalInfoUpdated));
}
