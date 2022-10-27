#ifndef HD_LAYOUT_H_
#define HD_LAYOUT_H_

#include "bj_pis/convent/bj_msg.h"
#include "bj_pis/convent/bj_layout_info.h"
#include <vector>
#include <memory>
#include <map>
#include "json/ScheduleObjs.h"

using namespace std;

class hd_layout
{
public:
  hd_layout();
  ~hd_layout();
  static void LayoutListSend(bj_msg *msg, ServerTcpConnection *conn);
  static void LayoutControlSend(bj_msg *msg, ServerTcpConnection *conn);
  static void DealNewLayout(const string& layoutGroupId);
  static bool LoadScheduleDetail(const std::string& layoutGroupId,bool onlyLoad=false);
  static vector<Json::MediaBasic*> GetMedia(shared_ptr<bj_partition_media> param,const string& partRect);
  static void LoadLocal();
  //planID:bj_layout_info
  static map<string, shared_ptr<bj_layout_info>> Layouts;
  //planID:local scheduledetailId
  static map<string,int> map_Plan_Sch;

  static const char* TAG;
};

#endif // HD_LAYOUT_H_
