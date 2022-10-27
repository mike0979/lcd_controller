/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file : CmdHandler.h
* @author : Benson
* @date : Sep 21, 2017
* @brief :
*/


#ifndef TRANSMANAGE_CMDHANDLER_H_
#define TRANSMANAGE_CMDHANDLER_H_

#include <CommonDef.h>
#include <DataTrans.h>
#include <json/CommandObjs.h>
#include <transmanage/ITransHandler.h>
#include <string>
#include <list>
#include <map>

class Command;
class SerialManager;

class CmdHandler: public ITransHandler
{
public:
    CmdHandler(TransManager* manager);
    virtual ~CmdHandler();

    virtual void Execute(Message * msg,int flag=0);

    virtual std::string ModifyPath(const TransFileType dltype,
                const std::string& oriPath, void* param);

private:
    /**
     * Check the new List to get the canceled commands.
     * @param newList[in]: The new get command list.
     * @param CanceledCmds[in,out]: Bring out the canceled commands.
     * @param normalCmds[in,out]: Bring out the normal commands.
     * @return True - newList canceled some commands.
     *         False - newList didn't canceled any commands.
     */
    bool CheckCanceled(const Json::CmdList& newList,std::vector<Json::CmdBasic>& canceledCmds,
            std::vector<Json::CmdBasic>& normalCmds);

    CommandMsgType GetCmdType(const std::string& cmdStr);

private:
    void HandleCmdListReply(DataTrans::TransWork *work,
            DataTrans::DownloadStatus status);

    void HandleCmdDetailReply(DataTrans::TransWork *work,
            DataTrans::DownloadStatus status);

    void HandleCancelCmd();
protected:
    virtual bool handleMessage(Message *msg);
    static const char *TAG;

    std::list<Json::CmdBasic> mCurrCancelList;

    std::map<int,Command* > mRunningCommadMap;

    std::list<int> mDownloadedCommandIDList;

    Json::CmdList mPrevCmdList;
};



#endif /* TRANSMANAGE_CMDHANDLER_H_ */
