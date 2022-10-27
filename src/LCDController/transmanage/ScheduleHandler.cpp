/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : ScheduleHandler.cpp
 * @author : Benson
 * @date : Sep 21, 2017
 * @brief :
 */

#include <config/configparser.h>
#include <FileSysUtils.h>
#include <json/LayoutGroupFileHandler.h>
#include <json/MediaFileHandler.h>
#include <json/ScheduleFileHandler.h>
#include <json/UpdateListFileHandler.h>
#include <Log.h>
#include <Message.h>
#include <SystemClock.h>
#include <transmanage/ScheduleHandler.h>
#include <transmanage/TransManager.h>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <list>
#include <sstream>
#include <memory>
#include "bj_pis/convent/hd_layout.h"

const char *ScheduleHandler::TAG = "ScheduleHandler";

#define DELETE_SCHEDULE_MAPRESOURCE(obj,mapres)		do {		  \
		for (std::map<int, obj>::iterator it = mapres.begin(); it != mapres.end(); ++it) \
		{                                             \
			DELETE_ALLOCEDRESOURCE(it->second);       \
		}                                             \
		mapres.clear();                               \
	} while (0)

ScheduleHandler::ScheduleHandler(TransManager* manager) :
        ITransHandler(manager), mCurrentScheduleForPlay(NULL), mCurrentLayouGroupForPlay(
        NULL), mCurrentLayoutGroupDetail(NULL), mCurrentLayoutForPlay(NULL), mCurrentLayoutIndex(
                0)
{
    mUpdateListHandler = new Json::UpdateListFileHandler();
    mScheduleDetailHandler = new Json::ScheduleFileHandler();
    mLayoutGroupDetailHandler = new Json::LayoutGroupFileHandler();
    mMediaDetailHandler = new Json::MediaFileHandler();

    mScheduleListString = "";
}

ScheduleHandler::~ScheduleHandler()
{
    DELETE_ALLOCEDRESOURCE(mUpdateListHandler);
    DELETE_ALLOCEDRESOURCE(mScheduleDetailHandler);
    DELETE_ALLOCEDRESOURCE(mLayoutGroupDetailHandler);
    DELETE_ALLOCEDRESOURCE(mMediaDetailHandler);

    DELETE_SCHEDULE_MAPRESOURCE(Json::ScheduleDetail*, mScheduleForDL);
    DELETE_SCHEDULE_MAPRESOURCE(Json::LayoutGroupDetail*, mLayoutGroupForDL);
    DELETE_SCHEDULE_MAPRESOURCE(Json::MediaBasic*, mContentListForDL);

    DELETE_SCHEDULE_MAPRESOURCE(Json::ScheduleDetail*, mScheduleForPlay);
    DELETE_SCHEDULE_MAPRESOURCE(Json::LayoutGroupDetail*, mLayoutGroupForPlay);
    DELETE_SCHEDULE_MAPRESOURCE(Json::MediaBasic*, mContentListForPlay);
}

#include "bj_pis/convent/hd_layout.h"
#include "bj_pis/convent/hd_schedule.h"

void ScheduleHandler::Execute(Message* msg, int flag)
{
    if ( NULL == mTransManager)
        return;

    switch (flag)
    {
    case LD_FromLocal:
    {
        LogI("[Schedule]--Load local files!\n ");
        //LoadLocalSchedule();
        hd_schedule::LoadLocal();
        hd_layout::LoadLocal();
        HandleUpdateSchedule();
        break;
    }
    case LD_FromServer:
    {
        LogI("[Schedule]-- Send [get schedule update list] req!\n ");


        if(hasMessage(DL_SchedulesUpdateListReq))
        {
        	removeMessage(DL_SchedulesUpdateListReq);
        }

        mTransManager->sendRequest(this, DL_SchedulesUpdateList, NULL);
        break;
    }
    }
}

std::string ScheduleHandler::ModifyPath(const TransFileType dltype,
        const std::string& oriPath, void* param)
{
    const ConfigParser* cfg = mTransManager->GetConfig();
    if (NULL == cfg)
        return oriPath;

    std::stringstream ssUrl;
    ssUrl << oriPath;

    switch (dltype)
    {
    case DL_SchedulesUpdateList:
    {
        if (cfg->mLCDLEDFlag == LED_Controller_flag)
        { // led
            std::string theDevId;
            if (cfg->mDeviceAddrMap.size() > 0)
            {
                theDevId = cfg->mDeviceAddrMap.begin()->first;
            }

            ssUrl << theDevId << "&distributed=true";
        } else
        { // lcd
            ssUrl << cfg->mDeviceId << "&distributed=true";
        }

        LogD(" $$$$$$$$$ DL_SchedulesUpdateList new url is =%s \n",
                ssUrl.str().c_str());

        return ssUrl.str();
    }
    case DL_Schedules:
    case DL_LayoutGroups:
    case DL_Medias:
    {
        int* theId = static_cast<int*>(param);
        if (theId != NULL)
        {
            ssUrl << *theId;
            return ssUrl.str();
        }
        break;
    }
    case DL_MediaMD5:
    case DL_MediaContent:
    {
        std::string* fileName = static_cast<std::string*>(param);
        if (fileName != NULL)
        {
            // UTF8 convert
            std::string cvtName;
            curlwapper::HttpJsonTrans::escape_string(*fileName, cvtName);
            ssUrl << cvtName;
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

bool ScheduleHandler::handleMessage(Message* msg)
{
    if (NULL == msg)
        return false;

    int what = msg->mWhat;

    switch (what)
    {
    case DL_SchedulesUpdateListReq:
    {
    	LogI("[Schedule]--Handle [request to get schedule update list]!\n ");
    	mTransManager->sendRequest(this, DL_SchedulesUpdateList, NULL);
    	break;
    }
    case DL_SchedulesUpdateList:
    { // handle download schedule update list reply.
        LogI("[Schedule]--Handle [get schedule update list] reply!\n ");
        DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;
        HandleSchUpdateListReply(work, (DataTrans::DownloadStatus) msg->mArg1);

        DELETE_ALLOCEDRESOURCE(work);

        break;
    }
    case DL_Schedules:
    { // handle download schedule detail reply.
        LogI("[Schedule]--Handle get schedule detail reply!\n ");
        DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;
        HandleSchDetailReply(work, (DataTrans::DownloadStatus) msg->mArg1);

        DELETE_ALLOCEDRESOURCE(work);

        break;
    }
    case DL_LayoutGroups:
    {
        // handle download layout group detail reply.
        LogI("[Schedule]--Handle get layout group detail reply!\n ");
        DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;
        HandleLayoutGroupDetailReply(work,
                (DataTrans::DownloadStatus) msg->mArg1);

        DELETE_ALLOCEDRESOURCE(work);

        break;
    }
    case DL_MediaMD5:
    {
        LogI("[CMD]--Handle get media MD5 info reply!\n ");
        DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;
        HandleMediaMD5Reply(work, (DataTrans::DownloadStatus) msg->mArg1);

        DELETE_ALLOCEDRESOURCE(work);
        break;
    }
    case DL_Medias:
    { // handle download media detail reply.
        LogI("[CMD]--Handle get media detail reply!\n ");
        DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;
        HandleMediaDetailReply(work, (DataTrans::DownloadStatus) msg->mArg1);

        DELETE_ALLOCEDRESOURCE(work);

        break;
    }
    case DL_MediaContent:
    { // handle download media content file reply.
        LogI("[CMD]--Handle get media content reply!\n ");
        DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;

        if (DataTrans::DownloadSuccess != msg->mArg1 && work->mTransCount <= 3)
        {
            mTransManager->downloadTransWork(work);
        } else if (DataTrans::DownloadSuccess == msg->mArg1)
        {
            HandleMediaContentReply(work,
                    (DataTrans::DownloadStatus) msg->mArg1);
            DELETE_ALLOCEDRESOURCE(work);
        } else
        {
            LogD("Download media error(%s) !!! \n", work->mLocalFile.c_str());
            DELETE_ALLOCEDRESOURCE(work);
        }

        break;
    }
    case NT_ScheduleUpdatedNotify:
    {
        LogD("ScheduleUpdatedNotify - new schedule update.\n");
        HandleSyncDld2Play();
        HandleUpdateSchedule();
        break;
    }
    case NT_UpdateSchedule:
    {
        HandleUpdateSchedule();
        break;
    }
    case NT_UpdateLayoutGroup:
    {
        HandleUpdateLayoutGroup();
        break;
    }
    case NT_UpdateLayout:
    {
        HandleUpdateLayout();
        break;
    }
    default:
        break;
    }

    return false;
}

/**
 * Handle "get schedule update list" reply and send "get schedule detail" request.
 * @param work
 * @param status
 */
void ScheduleHandler::HandleSchUpdateListReply(DataTrans::TransWork *work,
        DataTrans::DownloadStatus status)
{
    if (DataTrans::DownloadSuccess != status || NULL == work)
    {
        // Download list failed, then load file from local.
        LoadLocalSchedule();

        return;
    }

    LogD("#################### schedule list: %s.\n", work->mJsonData.c_str());

    if(mScheduleListString == work->mJsonData)
    {
    	LogD("######### The same schedule list string, return.\n");
    	return;
    }

    // Download succeed, Then parse the result.
    Json::ScheduleList newScheduleList;
    bool ret = mUpdateListHandler->UpdateList(work->mJsonData.c_str(),
            &newScheduleList);
    if (!ret)
    {
        LogE("UpdateList parse failed!\n");
        return;
    }

    //if list was no schedule, return, avoid network issue cause no schedule
    if(newScheduleList.mSchedules.size() == 0)
    {
    	LogE("No schedule, return.\n");
    	return ;
    }

    mScheduleListKeepedID.clear();
    mScheduleListUpdatedID.clear();
    mScheduleListDeletedID.clear();

    bool blastScheduleDlCompleted = true;
    if(mScheduleForDL.size() >0 || mLayoutGroupForDL.size() > 0 || mContentListForDL.size() > 0)
    {
    	blastScheduleDlCompleted = false;
    }

    if(!blastScheduleDlCompleted)
    {
    	LogD("##### The pre schedule downloading, please distribute new schedule late #####\n");

    	removeMessage(DL_SchedulesUpdateListReq);
    	sendMessage(new Message(DL_SchedulesUpdateListReq), 600 * 1000);

    	return;
    	//mScheduleListForDL.mSchedules.clear();
    }


    mScheduleListString = work->mJsonData;

    // Check if any schedule canceled.
    bool changed = mUpdateListHandler->CheckUpdateStatus(newScheduleList,
            mScheduleListForDL, mScheduleListKeepedID, mScheduleListUpdatedID,
            mScheduleListDeletedID);
    if (false == changed)
    { // No schedule changed,do nothing.
        return;
    }
    ScheduleHandler::mScheduleListForDL = newScheduleList;

    // Clean up all download map.
    DELETE_SCHEDULE_MAPRESOURCE(Json::ScheduleDetail*, mScheduleForDL);
    DELETE_SCHEDULE_MAPRESOURCE(Json::LayoutGroupDetail*, mLayoutGroupForDL);
    DELETE_SCHEDULE_MAPRESOURCE(Json::MediaBasic*, mContentListForDL);

    if (mScheduleListKeepedID.size() > 0 && mScheduleListUpdatedID.size() == 0
            && mScheduleListDeletedID.size() == 0)
    {
        LogD("ScheduleList do not update.\n");
        return;
    } else if (mScheduleListKeepedID.size() == 0
            && mScheduleListUpdatedID.size() == 0) //no schedule to play
    {
        sendMessage(new Message(NT_ScheduleUpdatedNotify));
        return;
    }

    //deleted schedule detail only set to be NULL
    std::list<int>::const_iterator itor = mScheduleListDeletedID.begin();
    for (; itor != mScheduleListDeletedID.end(); ++itor)
    {
        LogD("    deleted schedule - %d\n", *itor);
    }

    std::list<int>::const_iterator itor1 = mScheduleListKeepedID.begin();
    for (; itor1 != mScheduleListKeepedID.end(); ++itor1)
    {
        LogD("    keedped schedule - %d\n", *itor1);
        int schId = *itor1;

        // Add the schedule id to map and set value to 'NULL'!
        // After download the certain schedule, it will be set to a certain value!
        mScheduleForDL[schId] = NULL;

        mTransManager->sendRequest(this, DL_Schedules, &schId);
    }

    std::list<int>::const_iterator itor2 = mScheduleListUpdatedID.begin();
    for (; itor2 != mScheduleListUpdatedID.end(); ++itor2)
    {
        LogD("    updated schedule - %d\n", *itor2);
        int schId = *itor2;

        // Add the schedule id to map and set value to 'NULL'!
        // After download the certain schedule, it will be set to a certain value!
        mScheduleForDL[schId] = NULL;

        mTransManager->sendRequest(this, DL_Schedules, &schId);
    }
}

/**
 * Handle "get schedule detail reply" and send "get layout group detail" request and send "get media detail" request.
 * @param work
 * @param status
 */
void ScheduleHandler::HandleSchDetailReply(DataTrans::TransWork *work,
        DataTrans::DownloadStatus status)
{
    if (DataTrans::DownloadSuccess != status || NULL == work)
    {
        // TODO: download detail failed!
        return;
    }

    LogD("#################### schedule detail: %s\n", work->mJsonData.c_str());
    // Download succeed, Then parse the result.
    Json::ScheduleDetail* scheduleDtl = new Json::ScheduleDetail();
    bool rslt = mScheduleDetailHandler->UpdateScheduleDetail(
            work->mJsonData.c_str(), scheduleDtl);

    if (true == rslt)
    {
        mScheduleForDL[scheduleDtl->mScheduleBasic.mId] = scheduleDtl;

        std::vector<Json::SchLayoutGroupBasic>::const_iterator itor =
                scheduleDtl->mLayoutGroups.begin();

        // download layout group detail.
        for (; itor != scheduleDtl->mLayoutGroups.end(); ++itor)
        {
            if (mLayoutGroupForDL.find(itor->mId) == mLayoutGroupForDL.end())
            {        // not download yet.
                int layoutGroupId = itor->mId;

                /* Add the layout group id to map and set value to 'NULL'!
                 After download the certain layout group, it will be set to a certain value!*/
                mLayoutGroupForDL[layoutGroupId] = NULL;

                /* Send request to download layout group detail.*/
                mTransManager->sendRequest(this, DL_LayoutGroups,
                        &layoutGroupId);
            }

            // download media detail.
            std::vector<Json::PartitionMedias>::const_iterator itor2 =
                    itor->mPartionMedias.begin();
            for (; itor2 != itor->mPartionMedias.end(); ++itor2)
            {
                std::vector<int>::const_iterator itor3 =
                        itor2->mMediaIds.begin();
                for (; itor3 != itor2->mMediaIds.end(); ++itor3)
                {
                    if (mContentListForDL.find(*itor3)
                            == mContentListForDL.end())
                    {        // media not download yet.
                        int mediaId = *itor3;

                        /* Add the media id to map and set value to 'NULL'!
                         After download the certain media, it will be set to a certain value!*/
                        mContentListForDL[mediaId] = NULL;

                        /* Send request to download media detail.*/
                        mTransManager->sendRequest(this, DL_Medias, &mediaId);
                    }
                }        // end of for(mMediaIds)
            }        // end of for(mPartionMedias)
        }        // end of for( mLayoutGroups)
    } else
    {
        LogE("UpdateSchedule detail parse failed!\n");
        DELETE_ALLOCEDRESOURCE(scheduleDtl);
    }

    return;
}

/**
 * Handle "get layout group detail" reply and send "get media md5 info" request(partition back-images).
 * @param work
 * @param status
 */
void ScheduleHandler::HandleLayoutGroupDetailReply(DataTrans::TransWork *work,
        DataTrans::DownloadStatus status)
{
    if (DataTrans::DownloadSuccess != status || NULL == work)
    {
        // TODO: download detail failed!
        return;
    }
    LogD("#################### Layout detail: %s\n", work->mJsonData.c_str());
    const ConfigParser* cfg = mTransManager->GetConfig();
    if (NULL == cfg)
        return;

    // Download succeed, Then parse the result.
    Json::LayoutGroupDetail* layoutGroupDtl = new Json::LayoutGroupDetail();
    bool rslt = mLayoutGroupDetailHandler->UpdateLayoutGroupDetail(
            work->mJsonData.c_str(), layoutGroupDtl);

    if (true == rslt)
    {
        if (mLayoutGroupForDL.find(layoutGroupDtl->mId)
                != mLayoutGroupForDL.end())
        {
            /* Find the layout group detail id in map. update the layout group detail*/
            mLayoutGroupForDL[layoutGroupDtl->mId] = layoutGroupDtl;

            std::vector<Json::LayoutDetail>::const_iterator it =
                    layoutGroupDtl->mLayoutDetails.begin();

            for (; it != layoutGroupDtl->mLayoutDetails.end(); ++it)
            {
                // download background image.
                std::vector<Json::PartitionDetail>::const_iterator it2 =
                        it->mPartitions.begin();
                for (; it2 != it->mPartitions.end(); ++it2)
                {
                    std::string bkImg = it2->mBkgroudFile;

                    if (bkImg.size() > 0
                            && mResourceDownloadList.find(bkImg)
                                    == mResourceDownloadList.end())
                    {

                        mResourceDownloadList[bkImg] = RestReadyDownload;

                        /* Send request to download media information.*/
                        //LogD("@@@@@@@@@@@@@@@@@@@@@@@@@@@ partationid:%d,  bkfile:%s\n",it2->mId,bkImg.c_str());
                        //mTransManager->sendRequest(this, DL_MediaMD5, &bkImg);
                    }
//                    if (mContentListForDL.find(mediaId)
//                            == mContentListForDL.end())
//                    {   // not download yet.
//
//                        /* Add the media id to map and set value to 'NULL'!
//                         After download the certain media, it will be set to a certain value!*/
//                        mContentListForDL[mediaId] = NULL;
//
//                        /* Send request to download media detail.*/
//                        mTransManager->sendRequest(this, DL_Medias, &mediaId);
//                    }

                }   // end of for(mPartitions)

            }   // end of for(mLayoutDetails)
        }

        CheckSchDownloadCompleted();
    } else
    {
        LogE("UpdateLayout detail parse failed!\n");
        DELETE_ALLOCEDRESOURCE(layoutGroupDtl);
    }

    return;
}

/**
 * Handle "get media md5 info" reply and send "download media content file" request(media back-image or media file).
 * @param work
 * @param status
 */
void ScheduleHandler::HandleMediaMD5Reply(DataTrans::TransWork *work,
        DataTrans::DownloadStatus status)
{
    LogD("#################### Media MD5 status: %d\n", status);
    if (DataTrans::DownloadSuccess != status || NULL == work)
    {
        // TODO: download detail failed!
        return;
    }

    //Json data is empty but return DownloadSuccess
    if (work->mJsonData.size() <= 2)
    {
        return;
    }

    LogD("#################### Media MD5 detail: %s\n",
            work->mJsonData.c_str());
    Json::FileInfo md5Info;
    bool rslt = Json::FileInfo::Parse(work->mJsonData.c_str(), &md5Info);
    if (true == rslt)
    {
        LogI("############################  filename %s,MD5 = %s\n",
                md5Info.mFilePath.c_str(), md5Info.mMD5.c_str());
        /* send request to download media content file*/
        mTransManager->downloadFile(this, DL_MediaContent, md5Info.mFilePath,
                md5Info.mMD5, &(md5Info.mFilePath));
    }
}

/**
 * Handle "get media detail" reply and send "get media MD5 info" request(media back-image or media file).
 * @param work
 * @param status
 */
void ScheduleHandler::HandleMediaDetailReply(DataTrans::TransWork *work,
        DataTrans::DownloadStatus status)
{
    LogD("#################### Media detail status: %d\n", status);
    if (DataTrans::DownloadSuccess != status || NULL == work)
    {
        // TODO: download detail failed!
        return;
    }
    LogD("#################### Media detail: %s\n", work->mJsonData.c_str());
    const ConfigParser* cfg = mTransManager->GetConfig();
    if (NULL == cfg || NULL == mMediaDetailHandler)
        return;

    Json::MediaBasic::MediaType type;
    Json::MediaBasic* mediaBsc = NULL;

    bool rslt = mMediaDetailHandler->UpdateMediaDetail(work->mJsonData.c_str(),
            type, &mediaBsc);

    if (true == rslt && NULL != mediaBsc)
    {
        if (mContentListForDL.find(mediaBsc->mId) != mContentListForDL.end())
        {
            // store the media detail to map.
            mContentListForDL[mediaBsc->mId] = mediaBsc;

            std::string filename = "";
            if (mContentListForDL[mediaBsc->mId]->mType
                    == Json::MediaBasic::Text)
            {
                Json::MediaText* mediatext =
                        dynamic_cast<Json::MediaText*>(mContentListForDL[mediaBsc->mId]);
                if (mediatext->mParams.mBackImage.size() > 0
                        && !FileSysUtils::Accessible(
                                cfg->mDldRootDir + cfg->mContentpath
                                        + mediatext->mParams.mBackImage,
                                FileSysUtils::FR_OK))
                    mResourceDownloadList[mediatext->mParams.mBackImage] =
                            RestReadyDownload;
            } else if (mContentListForDL[mediaBsc->mId]->mType
                    == Json::MediaBasic::Image
                    || mContentListForDL[mediaBsc->mId]->mType
                            == Json::MediaBasic::Video
                    || mContentListForDL[mediaBsc->mId]->mType
                            == Json::MediaBasic::Live
                    || mContentListForDL[mediaBsc->mId]->mType
                            == Json::MediaBasic::AnalogClock
                    || mContentListForDL[mediaBsc->mId]->mType
                            == Json::MediaBasic::Flash)
            {
                Json::MediaCommon2* media =
                        dynamic_cast<Json::MediaCommon2*>(mContentListForDL[mediaBsc->mId]);
                if (media->mFile.size() > 0
                        && !FileSysUtils::Accessible(
                                cfg->mDldRootDir + cfg->mContentpath
                                        + media->mFile, FileSysUtils::FR_OK))
                    mResourceDownloadList[media->mFile] = RestReadyDownload;
            } else if (mContentListForDL[mediaBsc->mId]->mType
                    == Json::MediaBasic::ArrivalMsg
                    || mContentListForDL[mediaBsc->mId]->mType
                            == Json::MediaBasic::DigitalClock
                    || mContentListForDL[mediaBsc->mId]->mType
                            == Json::MediaBasic::Weather)
            {
                Json::MediaCommon1* mediaarrmsg =
                        dynamic_cast<Json::MediaCommon1*>(mContentListForDL[mediaBsc->mId]);

                for (std::vector<Json::LabelInfo>::iterator it =
                        mediaarrmsg->mParams.begin();
                        it != mediaarrmsg->mParams.end(); ++it)
                {
                    if ((*it).mVarText.mText.mBackImage.size() > 0
                            && !FileSysUtils::Accessible(
                                    cfg->mDldRootDir + cfg->mContentpath
                                            + (*it).mVarText.mText.mBackImage,
                                    FileSysUtils::FR_OK))
                        mResourceDownloadList[(*it).mVarText.mText.mBackImage] =
                                RestReadyDownload;
                }
            }

        }

        CheckSchDownloadCompleted();
    } else
    {
        DELETE_ALLOCEDRESOURCE(mediaBsc);
    }
}

/**
 * Handle "download media content file" result.
 * @param work
 * @param status
 */
void ScheduleHandler::HandleMediaContentReply(DataTrans::TransWork *work,
        DataTrans::DownloadStatus status)
{
    if (DataTrans::DownloadSuccess != status || NULL == work)
    {
        LogE(
                "############################  Download media content[%s] failed! Status = %d\n",
                work->mLocalFile.c_str(), status);

        // TODO: download content file failed!
        return;
    }

    LogD(
            "############################  media content   %s\n download succeed!!\n",
            work->mLocalFile.c_str());

    for (std::map<std::string, enum RestResourceDownloadStatus>::iterator it =
            mResourceDownloadList.begin(); it != mResourceDownloadList.end();)
    {
        if (work->mLocalFile == it->first)
        {
            mResourceDownloadList[it->first] = RestDownloaded;

            it = mResourceDownloadList.erase(it);
            LogD("checkResourceUpdate - file(%s) had been downloaded.\n",
                    work->mLocalFile.c_str());
            break;
        }
    }

    CheckSchDownloadCompleted();
}

/**
 * Sync the schedule detail,layout detail and media detail from 'download' map
 * to related 'play' map.
 */
void ScheduleHandler::HandleSyncDld2Play()
{
    //1. save download schedule json data to local json file.
    const ConfigParser* config = mTransManager->GetConfig();
    std::string sDownloadPath = config->mDldRootDir + config->mSchDldPath;
    std::string sPlayPath = config->mDldRootDir + config->mSchPlayPath;

    mUpdateListHandler->SetFileName(
            sDownloadPath + config->mSchedulesListFilename);
    mUpdateListHandler->Save();

    mScheduleDetailHandler->SetFileName(
            sDownloadPath + config->mSchedulesFilename);
    mScheduleDetailHandler->Save();

    mLayoutGroupDetailHandler->SetFileName(
            sDownloadPath + config->mLayoutGroupFilename);
    mLayoutGroupDetailHandler->Save();

    mMediaDetailHandler->SetFileName(
            sDownloadPath + config->mResourcesFilename);
    mMediaDetailHandler->Save();

    //2. copy json from download folder to play folder
    std::string copycmd = "cp -f " + sDownloadPath
            + config->mSchedulesListFilename + " " + sPlayPath
            + config->mSchedulesListFilename;
    system(copycmd.c_str());
    copycmd = "cp -f " + sDownloadPath + config->mSchedulesFilename + " "
            + sPlayPath + config->mSchedulesFilename;
    system(copycmd.c_str());
    copycmd = "cp -f " + sDownloadPath + config->mLayoutGroupFilename + " "
            + sPlayPath + config->mLayoutGroupFilename;
    system(copycmd.c_str());
    copycmd = "cp -f " + sDownloadPath + config->mResourcesFilename + " "
            + sPlayPath + config->mResourcesFilename;
    system(copycmd.c_str());

    //3. notify play new schedule
    //mScheduleForDL -  mScheduleForPlay
    if (mScheduleForDL.size() == 0) //noschedule
    {
        DELETE_SCHEDULE_MAPRESOURCE(Json::ScheduleDetail*, mScheduleForDL);
        DELETE_SCHEDULE_MAPRESOURCE(Json::LayoutGroupDetail*,
                mLayoutGroupForDL);
        DELETE_SCHEDULE_MAPRESOURCE(Json::MediaBasic*, mContentListForDL);

        DELETE_SCHEDULE_MAPRESOURCE(Json::ScheduleDetail*, mScheduleForPlay);
        DELETE_SCHEDULE_MAPRESOURCE(Json::LayoutGroupDetail*,
                mLayoutGroupForPlay);
        DELETE_SCHEDULE_MAPRESOURCE(Json::MediaBasic*, mContentListForPlay);
        return;
    }

    for (std::map<int, Json::ScheduleDetail*>::iterator i =
            mScheduleForDL.begin(); i != mScheduleForDL.end(); ++i)
    {
        // check weather items in ScheduleForPlay been deleted in ScheduleForDL
        std::map<int, Json::ScheduleDetail*>::iterator iter =
                mScheduleForPlay.find(i->first);

        // new schedule
        if (iter == mScheduleForPlay.end())
        {
            mScheduleForPlay[i->first] = i->second;
            continue;
        }

        //updated schedule
        if (iter != mScheduleForPlay.end() && i->second != NULL)
        {
            DELETE_ALLOCEDRESOURCE(iter->second);

            mScheduleForPlay[i->first] = i->second;

            continue;
        }
    }    // end of for( mScheduleForDL )

    for (std::list<int>::iterator i = mScheduleListDeletedID.begin();
            i != mScheduleListDeletedID.end(); ++i)
    {
        std::map<int, Json::ScheduleDetail*>::iterator iter =
                mScheduleForPlay.find(*i);
        if (iter != mScheduleForPlay.end())
        {
            DELETE_ALLOCEDRESOURCE(iter->second);
            mScheduleForPlay.erase(iter);
        }
    }
    mScheduleForDL.clear();

    for (std::map<int, Json::ScheduleDetail*>::iterator i =
            mScheduleForPlay.begin(); i != mScheduleForPlay.end(); ++i)
    {
        LogD(
                "*************************************************   11 mScheduleForPlay - %d\n",
                i->first);
    }

    //mLayoutListForDLRest -> mLayoutListForPlayRest
    for (std::map<int, Json::LayoutGroupDetail*>::iterator j =
            mLayoutGroupForDL.begin(); j != mLayoutGroupForDL.end(); ++j)
    {
        std::map<int, Json::LayoutGroupDetail*>::iterator iter =
                mLayoutGroupForPlay.find(j->first);
        if (iter == mLayoutGroupForPlay.end())
        {    // new layout group
            mLayoutGroupForPlay[j->first] = j->second;
        } else
        {    // found in mLayoutGroupForPlay
            DELETE_ALLOCEDRESOURCE(iter->second);

            mLayoutGroupForPlay[j->first] = j->second;
        }
    }
    mLayoutGroupForDL.clear();

    //mContentListForDLRest -> mContentListForPlayRest
    for (std::map<int, Json::MediaBasic*>::iterator k =
            mContentListForDL.begin(); k != mContentListForDL.end(); ++k)
    {
        std::map<int, Json::MediaBasic*>::iterator iter =
                mContentListForPlay.find(k->first);
        if (iter == mContentListForPlay.end())
        {    // new content
            mContentListForPlay[k->first] = k->second;
        } else
        {
            DELETE_ALLOCEDRESOURCE(iter->second);

            mContentListForPlay[k->first] = k->second;
        }

    }
    mContentListForDL.clear();
}

void ScheduleHandler::HandleUpdateSchedule()
{
    LogD("Update Schedule --------------------------time:%s\n",
            SystemClock::Today(SystemClockTMFormat).c_str());
    unsigned next = 0;

    Json::ScheduleDetail* cs = NULL;
    cs = GetCurrentSchedule(next);

    if ((mCurrentScheduleForPlay == NULL && cs != NULL)
            || (cs == NULL && mCurrentScheduleForPlay != NULL)
            || (cs != NULL && mCurrentScheduleForPlay != NULL
                    /*&& mCurrentScheduleForPlay != cs*/))
    //if (cs != NULL && (mCurrentScheduleForPlay == NULL || (mCurrentScheduleForPlay != NULL && cs->mScheduleBasic.mId != mCurrentScheduleForPlay->mScheduleBasic.mId)))
    //if (cs != NULL && cs != mCurrentScheduleForPlay)
    {
        mCurrentScheduleForPlay = cs;

        //LogE("###########  schedule id - %d\n",mCurrentScheduleForPlayRest->mScheduleBasic.mId);
        removeMessage(NT_UpdateLayoutGroup);
        sendMessage(new Message(NT_UpdateLayoutGroup));
        if (mCurrentScheduleForPlay != NULL)
            LogE("Ready to play schedule ID=%d\n",
                    mCurrentScheduleForPlay->mScheduleBasic.mId);
    } else
    {
        LogE(" --------------------------- handleUpdateSchedule\n");
    }

    removeMessage(NT_UpdateSchedule);
    if (next != 0)
    {
        LogE("######################## next - %d\n", next);
        sendMessage(new Message(NT_UpdateSchedule), next);
    }

}

void ScheduleHandler::HandleUpdateLayoutGroup()
{
    LogD("Update Layout group --------------------------time:%s\n",
            SystemClock::Today(SystemClockTMFormat).c_str());
    unsigned next = 0;

    Json::SchLayoutGroupBasic* schLayoutGroup = NULL;
    schLayoutGroup = GetCurrentLayoutGroup(next);

//    if ((schLayoutGroup == NULL)
//            || (schLayoutGroup != NULL
//                    && schLayoutGroup != mCurrentLayouGroupForPlay))
//    {
        mCurrentLayouGroupForPlay = schLayoutGroup;

        // Get the current layout group detail according to id.
        mCurrentLayoutGroupDetail = GetCurrentLayoutGroupDetail(
                mCurrentLayouGroupForPlay);

        removeMessage(NT_UpdateLayout);
        sendMessage(new Message(NT_UpdateLayout));

        if (mCurrentLayouGroupForPlay != NULL)
            LogD("Ready to play LayoutGroup ID=%d\n",
                    mCurrentLayouGroupForPlay->mId);
//    } else
//    {
//        LogD(
//                " --------------------------- HandleUpdateLayoutGroup,not any updated \n");
//    }

    LogD("****** HandleUpdateLayoutGroup, next = %d ms\n", next);
    removeMessage(NT_UpdateLayoutGroup);
    if (next != 0)
    {
        sendMessage(new Message(NT_UpdateLayoutGroup), next);
    }
}

void ScheduleHandler::HandleUpdateLayout()
{
    LogD("Update Layout --------------------------time:%s\n",
            SystemClock::Today(SystemClockTMFormat).c_str());
    unsigned next = 0;

    Json::LayoutDetail* layout = NULL;
    layout = GetCurrentLayout(next);

    //if ((layout == NULL) || (layout != NULL && layout != mCurrentLayoutForPlay))
    //{
        mCurrentLayoutForPlay = layout;
        if (mCurrentLayoutForPlay != NULL)
        {
            LogI("##########  current layout id=%d\n",
                    mCurrentLayoutForPlay->mId);
            // find all media, current layout need.
            Json::LayoutInfo4Qt* fullLayoutInfo = new Json::LayoutInfo4Qt();

            FindCurrentLayoutRelatedMedia(fullLayoutInfo);

            mTransManager->sendMessage(
                    new Message(LayoutUpdated, fullLayoutInfo));
        } else
        {
            mTransManager->sendMessage(
                    new Message(LayoutUpdated, (void*) NULL));
        }
   // }

    removeMessage(NT_UpdateLayout);
    if (next != 0)
    {
        LogD("****** UpdateScheduleTimeSlot, next = %d ms\n", next);
        CurrentLayoutIndexAdd(); // update the next time layout index.
        sendMessage(new Message(NT_UpdateLayout), next);
    }
}

bool ScheduleHandler::CheckSchDownloadCompleted()
{
    bool isschpkgdlcompleted = true;
    bool isresourcedlcompleted = true;

    /* Check weather schedule detail finished. */
    for (std::map<int, Json::ScheduleDetail*>::iterator i =
            mScheduleForDL.begin(); i != mScheduleForDL.end(); ++i)
    {
        if (i->second == NULL)
        {
            /*LogE("checkSchDownloadCompleted mScheduleForDLRest - %d\n",
             i->first);*/

            LogI(
                    "\n\t\t $$$$$$$$$$$ checkSchDownloadCompleted mScheduleForDLRest ,sch id=%d,val==NULL\n",
                    i->first);

            isschpkgdlcompleted = false;
            break;
        }
    }

    /* Check weather layout group detail finished. */
    for (std::map<int, Json::LayoutGroupDetail*>::iterator j =
            mLayoutGroupForDL.begin(); j != mLayoutGroupForDL.end(); ++j)
    {
        if (j->second == NULL)
        {
            /*LogE("checkSchDownloadCompleted mLayoutGroupForDL - %d\n",
             j->first);*/
            LogI(
                    "\n\t\t $$$$$$$$$$$ checkSchDownloadCompleted mLayoutGroupForDL ,layouGroup id=%d,val==NULL\n",
                    j->first);
            isschpkgdlcompleted = false;
            break;
        }
    }

    /* Check weather media detail finished. */
    for (std::map<int, Json::MediaBasic*>::iterator k =
            mContentListForDL.begin(); k != mContentListForDL.end(); ++k)
    {
        if (k->second == NULL)
        {
            /*LogE("checkSchDownloadCompleted mContentListForDL - %d\n",
             k->first);*/

            LogI(
                    "\n\t\t $$$$$$$$$$$ checkSchDownloadCompleted mContentListForDL ,content id=%d,val==NULL\n",
                    k->first);

            isschpkgdlcompleted = false;
            break;
        }
    }

    /* Check weather media content file finished. */
    if (mResourceDownloadList.size() == 0)
    {
        LogD("checkSchDownloadCompleted - Contents download completed.\n");
        isresourcedlcompleted = true;
    } else
    {
        isresourcedlcompleted = false;
    }

    LogE(
            "checkSchDownloadCompleted - isschpkgdlcompleted-%d,  isresourcedlcompleted-%d\n",
            isschpkgdlcompleted, isresourcedlcompleted);

    if (isschpkgdlcompleted)
    {
        /* Schedule detail, layout group detail and media detail download finished!
         * But media content file not finish download yet */
        if (mResourceDownloadList.size() > 0)
        {
            for (std::map<std::string, enum RestResourceDownloadStatus>::iterator it =
                    mResourceDownloadList.begin();
                    it != mResourceDownloadList.end(); ++it)
            {
                if (it->second == RestReadyDownload)
                {
                    mResourceDownloadList[it->first] = RestDownloading;
                    std::string mediaFileName = it->first;

                    /* send request to get media information(MD5)*/
                    mTransManager->sendRequest(this, DL_MediaMD5,
                            &mediaFileName);
                }
            }
        }
    }

    for (std::map<std::string, enum RestResourceDownloadStatus>::iterator it =
            mResourceDownloadList.begin(); it != mResourceDownloadList.end();
            ++it)
    {
        LogD(
                "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&  media content name: %s\n",
                it->first.c_str());
    }

    if (isschpkgdlcompleted && isresourcedlcompleted)
    {
        sendMessage(new Message(NT_ScheduleUpdatedNotify));

        if(hasMessage(DL_SchedulesUpdateListReq))
        {
        	removeMessage(DL_SchedulesUpdateListReq);
        	sendMessage(new Message(DL_SchedulesUpdateListReq), 1000);
        }
    }

    return isschpkgdlcompleted;
}
#include "bj_pis/convent/hd_schedule.h"
Json::ScheduleDetail* ScheduleHandler::GetCurrentSchedule(unsigned &next)
{
    std::string time = SystemClock::Today(SystemClockTMFormat);
    Json::ScheduleDetail* currsch = NULL;

    //search the current schedule ready to play
    currsch = FindSchedule(time);
    hd_schedule::SetPlaylistMedia(currsch,this);
    std::string nexttime = "";
    Json::ScheduleDetail* nextsch = NULL;


    //search the next schedule ready to play
//    if (nexttime.length() == 0)
//    {
        nextsch = FindNextSchedule(nexttime,currsch);

        if(nextsch != NULL && currsch != NULL &&
        		nextsch->mScheduleBasic.mId == currsch->mScheduleBasic.mId)
        {
        	nexttime = time.substr(0,9) + currsch->mScheduleBasic.mEndTime.substr(9,6);
        }
        else if (nextsch != NULL)
        {
        	LogD("next schedule id=%d\n",nextsch->mScheduleBasic.mId);
            nexttime = time.substr(0,9) + nextsch->mScheduleBasic.mStartTime.substr(9,6);
        }
        else  if (nextsch == NULL && currsch != NULL)
        {
            nexttime = time.substr(0,9) +currsch->mScheduleBasic.mEndTime.substr(9,6);
        }
  //  }

    if (nexttime.length() == 0)
    {
    	LogD("No Schedule, check every 60 second\n");
        next = 60000;
    } else
    {
    	LogD("next time:%s\n",nexttime.c_str());
        uint64_t uptime;
        SystemClock::StrToUptimeMillis(nexttime, uptime, SystemClockTMFormat);

        next = uptime - SystemClock::UptimeMillis();
    }

    return currsch;
}

Json::SchLayoutGroupBasic* ScheduleHandler::GetCurrentLayoutGroup(
        unsigned &next)
{
    std::string time = SystemClock::Today(SystemClockTMFormat);
    Json::SchLayoutGroupBasic* currGroup = NULL;

    if (mCurrentScheduleForPlay == NULL)
        return NULL;

    currGroup = FindLayoutGroup(time);

    // calculate the next time to 'GetCurrentLayoutGroup'
    std::string nexttime = "";
    Json::SchLayoutGroupBasic* nextGroup = NULL;

//    if (nexttime.length() == 0)
//    {
        nextGroup = FindNextLayoutGroup(nexttime);

        if (nextGroup != NULL)
        {
            LogD("next layout group id-%d  %s\n",nextGroup->mId,
                    nextGroup->mStartTime.c_str());
            nexttime = nextGroup->mStartTime;
        }
        else  if (nextGroup == NULL && currGroup != NULL)
        {
            nexttime = currGroup->mEndTime;
        }
  //  }

    if (nexttime.length() == 0)
    {
        next = 0;
    } else
    {
        uint64_t uptime;
        SystemClock::StrToUptimeMillis(nexttime, uptime, SystemClockTMFormat);

        next = uptime - SystemClock::UptimeMillis();
    }

    return currGroup;
}

Json::LayoutGroupDetail* ScheduleHandler::GetCurrentLayoutGroupDetail(
        Json::SchLayoutGroupBasic* bsc)
{
    if (NULL == bsc)
        return NULL;

    // according to current play schLayoutGroup id, find layout group detail.
    int layoutGroupId = bsc->mId;

    std::map<int, Json::LayoutGroupDetail*>::const_iterator itor =
            mLayoutGroupForPlay.find(layoutGroupId);

    if (itor == mLayoutGroupForPlay.end())
    {    // the group id not found in layout group for play.
        LogD("------ Group id[%d] not found in mLayoutGroupForPlay!!\n");
        return NULL;
    } else
    {
        // the group id found.
        if (NULL == itor->second)
        {
            LogD(
                    "------ Group id[%d] found in mLayoutGroupForPlay, but Value is NULL!!\n");
            return NULL;
        }
    }

    // the group id is found in mLayoutGroupForPlay and it's value is not NULL.
    Json::LayoutGroupDetail* groupDeatil = itor->second;

    return groupDeatil;
}

Json::LayoutDetail* ScheduleHandler::GetCurrentLayout(unsigned& next)
{
    std::vector<Json::LayoutDetail>::const_iterator iter;
    std::string time = SystemClock::Today(SystemClockTMFormat);

    if ( NULL == mCurrentLayoutGroupDetail)
        return NULL;

    // find the layout to play current now.
    int currIndex = GetCurrentLayoutIndex();
    Json::LayoutDetail* dtl =
            &(mCurrentLayoutGroupDetail->mLayoutDetails[currIndex]);

    next = mCurrentLayouGroupForPlay->mSwitchTime * 1000;

    return dtl;
}

void ScheduleHandler::CurrentLayoutIndexAdd()
{
    if (NULL == mCurrentLayoutGroupDetail)
        return;

    unsigned int size = mCurrentLayoutGroupDetail->mLayoutDetails.size();

    if (++mCurrentLayoutIndex >= size)
    {
        mCurrentLayoutIndex = 0; // reset to the first one.
    }

    return;
}

bool ScheduleHandler::LoadLocalSchedule()
{
    // Load local files.
    const ConfigParser* cfg = mTransManager->GetConfig();
    if (NULL == cfg || NULL == mUpdateListHandler
            || NULL == mScheduleDetailHandler
            || NULL == mLayoutGroupDetailHandler || NULL == mMediaDetailHandler)
        return false;

    std::string fileDir = cfg->mDldRootDir + cfg->mSchPlayPath;

    // 1.load schedule_list.json
    std::string filePath = fileDir + cfg->mSchedulesListFilename;
    if (!mUpdateListHandler->LoadSchlist(filePath, mScheduleListForDL))
    {
        LogE("LoadSchlist failed!!\n");
        return false;
    }

    // 2.load schedules.json
    filePath = fileDir + cfg->mSchedulesFilename;
    if (!mScheduleDetailHandler->LoadScheduleDetail(filePath, mScheduleForPlay))
    {
        LogE("LoadScheduleDetail failed!!\n");
        return false;
    }

    // 3.load layout_groups.json
    filePath = fileDir + cfg->mLayoutGroupFilename;
    if (!mLayoutGroupDetailHandler->LoadLayoutGroupDetail(filePath,
            mLayoutGroupForPlay))
    {
        LogE("LoadLayoutGroupDetail failed!!  %s\n", filePath.c_str());
        return false;
    }

    // 4.load resources.json
    filePath = fileDir + cfg->mResourcesFilename;
    if (!mMediaDetailHandler->LoadMediaDetail(filePath, mContentListForPlay))
    {
        LogE("LoadMediaDetail failed!!\n");
        return false;
    }

    HandleUpdateSchedule();

    return false;
}

int ScheduleHandler::GetCurrentLayoutIndex() const
{
    //LogI("Current LayoutIndex in group is=%d\n", mCurrentLayoutIndex);
    return mCurrentLayoutIndex;
}

/**
 * Find scheduleDetail according to schedule strategy and given time.
 * @param timePoint[in]: the time to judge.
 * @return
 */
Json::ScheduleDetail* ScheduleHandler::FindSchedule(
        const std::string& timePoint)
{
    Json::ScheduleDetail* retSch = NULL;

    std::map<int, Json::ScheduleDetail*>::iterator iter;
    for (iter = mScheduleForPlay.begin(); iter != mScheduleForPlay.end();
            ++iter)
    {
    	std::string currHMS = timePoint.substr(9,6);
    	std::string startHMS = iter->second->mScheduleBasic.mStartTime.substr(9,6);
    	std::string endHMS = iter->second->mScheduleBasic.mEndTime.substr(9,6);
        if ((iter->second != NULL
        		&& currHMS >= startHMS
				&& currHMS < endHMS
				&&timePoint >= iter->second->mScheduleBasic.mStartTime
            && timePoint < iter->second->mScheduleBasic.mEndTime)||iter->second->mScheduleBasic.mPriority==999)
        {
            if (retSch == NULL)
            { // the first schedule.
                retSch = iter->second;
            } else
            {
           //选择优先级更高的
                // 1.judge the priority .
                if (iter->second->mScheduleBasic.mPriority
                        > retSch->mScheduleBasic.mPriority)
                {
                    retSch = iter->second;
                } else if (iter->second->mScheduleBasic.mPriority
                        == retSch->mScheduleBasic.mPriority)//优先级如果相同，选择server level更高级的,0比1高级
                {
                    //0-pcc > 1-occ > 2 station
                    if (iter->second->mScheduleBasic.mServerLevel
                            < retSch->mScheduleBasic.mServerLevel)
                    {
                        retSch = iter->second;
                    }
                    if (iter->second->mScheduleBasic.mServerLevel
                            == retSch->mScheduleBasic.mServerLevel)//如果server level相同，选择发布时间更新的
                    {
                        // 2. priority is same,judge the publish time.
                        if (iter->second->mScheduleBasic.mPublishTime
                                > retSch->mScheduleBasic.mPublishTime)
                        {
                            retSch = iter->second;
                        }
                    }
                }
            }
        } // end of if (starttime endtime judge)
    } //end of for(mScheduleForPlay)

    return retSch;
}

Json::ScheduleDetail* ScheduleHandler::FindNextSchedule(
        const std::string& timePoint,const Json::ScheduleDetail* currsch)
{
    Json::ScheduleDetail* nextSch = NULL;
    std::string nexttime = timePoint;
    if (nexttime.length() == 0)
    {
        nexttime = SystemClock::Today(SystemClockTMFormat);
    }

    //nextSch = FindSchedule(nexttime);
    if (NULL == nextSch)
    {
        std::map<int, Json::ScheduleDetail*>::iterator iter;
        for (iter = mScheduleForPlay.begin(); iter != mScheduleForPlay.end();
                ++iter)
        {
        	std::string nextHMS = nexttime.substr(9,6);
            if (iter->second != NULL
                    && nextHMS < iter->second->mScheduleBasic.mStartTime.substr(9,6))
            {
            	if(currsch != NULL)
            	{
            		if(nextSch == NULL
							&& currsch->mScheduleBasic.mEndTime.substr(9,6) >
							iter->second->mScheduleBasic.mStartTime.substr(9,6))
					{
						if(iter->second->mScheduleBasic.mPriority >
								currsch->mScheduleBasic.mPriority)
						{
							nextSch = iter->second;
							continue;
						}
						else if(iter->second->mScheduleBasic.mPriority ==
								currsch->mScheduleBasic.mPriority
								&& iter->second->mScheduleBasic.mServerLevel >=
								currsch->mScheduleBasic.mServerLevel)
						{
							nextSch = iter->second;
							continue;
						}
					}
					else if(nextSch == NULL
							&& currsch->mScheduleBasic.mEndTime.substr(9,6) <=
							iter->second->mScheduleBasic.mStartTime.substr(9,6))
					{
						nextSch = iter->second;
						continue;
					}
            	}
            	else
            	{
            		if(nextSch == NULL)
            		{
            			nextSch = iter->second;
            			continue;
            		}
            	}

                if (nextSch != NULL)
                {
                    // 1.judge the priority .
                    if (iter->second->mScheduleBasic.mPriority
                            > nextSch->mScheduleBasic.mPriority)
                    {
                        nextSch = iter->second;
                    } else if (iter->second->mScheduleBasic.mPriority
                            == nextSch->mScheduleBasic.mPriority)
                    {
                        //0-pcc > 1-occ > 2 station
                        if (iter->second->mScheduleBasic.mServerLevel
                                < nextSch->mScheduleBasic.mServerLevel)
                        {
                            nextSch = iter->second;
                        }
                        if (iter->second->mScheduleBasic.mServerLevel
                                == nextSch->mScheduleBasic.mServerLevel)
                        {
                            // 2. priority is same,judge the publish time.
                            if (iter->second->mScheduleBasic.mStartTime.substr(9,6)
                                    < nextSch->mScheduleBasic.mStartTime.substr(9,6))
                            {
                                nextSch = iter->second;
                            }
                        }
                    }
                }
            } // end of if (starttime endtime judge)
        } //end of for(mScheduleForPlay)
    }

    return nextSch;
}

/**
 * Find LayoutGroupDetail according to LayoutGroup strategy and given time.
 * @param timePoint[in]: the time to judge.
 * @return
 */
Json::SchLayoutGroupBasic* ScheduleHandler::FindLayoutGroup(
        const std::string& timePoint)
{
    Json::SchLayoutGroupBasic* retGroup = NULL;

    //search the current layout group ready to play.
    std::vector<Json::SchLayoutGroupBasic>::iterator iter =
            mCurrentScheduleForPlay->mLayoutGroups.begin();

    for (; iter != mCurrentScheduleForPlay->mLayoutGroups.end(); ++iter)
    {
        if (timePoint >= iter->mStartTime && timePoint < iter->mEndTime)
        {
            if (retGroup == NULL)
            {
                retGroup = &(*iter);
            } else
            {
                if (iter->mUpdatedTime > retGroup->mUpdatedTime)
                {
                    retGroup = &(*iter);
                }
            }

        }
    }    // end of for(mCurrentScheduleForPlay->mLayoutGroups)

    return retGroup;
}

Json::SchLayoutGroupBasic* ScheduleHandler::FindNextLayoutGroup(
        const std::string& timePoint)
{
    Json::SchLayoutGroupBasic* nextGroup = NULL;
    std::string nexttime = timePoint;
    if (nexttime.length() == 0)
    {
        nexttime = SystemClock::Today(SystemClockTMFormat);
    }

    //nextGroup = FindLayoutGroup(nexttime);
    if (NULL == nextGroup)
    {
        std::vector<Json::SchLayoutGroupBasic>::iterator iter =
                mCurrentScheduleForPlay->mLayoutGroups.begin();
        for (; iter != mCurrentScheduleForPlay->mLayoutGroups.end(); ++iter)
        {
            if (nexttime <= iter->mStartTime)
            {
                if (nextGroup == NULL)
                {
                    nextGroup = &(*iter);
                } else
                {
                    if (iter->mStartTime < nextGroup->mStartTime)
                    {
                        nextGroup = &(*iter);
                    }
                }

            }
        }    // end of for(mCurrentScheduleForPlay->mLayoutGroups)
    }
    return nextGroup;
}
bool ScheduleHandler::FindCurrentLayoutRelatedMedia(
        Json::LayoutInfo4Qt* layoutAllInfo)
{
    if (NULL == mCurrentLayoutForPlay || NULL == layoutAllInfo)
        return false;

    if (NULL == mCurrentLayouGroupForPlay)
        return false;

    layoutAllInfo->layoutDtl = *mCurrentLayoutForPlay;

    std::vector<Json::PartitionDetail>::const_iterator itor =
            mCurrentLayoutForPlay->mPartitions.begin();

    for (; itor != mCurrentLayoutForPlay->mPartitions.end(); ++itor)
    {
        int partitionId = itor->mId; // the partition id in layout group.

        // according partition id to get partition detail in 'schedule detail'
        std::vector<Json::PartitionMedias>::const_iterator itorPtM =
                mCurrentLayouGroupForPlay->mPartionMedias.begin();

        for (; itorPtM != mCurrentLayouGroupForPlay->mPartionMedias.end();
                ++itorPtM)
        {
            if (itorPtM->mPartitionId == partitionId)
            { // found the partition.
                Json::LayoutInfo4Qt::MediaContents theContentsMap;
                const std::vector<int>& mediaIds = itorPtM->mMediaIds;

                FindMediaInfos(mediaIds, theContentsMap);

                // insert the partition map.
                layoutAllInfo->mPartitonInfos.insert(
                        std::make_pair(partitionId, theContentsMap));
            }
        }    // end of for(mCurrentLayouGroupForPlay->mPartionMedias)
    }    // end of for(mCurrentLayoutForPlay->mPartitions)

    return true;
}

void ScheduleHandler::FindMediaInfos(const std::vector<int>& mediaIds,
        Json::LayoutInfo4Qt::MediaContents& medias)
{
    std::map<int, Json::MediaBasic*>::const_iterator itor =
            mContentListForPlay.begin();

    int mediaId = 0;
    for (size_t i = 0; i < mediaIds.size(); ++i)
    {
        mediaId = mediaIds[i];
        itor = mContentListForPlay.find(mediaId);
        if (itor != mContentListForPlay.end())
        {    // found the media
            medias[mediaId] = itor->second;
        }
    }
}
