/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : CmdDownLoader.cpp
 * @author : Benson
 * @date : Sep 21, 2017
 * @brief :
 */

#include <command/Command.h>
#include <Log.h>
#include <Message.h>
#include <sys/types.h>
#include <SystemClock.h>
#include <transmanage/CmdHandler.h>
#include <transmanage/TransManager.h>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <sstream>
#include "serial/SerialManager.h"

const char * CmdHandler::TAG = "CmdDownLoader";

CmdHandler::CmdHandler(TransManager* manager) :
        ITransHandler(manager)
{
	mRunningCommadMap.clear();

	mPrevCmdList.mCmds.clear();
}

CmdHandler::~CmdHandler()
{
	for(std::map<int,Command* >::iterator iter = mRunningCommadMap.begin();
			iter != mRunningCommadMap.end();++iter)
	{
		DELETE_ALLOCEDRESOURCE(iter->second);
	}
	mRunningCommadMap.clear();
}

void CmdHandler::Execute(Message* msg,int flag)
{
    if ( NULL == mTransManager)
        return;

    LogI("[CMD]--Handle get command update list req!\n ");
    mTransManager->sendRequest(this,DL_CmdsUpdateList, NULL);
}

std::string CmdHandler::ModifyPath(const TransFileType dltype,
        const std::string& oriPath, void* param)
{
    const ConfigParser* cfg = mTransManager->GetConfig();
    if (NULL == cfg)
        return oriPath;

    std::stringstream ssUrl;
    ssUrl << oriPath;

    switch (dltype)
    {
    case DL_CmdsUpdateList:
    {
        ssUrl << cfg->mDeviceId;
    	//ssUrl << "code_2";

        return ssUrl.str();
    }
    case DL_CmdRequest:
    {
        Json::CmdBasic* bsc = static_cast<Json::CmdBasic*>(param);
        if (bsc != NULL)
        {
            ssUrl << bsc->mId;
            ssUrl << "?device="<<mTransManager->GetConfig()->mDeviceId;
            return ssUrl.str();
        }
        break;
    }
    case UP_CmdExeReply:
    {
        int cmdid = *((int*)param);
       // LogD("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@2  %d\n",cmdid);
        if (cmdid > 0)
        {
            ssUrl << cmdid << "/" << "reply";
            return ssUrl.str();
        }
        break;
    }
    case UP_SnapShot:
    {
        // Set the upload url of target server!
        std::string* fileName = static_cast<std::string*>(param);
        if(fileName != NULL)
        {
            ssUrl << fileName << "&usage="<<F_SNAPSHOT;
        }

        return ssUrl.str();
    }
    default:
        // Do not need any modification on the path.
        break;
    }
    return oriPath;
}

bool CmdHandler::CheckCanceled(const Json::CmdList& newList,
        std::vector<Json::CmdBasic>& canceledCmds,
        std::vector<Json::CmdBasic>& normalCmds)
{
    canceledCmds.clear();
    normalCmds.clear();

    for (uint i = 0; i < newList.mCmds.size(); i++)
    {
        if (newList.mCmds[i].mStatus > 0)
            canceledCmds.push_back(newList.mCmds[i]);
        else
            normalCmds.push_back(newList.mCmds[i]);
    }

    return (canceledCmds.size() > 0 ? true : false);
}

CommandMsgType CmdHandler::GetCmdType(const std::string& cmdStr)
{
    if ("startup" == cmdStr)
        return MSG_CMD_StartUp;
    else if ("shutdown" == cmdStr)
        return MSG_CMD_ShutDown;
    else if ("reboot" == cmdStr)
        return MSG_CMD_ReBoot;
    else if ("get_log" == cmdStr)
        return MSG_CMD_GetLog;
    else if ("get_snapshot" == cmdStr)
        return MSG_CMD_GetSnapShot;
    else if ("housekeeping" == cmdStr)
        return MSG_CMD_HouseKeeping;
    else if("setbrightness" == cmdStr)
    	return MSG_CMD_SetBrightness;
    else if("setvolumn" == cmdStr)
    	return MSG_CMD_SetVolumn;
    else if("mute" == cmdStr || "recover" == cmdStr)
    	return MSG_CMD_SetVolumeMute;
    else
        return MSG_CMD_Unknown;
}

bool CmdHandler::handleMessage(Message *msg)
{
    if (NULL == msg)
        return false;

    int what = msg->mWhat;

    switch (what)
    {
    case DL_CmdsUpdateList:
    { // handle download command list reply.
        LogI("[CMD]--Handle get command update list reply!\n ");
        DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;
        HandleCmdListReply(work, (DataTrans::DownloadStatus) msg->mArg1);

        DELETE_ALLOCEDRESOURCE(work);

        break;
    }
    case DL_CmdRequest:
    { // handle download command detail reply.
        LogI("[CMD]--Handle get command detail reply!\n ");
        DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;
        HandleCmdDetailReply(work, (DataTrans::DownloadStatus) msg->mArg1);

        DELETE_ALLOCEDRESOURCE(work);

        break;
    }
    case MSG_CancelCmd:
    { // handle cancel command.
        LogI("[CMD]--Handle cancel command!\n ");
        HandleCancelCmd();
        break;
    }
    case MSG_ExecuteCmd:
    { // handle execute command.
        Command* cmd = static_cast<Command*>(msg->mData);
        if (NULL == cmd)
        {
            LogW("Empty Command!\n");
            break;
        }

        LogI("[CMD]--Handle execute command -%s[%d]-%s!\n ",
                cmd->GetCmdName().c_str(), cmd->GetCmdId(),
                SystemClock::Today(SystemClockTMFormat).c_str());
        if(cmd != NULL)
        	cmd->Execute();

        break;
    }
    case MSG_CmdCompleted:
    {
		std::map<int,Command* >::iterator iter = mRunningCommadMap.find(msg->mArg1);
		if(iter != mRunningCommadMap.end())
		{
			DELETE_ALLOCEDRESOURCE(iter->second);
			mRunningCommadMap.erase(iter);
		}

        break;
    }
    case MSG_CmdResult:
    {
        LogD("############### GenerateReply - %s\n",msg->mStr.c_str());
        // TODO: [CMD] send the reply !!!!
        mTransManager->sendReply(this, DataTrans::DataTransType_String_PUT,
                UP_CmdExeReply, msg->mStr, (void*)&(msg->mArg1));

    	break;
    }
    case UP_CmdExeReply:
    {
    	DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;
        DataTrans::UploadStatus rslt =
                static_cast<DataTrans::UploadStatus>(msg->mArg1);
        if (DataTrans::UploadSuccess == rslt)
		{
			LogI("[Report status]-- Report command status succeed!\n ");

		} else
		{
			// TODO: Report failed, what should i do?
			LogI("[Report status]-- Report command status failed!\n ");
		}

    	DELETE_ALLOCEDRESOURCE(work);
    	break;
    }
    case MSG_CMDToLED:
    {
    	std::map<int,Command* >::iterator iter = mRunningCommadMap.find(msg->mArg1);
    	if(iter != mRunningCommadMap.end())
    	{
    		mTransManager->sendMessage(new Message(LEDCmdUpdated,(void*)(iter->second)));
    	}
    	break;
    }
    case MSG_CMDLEDReply:
    {

    	break;
    }
    default:
        break;
    }

    return true;
}

void CmdHandler::HandleCmdListReply(DataTrans::TransWork *work,
        DataTrans::DownloadStatus status)
{
    if (DataTrans::DownloadSuccess != status || NULL == work)
    {
        // TODO: download list failed!
        return;
    }
    LogD("command list: %s\n",work->mJsonData.c_str());
    // Download succeed, Then parse the result.
    Json::CmdList newCmdList;
    bool ret = Json::CmdList::Parse(work->mJsonData.c_str(), &newCmdList);
    if (!ret)
    {
        LogE("CmdList parse failed!\n");
        return;
    }

    // Check if any command canceled.
    std::vector<Json::CmdBasic> newCancelList;
    std::vector<Json::CmdBasic> tobeExcutedList;

    bool hasCanceled = CheckCanceled(newCmdList, newCancelList,
            tobeExcutedList);

    mCurrCancelList.insert(mCurrCancelList.end(), newCancelList.begin(),
            newCancelList.end());

    if (hasCanceled)
    {
        sendMessage(new Message(MSG_CancelCmd));
    }

    // Get the detail of command need to be executed.
    for (uint i = 0; i < tobeExcutedList.size(); i++)
    {
        Json::CmdBasic bsc = tobeExcutedList[i];

        bool bfind = false;
        for (uint i = 0; i < mPrevCmdList.mCmds.size(); i++)
	    {
        	if (mPrevCmdList.mCmds[i].mId  == bsc.mId)
        	{
        		bfind = true;
        		break;
        	}
	    }
        if(bfind)
        {
        	LogD("Command had been download.\n",bsc.mId);
        	continue;
        }

        LogI("[CMD]--Handle get command detail req[id=%d]!\n ", bsc.mId);
        mTransManager->sendRequest(this,DL_CmdRequest, &bsc);
    }

    mPrevCmdList = newCmdList;
}

void CmdHandler::HandleCmdDetailReply(DataTrans::TransWork *work,
        DataTrans::DownloadStatus status)
{
    if (DataTrans::DownloadSuccess != status || NULL == work)
    {
        // TODO: download list failed!
        return;
    }

    LogD("Command detail: %s\n",work->mJsonData.c_str());
    CommandMsgType type = MSG_CMD_Unknown;
    uint64_t startTime = 0;
    int64_t timeOffset = 0;
    std::vector<std::string>::iterator itor;
    Command* cmd = NULL;
    bool bSameCmdFind = false;
    // Download succeed, Then parse the result.
    Json::CmdDetail* cmdDtl = new Json::CmdDetail();

    bool ret = Json::CmdDetail::Parse(work->mJsonData.c_str(), cmdDtl);
    if (!ret || cmdDtl == NULL)
    {
        LogE("CmdDetail parse failed!\n");
        goto CLEAN_UP;
    }

//    if(mRunningCommadMap.find(cmdDtl->mBasic.mId) != mRunningCommadMap.end())
//    {
//    	LogD("Command had been downloaded.\n");
//    	goto CLEAN_UP;
//    }

    if (cmdDtl->mBasic.mStartTime <= SystemClock::Today(SystemClockTMFormat))
    {
    	LogD("Command execute right now.\n");
        timeOffset = 0;  // execute right now.
    } else
    {
        // Calculate start time.
        SystemClock::StrToUptimeMillis(cmdDtl->mBasic.mStartTime, startTime,
        SystemClockTMFormat);
        timeOffset = startTime - SystemClock::UptimeMillis();
        LogD("Command execute delay:%lld.\n",timeOffset);
    }

	// Generate commands according to command type.

	type = GetCmdType(cmdDtl->mBasic.mCmd);
	cmd = CmdFactory::GenerateCmd(type, mTransManager->GetConfig(),this);
	if (cmd != NULL)
	{
		if(mTransManager->getLCDController() != NULL)
		{
			cmd->setLCDSerialObj(mTransManager->getLCDController()->getLCDPlayer());
		}
		else
		{
			LogD("mTransManager->getLCDController() == NULL.\n");
		}

		cmd->SetParam(&(cmdDtl->mBasic));
		itor = cmdDtl->mSpecObj.mTargets.begin();

		bool bret = true;
		if(cmdDtl->mSpecObj.mTargets.size() > 0)
		{
			bret = cmd->setSuitableDev(cmdDtl->mSpecObj.mTargets,mTransManager->GetConfig()->mLCDLEDFlag);
		}

		if(cmdDtl->mSpecObj.mTargets.size() == 0 || !bret)
		{
			LogD("No Trarget to run the command.\n");
			DELETE_ALLOCEDRESOURCE(cmd);
			goto CLEAN_UP;
		}

		mRunningCommadMap[cmdDtl->mBasic.mId] = cmd;

		sendMessage(new Message(MSG_ExecuteCmd, cmd), timeOffset);
	}
	else
	{
		LogD("cmd == NULL, type - %d \n",type);
	}

    CLEAN_UP:
    {
        DELETE_ALLOCEDRESOURCE(cmdDtl);
        return;
    }
}

void CmdHandler::HandleCancelCmd()
{
    Json::CmdBasic bsc;
    CommandMsgType type = MSG_CMD_Unknown;
    while (mCurrCancelList.size() > 0)
    {
        bsc = mCurrCancelList.front();
        type = GetCmdType(bsc.mCmd);
        if (MSG_CMD_Unknown != type)
        {
            // cancel the command execute.
            removeMessage(type);
        }

        mCurrCancelList.pop_front();
    }
}
