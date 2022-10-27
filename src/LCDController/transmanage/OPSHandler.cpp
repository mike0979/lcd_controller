/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : OPSHandler.cpp
 * @author : Benson
 * @date : Sep 21, 2017
 * @brief :
 */

#include <CommonDef.h>
#include <DataTypeConvert.h>
#include <Log.h>
#include <Message.h>
#include <SystemClock.h>
#include <transmanage/OPSHandler.h>
#include <transmanage/TransManager.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <list>
#include <string>
#include <vector>

OPSHandler::OPSHandler(TransManager* manager) :
        ITransHandler(manager)
{
    assert(NULL != mTransManager);

    mOPSMsgList.mOPSMsgs.clear();
    mOPSMsgDetailList.clear();
}

OPSHandler::~OPSHandler()
{
    std::map<int, Json::OPSMsgDetail*>::iterator itor =
            mOPSMsgDetailList.begin();
    for (; itor != mOPSMsgDetailList.end(); ++itor)
    {
        DELETE_ALLOCEDRESOURCE(itor->second);
    }
}

void OPSHandler::Execute(Message* msg, int flag)
{
    if ( NULL == mTransManager)
        return;

    mTransManager->sendRequest(this, DL_OPMsgList, NULL);
}

bool OPSHandler::handleMessage(Message* msg)
{
    if (NULL == msg)
        return false;

    int what = msg->mWhat;
    DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;

    switch (what)
    {
    case DL_OPMsgList:
    {
        // handle download OPMsg list reply.
        LogD("OPSHandler list - subpath: %s\n", work->mSubPath.c_str());
        HandleOPSListReply(work, (DataTrans::DownloadStatus) msg->mArg1);

        DELETE_ALLOCEDRESOURCE(work);
        break;
    }
    case DL_OPMsgRequest:
    {
        // handle download OPMsg detail reply.
        LogD("OPSHandler detail - subpath: %s\n", work->mSubPath.c_str());
        HandleOPSDetailReply(work, (DataTrans::DownloadStatus) msg->mArg1);

        DELETE_ALLOCEDRESOURCE(work);
        break;
    }
    case DL_OPSMsgUpdatedNotify:
    {
        HandleOPSMsgUpdated();
        break;
    }
    case DL_OPSBackImage:
    {
        LogD("OPSHandler backImage rlst\n");
        break;
    }
    case UP_OPSReply:
    {
        // handle upload OPS Msg execute result request.
        LogI("OPSHandler send OPS execute reply: ID=%d,Execute rslt= %d\n",
                msg->mArg1, msg->mArg2);

        std::map<int, Json::OPSMsgDetail*>::iterator iter =
                mOPSMsgDetailList.find(msg->mArg1);
        if (iter != mOPSMsgDetailList.end())
        {
            if (msg->mArg2 == OPS_Playing)
            {
                mCurrPlayingOPSMsg[iter->second->mDisplayRegion] = *(iter->second);
            }
            else if (msg->mArg2 == OPS_PlayFinished)
            {
                if (iter->second != NULL)
                    mOPSMsgFinishedList[iter->first] = iter->second;
				for (auto it = mCurrPlayingOPSMsg.begin(); it != mCurrPlayingOPSMsg.end(); it++)
				{
					it->second.mPriority = -1;
				}
            } 
            else if (msg->mArg2 == OPS_Reveived)
            { // lbs new add.
              // do nothing.
				for (auto it = mCurrPlayingOPSMsg.begin(); it != mCurrPlayingOPSMsg.end(); it++)
				{
					LogI(
						"\t\t @@@@@@@@@@ new ops status[%d],ops id=%d,mCurrPlayingOPSMsg id=%d!!!\n",
						msg->mArg2, msg->mArg1, it->second.mBasic.mId);
				}
            } 
            else
            {
				for (auto it = mCurrPlayingOPSMsg.begin(); it != mCurrPlayingOPSMsg.end(); it++)
				{
					LogI(
						"\t\t @@@@@@@@@@ mCurrPlayingOPSMsg status[%d],ops id=%d,mCurrPlayingOPSMsg id=%d!!!\n",
						msg->mArg2, msg->mArg1, it->second.mBasic.mId);
					it->second.mPriority = -1;
				}
            }
        } 
        else
        {
			for (auto it = mCurrPlayingOPSMsg.begin(); it != mCurrPlayingOPSMsg.end(); it++)
			{
				it->second.mPriority = -1;
			}
            LogE("Get current playing OPSMsg failed - id: %d!!!\n", msg->mArg1);
        }

        HandleOPSExecuteReply(msg->mArg1, msg->mArg2);
        break;
    }
    case UP_OPSReplyResult:
    {
        LogD("Upload OPSReply result = %d\n", msg->mArg1);
        break;
    }
    default:
        break;
    }

    DELETE_ALLOCEDRESOURCE(work);

    return true;
}

bool OPSHandler::getOPSMsgUpdatedID(Json::OPSMsgList& newlist,
        std::list<int>& addlist, std::list<int>& updatedlist,
        std::list<int>& deletedlist)
{
    //updated
    bool isfind = false;
    bool isupdate = false;
    for (std::vector<Json::OPSMsgBasic>::iterator i = newlist.mOPSMsgs.begin();
            i != newlist.mOPSMsgs.end(); ++i)
    {
        isfind = false;
        isupdate = false;
        for (std::vector<Json::OPSMsgBasic>::iterator j =
                mOPSMsgList.mOPSMsgs.begin(); j != mOPSMsgList.mOPSMsgs.end();
                ++j)
        {
            if (i->mId == j->mId && i->mUpdateTime != j->mUpdateTime)
            {
                isupdate = true;
                break;
            }

            if (i->mId == j->mId && i->mUpdateTime == j->mUpdateTime)
            {
                isfind = true;
                break;
            }
        }

        if (!isfind && !isupdate)
        {
            LogI("OPSMsg added\n");
            addlist.push_back(i->mId);
        }

        if (!isfind && isupdate)
        {
            LogI("OPSMsg updated\n");
            updatedlist.push_back(i->mId);
        }
    }

    //deleted
    for (std::vector<Json::OPSMsgBasic>::iterator i =
            mOPSMsgList.mOPSMsgs.begin(); i != mOPSMsgList.mOPSMsgs.end(); ++i)
    {
//        if (CheckOPSOutDated(*i))
//        {
//            deletedlist.push_back(i->mId);
//            continue;
//        }

        isfind = false;
        for (std::vector<Json::OPSMsgBasic>::iterator j =
                newlist.mOPSMsgs.begin(); j != newlist.mOPSMsgs.end(); ++j)
        {
            if (i->mId == j->mId)
            {
                isfind = true;
                break;
            }
        }

        if (!isfind)
        {
            LogI("OPSMsg deleted\n");
            deletedlist.push_back(i->mId);
        }
    }

    return true;
}

int OPSHandler::HandleOPSListReply(DataTrans::TransWork* work,
        DataTrans::DownloadStatus status)
{
    if (DataTrans::DownloadSuccess != status || NULL == work)
    {
    	LogE("download ops list failed\n");
        return 0;
    }

    std::string opslistdata = work->mJsonData;

    Json::OPSMsgList opslist;

    LogD("OPSList detail: %d - %s\n", opslistdata.size(), opslistdata.c_str());
    bool pres = Json::OPSMsgList::Parse(opslistdata.c_str(), &opslist);
    if (!pres)
    {
        LogE("parse opsmsg list failed.\n");
        return 1;
    }

    LogD("Got reply,OPS list size-%d\nopslist - %s\n", opslist.mOPSMsgs.size(),
            opslistdata.c_str());

    std::list<int> OPSMsgAddList;
    std::list<int> OPSMsgUpdateList;
    std::list<int> OPSMsgDeletedList;
    OPSMsgAddList.clear();
    OPSMsgUpdateList.clear();
    OPSMsgDeletedList.clear();
    getOPSMsgUpdatedID(opslist, OPSMsgAddList, OPSMsgUpdateList,
            OPSMsgDeletedList);

    if (OPSMsgAddList.size() > 0 || OPSMsgUpdateList.size() > 0
            || OPSMsgDeletedList.size() > 0)
    {
    	mOPSMsgAddList = OPSMsgAddList;
    	mOPSMsgUpdateList = OPSMsgUpdateList;
    	mOPSMsgDeletedList = OPSMsgDeletedList;
    }
    else
    {
    	LogD("The same OPSList had been downloaded.\n");
    	mOPSMsgList = opslist;
    	return 0;
    }

    if (mOPSMsgAddList.size() > 0)
    {
        LogD("opsmsgaddList size-%d\n", mOPSMsgAddList.size());
        for (std::list<int>::iterator i = mOPSMsgAddList.begin();
                i != mOPSMsgAddList.end(); ++i)
        {
            LogD("  added OPS id-%d\n", *i);
            int lid = *i;
            mOPSMsgDetailList[lid] = NULL;
            mTransManager->sendRequest(this, DL_OPMsgRequest, (void*) (&lid)); //(void*)(&(DataTypeConvert::dataToString(*i)))
        }
    }

    if (mOPSMsgUpdateList.size() > 0)
    {
        LogD("opsmsgupdateList size-%d\n", mOPSMsgUpdateList.size());
        for (std::list<int>::iterator i = mOPSMsgUpdateList.begin();
                i != mOPSMsgUpdateList.end(); ++i)
        {
            LogD("  updated OPS id-%d\n", *i);
            int lid = *i;

            std::map<int, Json::OPSMsgDetail*>::iterator iter =
                    mOPSMsgDetailList.find(lid);
            if (iter != mOPSMsgDetailList.end())
                DELETE_ALLOCEDRESOURCE(iter->second);

            mTransManager->sendRequest(this, DL_OPMsgRequest, (void*) (&lid)); //(void*)(&(DataTypeConvert::dataToString(*i)))
        }
    }

    if (mOPSMsgDeletedList.size() > 0)
    {
        LogD("opsmsgdeletedList size-%d\n", mOPSMsgDeletedList.size());
        for (std::list<int>::iterator i = mOPSMsgDeletedList.begin();
                i != mOPSMsgDeletedList.end(); ++i)
        {
            std::map<int, Json::OPSMsgDetail*>::iterator iter =
                    mOPSMsgDetailList.find(*i);
            if (iter != mOPSMsgDetailList.end())
            {
                DELETE_ALLOCEDRESOURCE(iter->second);
                mOPSMsgDetailList.erase(iter);
            }
        }
    }

    if (mOPSMsgAddList.size() == 0 && mOPSMsgUpdateList.size() == 0
            && mOPSMsgDeletedList.size() > 0)
    {
        sendMessage(new Message(DL_OPSMsgUpdatedNotify));
    }

    mOPSMsgList = opslist;

    return 0;
}

int OPSHandler::HandleOPSDetailReply(DataTrans::TransWork* work,
        DataTrans::DownloadStatus status)
{
    LogE("####### HandleOPSDetailReply-%s\n", work->mJsonData.c_str());
    if(DataTrans::DownloadStatus::DownloadSuccess != status ||
    		work->mJsonData.size() <=2)
    {
    	return 0;
    }

    bool bres = false;
    Json::OPSMsgDetail* opsmsgdetail = new Json::OPSMsgDetail();
    bres = Json::OPSMsgDetail::Parse(work->mJsonData.c_str(), opsmsgdetail);
    if (bres && opsmsgdetail != NULL)
    {
        // Download back image.
        if (opsmsgdetail->mText.mBackImage.size() > 0)
        {
            mTransManager->downloadFile(this, DL_OPSBackImage,
                    opsmsgdetail->mText.mBackImage, "");

            const ConfigParser* cfg = mTransManager->GetConfig();
            opsmsgdetail->mText.mBackImageDir = cfg->mDldRootDir
                    + cfg->mContentpath;
        }

//        std::string time = SystemClock::Today(SystemClockTMFormat);
//        if (time >= opsmsgdetail->mBasic.mStartTime
//                && time <= opsmsgdetail->mBasic.mEndTime)
//        {
        int uId = 0;
        std::string sId;
        std::size_t namepos = work->mSubPath.find_last_of('/');
        if (namepos != std::string::npos)
        {
            sId = work->mSubPath.substr(namepos + 1);
            uId = DataTypeConvert::stringToNum<int>(sId);
            mOPSMsgDetailList[uId] = opsmsgdetail;
            LogE("OPSMsg download completed-%d\n", uId);

            // send reply to server.
            sendMessage(new Message(UP_OPSReply, uId, OPS_Reveived));
        } else
        {
            LogE("HandleOPSDetailReply() - get opsmsg detail id failed(%s).\n",
                    sId.c_str());
            DELETE_ALLOCEDRESOURCE(opsmsgdetail);
        }
        //  }
    } else
    {
        DELETE_ALLOCEDRESOURCE(opsmsgdetail);
    }

    bool isopsdlfinished = true;
    for (std::map<int, Json::OPSMsgDetail*>::iterator iter =
            mOPSMsgDetailList.begin(); iter != mOPSMsgDetailList.end(); ++iter)
    {
        if (iter->second == NULL)
        {
            isopsdlfinished = false;
        }
    }

    if (isopsdlfinished)
    {
        sendMessage(new Message(DL_OPSMsgUpdatedNotify));
    }

    return 0;
}


void OPSHandler::HandleOPSDetailReply(const OpmMsg& opm_msg, int opm_id)
{
	Json::OPSMsgDetail* opsmsgdetail = new Json::OPSMsgDetail();
	Json::OPSMsgDetail::Parse(opm_msg, opsmsgdetail, opm_id);

	if (opsmsgdetail->mText.mBackImage.size() > 0)
    {
        const ConfigParser* cfg = mTransManager->GetConfig();
        opsmsgdetail->mText.mBackImageDir = cfg->mDldRootDir + cfg->mContentpath;
    }

	mOPSMsgDetailList[opm_id] = opsmsgdetail;
	mOPSMsgAddList.push_back(opm_id);
	LogE("OPSMsg download completed-%d\n", opm_id);

	sendMessage(new Message(DL_OPSMsgUpdatedNotify));
}

int OPSHandler::HandleOPSMsgUpdated()
{
    LogD("HandleOPSMsgUpdated mOPSMsgDetailList size = %d\n ",
            mOPSMsgDetailList.size());

    OPSUpdateStatus updatestatus = OPSUpdateStatus::OPS_otherstatus;
    Json::OPSMsgDetail* opsmsg = NULL;
	auto func = [&]()
	{
        auto it = mCurrPlayingOPSMsg.find(opsmsg->mDisplayRegion);
		if (it != mCurrPlayingOPSMsg.end())
		{
			if (opsmsg->mPriority >= it->second.mPriority)
			{
				mTransManager->sendMessage(
					new Message(OPSMsgUpdated, (void*)opsmsg,
						(int)updatestatus));
			}
			else
			{
				LogD("keep current OPS play\n");
			}
		}
		else
		{
			mTransManager->sendMessage(
				new Message(OPSMsgUpdated, (void*)opsmsg,
					(int)updatestatus));
		}
	};
    std::string ltime = SystemClock::Today(SystemClockTMFormat);
    if (mOPSMsgAddList.size() > 0)
    {
        updatestatus = OPSUpdateStatus::OPS_addstatus;
        for (std::list<int>::iterator i = mOPSMsgAddList.begin();
                i != mOPSMsgAddList.end(); ++i)
        {
            std::map<int, Json::OPSMsgDetail*>::iterator iter =
                    mOPSMsgDetailList.find(*i);
            if (iter != mOPSMsgDetailList.end()
                    //&& (iter->second->mBasic.mStartTime <= ltime
                    //        && iter->second->mBasic.mEndTime > ltime)
/*                    && (opsmsg == NULL
                            || opsmsg->mBasic.mUpdateTime
                                    < iter->second->mBasic.mUpdateTime)*/)
            {
                opsmsg = iter->second;
                LogD("add ops id = %d\n", opsmsg->mBasic.mId);
                func();
            }
        }
    } else if (mOPSMsgUpdateList.size() > 0)
    {
        updatestatus = OPSUpdateStatus::OPS_updatestatus;
        std::map<int, Json::OPSMsgDetail*>::iterator iter =
                mOPSMsgDetailList.find(*(mOPSMsgUpdateList.begin()));
        if (iter != mOPSMsgDetailList.end()
                && (iter->second->mBasic.mStartTime <= ltime
                        && iter->second->mBasic.mEndTime > ltime))
        {
            opsmsg = iter->second;
        }
    } else if (mOPSMsgDeletedList.size() > 0)
    {
        updatestatus = OPSUpdateStatus::OPS_deletestatus;
        opsmsg = NULL;
    } else if (mOPSMsgDetailList.size() == 0)
    {
        updatestatus = OPSUpdateStatus::OPS_noopsstatus;
        opsmsg = NULL;
    } else
    {
        updatestatus = OPSUpdateStatus::OPS_otherstatus;
        opsmsg = NULL;
        getCurrOPSMsg(opsmsg);
        if (opsmsg == NULL)
        {
            updatestatus = OPSUpdateStatus::OPS_noopsstatus;
        }
    }

    if (opsmsg != NULL)
    {
        if (mOPSMsgAddList.size() == 0)
        {
			//        LogD("\t\t ------ 6666 HandleOPSMsgUpdated id=%d,pri=%d  [Current Id=%d],pri%d\n ",
//                opsmsg->mBasic.mId,opsmsg->mPriority ,mCurrPlayingOPSMsg.mBasic.mId, mCurrPlayingOPSMsg.mPriority);
            func();
        }
    }
    else
    {
        if (updatestatus == OPSUpdateStatus::OPS_deletestatus)
        {
            for (std::list<int>::iterator i = mOPSMsgDeletedList.begin();
                    i != mOPSMsgDeletedList.end(); ++i)
            {
            	int* deletedid = new int(0);
                *deletedid = *i;

                mTransManager->sendMessage(
                                    new Message(OPSMsgUpdated, (void*) deletedid,
                                            (int) updatestatus));

                sendMessage(new Message(UP_OPSReply, *deletedid, OPS_PlayFinished)); // report delate status.
                            //sendMessage(new Message(UP_OPSReply,*deletedid,OPS_PlayWithDrawed)); // report delate status.
            }

        } 
        else if (updatestatus == OPSUpdateStatus::OPS_noopsstatus)
        {
            mTransManager->sendMessage(
                    new Message(OPSMsgUpdated, (void*) opsmsg,
                            (int) updatestatus));
        }
    }

    mOPSMsgAddList.clear();
    mOPSMsgUpdateList.clear();
    mOPSMsgDeletedList.clear();

    unsigned next = 0;

    getNextOPSMsgPlayTime(next);
    if (next != 0)
    {
        LogD("OPS next play time:%d\n", next);
        removeMessage(DL_OPSMsgUpdatedNotify);
        sendMessage(new Message(DL_OPSMsgUpdatedNotify), next);
    }

    return 0;
}

bool OPSHandler::getCurrOPSMsg(Json::OPSMsgDetail* &opsmsg)
{
    bool bret = false;

    std::string nextstarttime = "";
    for (std::map<int, Json::OPSMsgDetail*>::iterator iter =
            mOPSMsgDetailList.begin(); iter != mOPSMsgDetailList.end(); ++iter)
    {
        std::map<int, Json::OPSMsgDetail*>::iterator iter_played =
                mOPSMsgFinishedList.find(iter->first);
        if (iter_played != mOPSMsgFinishedList.end() && iter->second != NULL
                && iter_played->second->mBasic.mUpdateTime
                        == iter->second->mBasic.mUpdateTime)
        {
            continue;
        }

        std::string ltime = SystemClock::Today(SystemClockTMFormat);
        if (iter->second->mBasic.mEndTime <= ltime
                || iter->second->mBasic.mStartTime > ltime)
        {
            continue;
        }

        if (opsmsg == NULL)
        {
            opsmsg = iter->second;
            continue;
        }

        if (opsmsg->PriorityHigherThan(*(iter->second)))
        {
            continue;
        } 
        else if (opsmsg->PriorityLowerThan(*(iter->second)))
        {
            opsmsg = iter->second;
            continue;
        }
        else
        {
            if (iter->second->mBasic.mUpdateTime > opsmsg->mBasic.mUpdateTime)
            {
                opsmsg = iter->second;
                bret = true;
            }
        }
    } // end of for() loop

    return bret;
}

int OPSHandler::HandleOPSExecuteReply(int opsId, int opsRlst)
{
    const TransManager* transMgr = this->GetManager();
    if (NULL == transMgr)
        return -1;
    const ConfigParser* cfg = transMgr->GetConfig();
    if (NULL == cfg)
        return -1;

    // generate reply;
    Json::OPSReply theReply;
    theReply.mId = opsId;

    if (cfg->mLCDLEDFlag == LED_Controller_flag)
    { // led
        if (cfg->mDeviceAddrMap.size() > 0)
            theReply.mDevice = cfg->mDeviceAddrMap.begin()->first;
    } else
    { // lcd
        theReply.mDevice = cfg->mDeviceId;
    }

    theReply.mStatus = opsRlst;
    theReply.mRepTime = SystemClock::Today(SystemClockTMFormat);

    std::string jsonRply;
    Json::OPSReply::ToJson(&theReply, jsonRply);

    mTransManager->sendReply(this, DataTrans::DataTransType_String_PUT,
            UP_OPSReplyResult, jsonRply, &opsId);

    return 0;
}

bool OPSHandler::getNextOPSMsgPlayTime(unsigned& next)
{
    bool bret = false;

    Json::OPSMsgDetail* opsmsg = NULL;
    std::string currenttime = "";
    for (std::map<int, Json::OPSMsgDetail*>::iterator iter =
            mOPSMsgDetailList.begin(); iter != mOPSMsgDetailList.end(); ++iter)
    {
        std::string ltime = SystemClock::Today(SystemClockTMFormat);
        if (opsmsg == NULL && iter->second->mBasic.mStartTime > ltime)
        {
            opsmsg = iter->second;
            currenttime = ltime;
            continue;
        }

        if (opsmsg != NULL && iter->second->mBasic.mStartTime > ltime
                && iter->second->mBasic.mStartTime < opsmsg->mBasic.mStartTime)
        {
            opsmsg = iter->second;
            currenttime = ltime;
            continue;
        }
    }

    if (opsmsg != NULL)
    {
        uint64_t upstarttime;
        uint64_t upcurrtime;
        SystemClock::StrToUptimeMillis(opsmsg->mBasic.mStartTime, upstarttime,
        SystemClockTMFormat);
        SystemClock::StrToUptimeMillis(currenttime, upcurrtime,
        SystemClockTMFormat);

        next = upstarttime - upcurrtime;
        bret = true;
    } else
    {
        next = 0;
        bret = false;
    }

    return bret;
}

std::string OPSHandler::ModifyPath(const TransFileType dltype,
        const std::string& oriPath, void* param)
{
    const ConfigParser* cfg = mTransManager->GetConfig();
    if (NULL == cfg)
        return oriPath;

    std::stringstream ssUrl;
    ssUrl << oriPath;
    switch (dltype)
    {
    case DL_OPMsgList:
    {
        if (cfg->mLCDLEDFlag == LED_Controller_flag)
        { // led
            if (cfg->mDeviceAddrMap.size() > 0)
            {
                ssUrl << cfg->mDeviceAddrMap.begin()->first;
            }
        } else
        { // lcd
            ssUrl << cfg->mDeviceId;
        }

        return ssUrl.str();
    }
    case DL_OPMsgRequest:
    {
        if (NULL != param)
        {
            int lid = *((int*) (param));
            ssUrl << lid;
            return ssUrl.str();
        }
        break;
    }
    case UP_OPSReplyResult:
    {
        if (NULL != param)
        {
            int opsId = *((int*) (param));
            ssUrl << opsId << "/" << "reply";
            return ssUrl.str();
        }
        break;
    }
    default:
        // Do not need any modification on the path.
        break;
    }
    return oriPath;
}

bool OPSHandler::CheckOPSOutDated(Json::OPSMsgBasic& opsbasic)
{
    bool bret = false;
    std::string time = SystemClock::Today(SystemClockTMFormat);
    if (opsbasic.mStartTime > time || opsbasic.mEndTime < time)
    {
//        printf("OPSID-%d have out of date.\n", opsbasic.mId);
        bret = true;
    }

    return bret;
}

const char *OPSHandler::TAG = "OPSHandler";
