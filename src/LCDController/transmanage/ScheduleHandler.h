/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : ScheduleHandler.h
 * @author : Benson
 * @date : Sep 21, 2017
 * @brief :
 */

#ifndef TRANSMANAGE_SCHEDULEHANDLER_H_
#define TRANSMANAGE_SCHEDULEHANDLER_H_

#include <CommonDef.h>
#include <DataTrans.h>
#include <json/ScheduleObjs.h>
#include <transmanage/ITransHandler.h>
#include <map>
#include <string>

class hd_layout;
class hd_schedule;

namespace Json
{
//class LayoutHandler;
class LayoutGroupFileHandler;
class MediaFileHandler;
class ScheduleFileHandler;
class UpdateListFileHandler;
} /* namespace Json */

class Message;
class ScheduleHandler: public ITransHandler
{
public:
    enum LoadMode
    {
        LD_FromServer = 0,
        LD_FromLocal = 1,
    };
    enum RestResourceDownloadStatus
    {
        RestReadyDownload = 0, RestDownloading, RestDownloaded,
    };

    ScheduleHandler(TransManager* manager);
    virtual ~ScheduleHandler();

    /**
     * Send "get schedule update list" request.
     * @param msg
     * @param flag: 0 --- get schedule from server
     *              1 --- load schedule from local.
     */
    virtual void Execute(Message * msg,int flag=0);

    /**
     * Modify the URL path.
     * @param dltype
     * @param oriPath
     * @param param
     * @return
     */
    virtual std::string ModifyPath(const TransFileType dltype,
            const std::string& oriPath, void* param);

protected:
    virtual bool handleMessage(Message *msg);

private:
    /**
     * Handle "get schedule update list" reply and send "get schedule detail" request.
     * @param work
     * @param status
     */
    void HandleSchUpdateListReply(DataTrans::TransWork *work,
            DataTrans::DownloadStatus status);

    /**
     * Handle "get schedule detail reply" and send "get layout group detail" request and send "get media detail" request.
     * @param work
     * @param status
     */
    void HandleSchDetailReply(DataTrans::TransWork *work,
            DataTrans::DownloadStatus status);

    /**
     * Handle "get layout group detail" reply and send "get media md5 info" request(partition back-images).
     * @param work
     * @param status
     */
    void HandleLayoutGroupDetailReply(DataTrans::TransWork *work,
            DataTrans::DownloadStatus status);

    /**
     * Handle "get media detail" reply and send "get media MD5 info" request(media back-image or media file).
     * @param work
     * @param status
     */
    void HandleMediaDetailReply(DataTrans::TransWork *work,
            DataTrans::DownloadStatus status);

    /**
     * Handle "get media md5 info" reply and send "download media content file" request(media back-image or media file).
     * @param work
     * @param status
     */
    void HandleMediaMD5Reply(DataTrans::TransWork *work,
            DataTrans::DownloadStatus status);

    /**
     * Handle "download media content file" result.
     * @param work
     * @param status
     */
    void HandleMediaContentReply(DataTrans::TransWork *work,
            DataTrans::DownloadStatus status);

    /**
     * Sync the schedule detail,layout detail and media detail from 'download' map
     * to related 'play' map.
     */
    void HandleSyncDld2Play();

    /**
     * Update current schedule.
     */
    void HandleUpdateSchedule();

    /**
     * Update current layout group.
     */
    void HandleUpdateLayoutGroup();

    /**
     * Update current layout.
     */
    void HandleUpdateLayout();

    /**
     * Check weather schedule detail,layout detail and media detail been download.
     * If media content file not download yet, send request to download content file.
     * @return true : schedule detail,layout detail and media detail been download.
     *         false : schedule detail,layout detail and media detail been download not finished download yet!
     */
    bool CheckSchDownloadCompleted();

    /**
     * Judge which schedule should play current now.
     * @param next[out]: the next judgment time.(ms)
     * @return
     */
    Json::ScheduleDetail* GetCurrentSchedule(unsigned &next);

    /**
     * Judge which layout group should play current now.
     * @param next[out]: the next judgment time.(ms)
     * @return
     */
    Json::SchLayoutGroupBasic* GetCurrentLayoutGroup(unsigned &next);

    /**
     * Get layout detail according to SchLayoutGroupBasic.
     * @return
     */
    Json::LayoutGroupDetail* GetCurrentLayoutGroupDetail(
            Json::SchLayoutGroupBasic* bsc);

    /**
     * Judge which layout should play current now.
     * @param next[out]: the next judgment time.(ms)
     * @return
     */
    Json::LayoutDetail* GetCurrentLayout(unsigned& next);

    /**
     * Load local files.
     * 1. schedule_list.json
     * 2. schedules.json
     * 3. layout_groups.json
     * 4. resources.json
     * @return
     */
    bool LoadLocalSchedule();

    void CurrentLayoutIndexAdd();
    int GetCurrentLayoutIndex() const;

    /**
     * Find ScheduleDetail according to schedule strategy and given time.
     * @param timePoint[in]: the time to judge.
     * @return
     */
    Json::ScheduleDetail* FindSchedule(const std::string& timePoint);

    Json::ScheduleDetail* FindNextSchedule(const std::string& timePoint,
    		const Json::ScheduleDetail* currsch);

    /**
     * Find SchLayoutGroupBasic according to LayoutGroup strategy and given time.
     * @param timePoint[in]: the time to judge.
     * @return
     */
    Json::SchLayoutGroupBasic* FindLayoutGroup(const std::string& timePoint);

    Json::SchLayoutGroupBasic* FindNextLayoutGroup(const std::string& timePoint);

    bool FindCurrentLayoutRelatedMedia(Json::LayoutInfo4Qt* layoutAllInfo);

    void FindMediaInfos(const std::vector<int>& mediaIds,Json::LayoutInfo4Qt::MediaContents& medias);
private:
    Json::UpdateListFileHandler* mUpdateListHandler;
    Json::ScheduleFileHandler* mScheduleDetailHandler;
    Json::LayoutGroupFileHandler* mLayoutGroupDetailHandler;
    Json::MediaFileHandler* mMediaDetailHandler;

    Json::ScheduleDetail* mCurrentScheduleForPlay; // current playing schedule.
    Json::SchLayoutGroupBasic* mCurrentLayouGroupForPlay; // current playing layout group basic.
    Json::LayoutGroupDetail* mCurrentLayoutGroupDetail; // the detail information of current playing layout group.
    unsigned int mCurrentLayoutIndex; // current layout index in current layout group.
    Json::LayoutDetail* mCurrentLayoutForPlay; // current playing layout.

    Json::ScheduleList mScheduleListForDL; // store the schedule list for download.
    std::map<int, Json::ScheduleDetail*> mScheduleForDL; // store the schedule detail for download.
    std::map<int, Json::LayoutGroupDetail*> mLayoutGroupForDL; // store the layout group detail for download.

    std::map<int, Json::MediaBasic*> mContentListForDL; // store the media detail for download.
    std::map<std::string, enum RestResourceDownloadStatus> mResourceDownloadList;

    std::map<int, Json::ScheduleDetail*> mScheduleForPlay;
    std::map<int, Json::LayoutGroupDetail*> mLayoutGroupForPlay;
    std::map<int, Json::MediaBasic*> mContentListForPlay;

    std::list<int> mScheduleListKeepedID;
    std::list<int> mScheduleListUpdatedID;
    std::list<int> mScheduleListDeletedID;

    std::string mScheduleListString;
    static const char *TAG;

    friend class hd_layout;
    friend class hd_schedule;
};

#endif /* TRANSMANAGE_SCHEDULEHANDLER_H_ */
