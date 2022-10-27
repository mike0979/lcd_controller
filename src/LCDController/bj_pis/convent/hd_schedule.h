#ifndef HD_SCHEDULE_H_
#define HD_SCHEDULE_H_

#include "bj_pis/convent/bj_msg.h"
#include "bj_plan_info.h"
#include "bj_file_info.h"
#include <map>
#include "bj_playlist_info.h"
#include "transmanage/ScheduleHandler.h"

class hd_schedule
{
public:
    hd_schedule();
    ~hd_schedule();

    static void PlayListSend(bj_msg *msg, ServerTcpConnection *conn);
    static void PlayListControl(bj_msg *msg, ServerTcpConnection *conn);

    //taskID:bj_plan_info
    static map<string, shared_ptr<bj_plan_info>> Plans;

    static const char* TAG;
    static void SetPlaylistMedia(Json::ScheduleDetail* sch,ScheduleHandler* handler);
    static void LoadLocal();
private:
	static void DealPlayList(const string& planId,bool reloadSchedule=true);
	static void GetPlayListMedia(Json::PartitionMedias &pm, ScheduleHandler* handler);
};

#endif // HD_SCHEDULE_H





