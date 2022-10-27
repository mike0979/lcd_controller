/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file : OPSHandler.h
* @author : Benson
* @date : Sep 21, 2017
* @brief :
*/

#ifndef TRANSMANAGE_OPSHANDLER_H_
#define TRANSMANAGE_OPSHANDLER_H_

//#include <transmanage/ITransHandler.h>
//
//class OPSHandler: public ITransHandler
//{
//public:
//    OPSHandler(TransManager* manager);
//    virtual ~OPSHandler();
//
//    virtual void Execute(Message * msg);
//
//    virtual std::string ModifyPath(const TransFileType dltype,
//                const std::string& oriPath, void* param);
//
//protected:
//    virtual bool handleMessage(Message *msg);
//};

#include <transmanage/ITransHandler.h>
#include "DataTrans.h"
#include <json/ScheduleObjs.h>
#include <map>

class OPSHandler: public ITransHandler
{
public:
    OPSHandler(TransManager* manager);
    virtual ~OPSHandler();

    virtual void Execute(Message * msg,int flag=0);

    virtual std::string ModifyPath(const TransFileType dltype,
                const std::string& oriPath, void* param);
    void HandleOPSDetailReply(const OpmMsg& opm_msg, int opm_id);
protected:
    virtual bool handleMessage(Message *msg);

private:
    int HandleOPSListReply(DataTrans::TransWork* work, DataTrans::DownloadStatus status);
    int HandleOPSDetailReply(DataTrans::TransWork* work, DataTrans::DownloadStatus status);
    int HandleOPSMsgUpdated();

    /**
     * Handle upload ops-message executed result.
     * @param opsId
     * @param opsRlst
     * @return
     */
    int HandleOPSExecuteReply(int opsId,int opsRlst);

    bool CheckOPSOutDated(Json::OPSMsgBasic& opsbasic);
    bool getOPSMsgUpdatedID(Json::OPSMsgList& newlist,std::list<int>& addlist,std::list<int>& updatedlist,std::list<int>& deletedlist);
    bool getNextOPSMsgPlayTime(unsigned& next);
    bool getCurrOPSMsg(Json::OPSMsgDetail* &opsmsg);
private:
    Json::OPSMsgList mOPSMsgList;
    map<int, Json::OPSMsgDetail> mCurrPlayingOPSMsg;
    std::map<int, Json::OPSMsgDetail*> mOPSMsgFinishedList;

    std::list<int> mOPSMsgAddList;
    std::list<int> mOPSMsgUpdateList;
    std::list<int> mOPSMsgDeletedList;

    std::map<int, Json::OPSMsgDetail*> mOPSMsgDetailList;

    static const char *TAG;
};


#endif /* TRANSMANAGE_OPSHANDLER_H_ */
