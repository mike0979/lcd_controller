/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file name : comment_example.h
 * @author : alex
 * @date : 2017/7/14 14:29
 * @brief : open a file
 */

#include "DataTrans.h"

#include <bsd/md5.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>

#include "Log.h"
#include "Looper.h"
#include "Message.h"
#include "Runnable.h"
#include "SystemClock.h"
#include "FileSysUtils.h"

#define SAFE_DELETE(obj)     do {          \
                if(NULL != obj)                       \
                {                                     \
                     delete (obj);                    \
                     obj = NULL;                      \
                }                                     \
            } while (0)



class FtpTransAsyncHandler: public Handler
{
public:
    FtpTransAsyncHandler(DataTrans *dataTrans);

private:
    virtual bool handleMessage(Message *msg);

private:
    static const char *TAG;

    DataTrans *mDataTrans;
};

FtpTransAsyncHandler::FtpTransAsyncHandler(DataTrans *dataTrans) :
        mDataTrans(dataTrans)
{

}

bool FtpTransAsyncHandler::handleMessage(Message *msg)
{
    DataTrans::TransWork *work = (DataTrans::TransWork *) msg->mData;

    switch (msg->mWhat)
    {
    case DataTrans::TransUpload:
        mDataTrans->uploadInner(work);
        break;

    case DataTrans::TransDownload:
        mDataTrans->downloadInner(work);

        break;
    default:
        //LogE("Unknown FtpTrans async message type : %d\n", msg->mWhat);
        return false;
    }

    return true;
}

class UploadWorker: public Runnable
{
public:
    UploadWorker(DataTrans *ftpTrans);

    virtual void run();

private:
    static const char *TAG;

    DataTrans *mdataTrans;
};

UploadWorker::UploadWorker(DataTrans *dataTrans) :
        mdataTrans(dataTrans)
{

}

void UploadWorker::run()
{
    char syncMsg;
    int rlen;

    while (true)
    {
        mdataTrans->mUploadSyncer.syncer();
        rlen = mdataTrans->mUploadSyncer.recvSyncerMsg(&syncMsg, 1);
        if (rlen == 1)
        {
            if (syncMsg == 'U')
            {
                DataTrans::TransWork *tw = NULL;

                synchronized (mdataTrans->mUploadListMutex){
                if (mdataTrans->mUploadList.empty() == false)
                {
                    tw = mdataTrans->mUploadList.front();
                    mdataTrans->mUploadList.pop_front();
                }
            }

                if (tw != NULL)
                {
                    mdataTrans->doUpload(tw);
                }
            }
        }
    }
}

class DownloadWorker: public Runnable
{
public:
    DownloadWorker(DataTrans *dataTrans);

    virtual void run();

private:
    static const char *TAG;

    DataTrans *mdataTrans;
};

DownloadWorker::DownloadWorker(DataTrans *dataTrans) :
        mdataTrans(dataTrans)
{

}

void DownloadWorker::run()
{
    char syncMsg;
    int rlen;

    while (true)
    {
        mdataTrans->mDownloadSyncer.syncer();
        rlen = mdataTrans->mDownloadSyncer.recvSyncerMsg(&syncMsg, 1);
        if (rlen == 1)
        {
            if (syncMsg == 'D')
            {
                DataTrans::TransWork *tw = NULL;

                synchronized (mdataTrans->mDownloadListMutex){
                if (mdataTrans->mDownloadList.empty() == false)
                {
                    tw = mdataTrans->mDownloadList.front();
                    mdataTrans->mDownloadList.pop_front();
                }
            }

                if (tw != NULL)
                {
                    mdataTrans->doDownload(tw, false);
                }
            }
        }
    }
}

void Md5Checkers::run()
{
    Looper *mlooper = Looper::CreateLooper();
    setLooper(mlooper);

    mlooper->loop();
}

bool Md5Checkers::handleMessage(Message *msg)
{
    Md5CheckersMsg *checkmsg = (Md5CheckersMsg *) (msg->mData);
    DataTrans::TransWork *work = (DataTrans::TransWork *) (checkmsg->mData);

    DataTrans::DownloadStatus done = DataTrans::downloadcheck(
            checkmsg->mFilePath, checkmsg->mMd5);

    if (work->mHandler != NULL)
    {
        Message *msg = new Message(work->mTransFileType, work, done);

        work->mHandler->sendMessage(msg);
    } else
    {
        SAFE_DELETE(work);
    }

    SAFE_DELETE(checkmsg);
    return true;
}

//-----------------------------------------------
// class DataTrans
//-----------------------------------------------
DataTrans::TransWork::TransWork(const std::string &subpath,
        const std::string &localdir, const std::string &localfile,
        const DataTrans::DataTransType transtype, const int dltype,
        const std::string& content, const std::string &md5, const int flag,
        Handler *handler)
{
    mSubPath = subpath;
    mLocalDir = localdir;
    mLocalFile = localfile;
    mTransType = transtype;
    mTransFileType = dltype;
    mHandler = handler;
    mMd5 = md5;
    mFlag = flag;

    mDataTrans = NULL;
    mTransCnt = 0;
    mJsonData = content;
    mTransSize = 0;

    mTransCount = 0;
}

DataTrans* DataTrans::TransWork::getDataTrans()
{
    return mDataTrans;
}

int DataTrans::TransWork::getTransCount()
{
    return mTransCnt;
}

DataTrans::DataTrans(const std::string ip, const int port,
        const std::string usr, const std::string pwd)
{

    mHttpJsonTrans = new curlwapper::HttpJsonTrans();
    mHttpJsonTrans->set_ipport(ip, port);
    mHttpJsonTrans->set_protocol("https://");

    mUploadThread.setRunnable(new UploadWorker(this));
    mDownloadThread.setRunnable(new DownloadWorker(this));

    mAsyncHandler = new FtpTransAsyncHandler(this);

    mMd5Checkers.start();
    mUploadThread.start();
    mDownloadThread.start();

}

DataTrans::~DataTrans()
{
	if(mHttpJsonTrans != NULL)
	{
		delete mHttpJsonTrans;
		mHttpJsonTrans = NULL;
	}

	if(mAsyncHandler != NULL)
	{
		delete mAsyncHandler;
		mAsyncHandler = NULL;
	}
}

int DataTrans::download(Handler *handler, const std::string &subpath,
        const DataTransType mTranstype, const int dltype,
        const std::string &localdir, const std::string &localfile,
        const std::string &md5, int flag, unsigned delay)
{
    TransWork *work = new TransWork(subpath, localdir, localfile, mTranstype,
            dltype, "", md5, flag, handler);

    return download(work, delay);
}

int DataTrans::upload(Handler *handler, const std::string &subpath,
        const DataTransType mTranstype, const int dltype,
        const std::string &upContent, const std::string &localdir,
        const std::string &localfile, const std::string &md5, const int flag,
        const unsigned delay)
{
    TransWork *work = new TransWork(subpath, localdir, localfile, mTranstype,
            dltype, upContent, md5, flag, handler);
    return upload(work, delay);
}

void DataTrans::setToken(const std::string& token)
{
    std::string header("token:");
    header.append(token);

    // append token header.
    mHttpJsonTrans->append_header(std::make_pair("token", token));
}

int DataTrans::download(TransWork *work, unsigned delay)
{
    if (delay == 0)
    {
        downloadInner(work);
    } else
    {
        Message *msg = new Message(TransDownload, work);
        mAsyncHandler->sendMessage(msg, delay);
    }

    return 0;
}

int DataTrans::upload(TransWork *work, unsigned delay)
{
    if (delay == 0)
    {
        uploadInner(work);
    } else
    {
        Message *msg = new Message(TransUpload, work);
        mAsyncHandler->sendMessage(msg, delay);
    }

    return 0;
}

int DataTrans::downloadInner(TransWork *work)
{
    int wp, ip;

    synchronized (mDownloadListMutex){
    bool added = false;

    wp = work->mFlag & TransFlagPriorityMask;
    for (std::list<TransWork *>::iterator i = mDownloadList.begin(); i != mDownloadList.end(); i++)
    {
        ip = (*i)->mFlag & TransFlagPriorityMask;
        if (ip > wp)
        {
            mDownloadList.insert(i, work);

            added = true;
            break;
        }
    }

    if (added == false)
    {
        mDownloadList.push_back(work);
    }
}
    mDownloadSyncer.sendSyncerMsg("D");

    return 0;
}

int DataTrans::uploadInner(TransWork *work)
{
    int wp, ip;

    synchronized (mUploadListMutex){
    bool added = false;

    wp = work->mFlag & TransFlagPriorityMask;
    for (std::list<TransWork *>::iterator i = mUploadList.begin(); i != mUploadList.end(); i++)
    {
        ip = (*i)->mFlag & TransFlagPriorityMask;
        if (ip > wp)
        {
            mUploadList.insert(i, work);

            added = true;
            break;
        }
    }

    if (added == false)
    {
        mUploadList.push_back(work);
    }
}
    mUploadSyncer.sendSyncerMsg("U");

    return 0;
}

DataTrans::DownloadStatus DataTrans::doDownload(TransWork *work, bool md5sync)
{
    DownloadStatus done = DownloadSuccess;
    work->mTransCount++;

    if (work->mTransType == DataTrans::DataTransType_String)
    {
    	 LogD("[download]########################################  %s\n",
         work->mSubPath.c_str());
         std::string dlcontent = "";
         double dlsize = 0;

         if (mHttpJsonTrans->download_jsondata(work->mSubPath, dlcontent,dlsize) && dlsize >= 2)
         {
        	 work->mJsonData = dlcontent;
        	 work->mTransSize = dlsize;
         }
         else
         {
        	 done = DownloadGenericError;
         }

#if 0
         //test cmd
        char buffer[4096];
		memset(buffer, 0, sizeof(buffer));
		int fd = 0;
		long size = 0;
		if (work->mTransFileType == 10/*cmd list*/)
		{

			fd =open("/home/workspace/GPIDS_Tongxing/code/LCDController/Debug/command/list.json",
							O_RDONLY);
			size = read(fd, buffer, sizeof(buffer));
			done = DownloadSuccess;

			close(fd);
			work->mJsonData = (std::string) buffer;
		}
		else if (work->mTransFileType == 11/*cmd list*/)
		{

			fd =open("/home/workspace/GPIDS_Tongxing/code/LCDController/Debug/command/detail.json",
							O_RDONLY);
			size = read(fd, buffer, sizeof(buffer));
			done = DownloadSuccess;

			close(fd);
			work->mJsonData = (std::string) buffer;
		}



       char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        int fd = 0;
        long size = 0;
        if (work->mTransFileType == 0/*DL_SchedulesUpdateList*/)
        {

//            fd =
//                    open(
//                            "/root/Desktop/Link to workspace/ground_pids_template/LCDController/Debug/down_jsons/schedule_list.json",
//                            O_RDONLY);
//            size = read(fd, buffer, sizeof(buffer));
            done = DownloadGenericError;
        } else if (work->mTransFileType == 1/*DL_Schedules*/)
        {
            fd =
                    open(
                            "/root/Desktop/Link to workspace/ground_pids_template/LCDController/Debug/down_jsons/schedule_detail.json",
                            O_RDONLY);
            size = read(fd, buffer, sizeof(buffer));
        } else if (work->mTransFileType == 2/*DL_LayoutGroups*/)
        {
            fd =
                    open(
                            "/root/Desktop/Link to workspace/ground_pids_template/LCDController/Debug/down_jsons/layout_group.json",
                            O_RDONLY);
            size = read(fd, buffer, sizeof(buffer));
        } else if (work->mTransFileType == 3/*DL_Medias*/)
        {
            fd =
                    open(
                            "/root/Desktop/Link to workspace/ground_pids_template/LCDController/Debug/down_jsons/media_detail.json",
                            O_RDONLY);
            size = read(fd, buffer, sizeof(buffer));
        } else if (work->mTransFileType == 4/*DL_MediaMD5*/)
        {
            fd =
                    open(
                            "/root/Desktop/Link to workspace/ground_pids_template/LCDController/Debug/down_jsons/media_MD5.json",
                            O_RDONLY);
            size = read(fd, buffer, sizeof(buffer));
        } else if (work->mTransFileType == 22/*DL_ArrivalInfo*/)
        {
            fd =
                    open(
                            "/root/Desktop/Link to workspace/ground_pids_template/LCDController/Debug/down_jsons/arrival_info.json",
                            O_RDONLY);
            size = read(fd, buffer, sizeof(buffer));
        }

        close(fd);
        work->mJsonData = (std::string) buffer;
		#endif

    } else if (work->mTransType == DataTrans::DataTransType_Resource)
    {
         double dlsize = 0;
         LogD("[download resource]########################################  %s , %s   %s\n",
        		 work->mSubPath.c_str(),work->mLocalDir.c_str(),
        		 work->mLocalFile.c_str());
         std::string filePath = work->mLocalDir + work->mLocalFile;
         if(FileSysUtils::Accessible(filePath, FileSysUtils::FR_OK))
         {
        	 done = WaittingMD5Checksum;
         }
         else
         {
             if (mHttpJsonTrans->download_resource(work->mSubPath, work->mLocalDir,
            		 work->mLocalFile, dlsize))
             {
    			 if (work->mMd5.empty() == false)
    			 {
    				 if (md5sync)
    				 {

    					 done = downloadcheck(filePath, work->mMd5);
    				 }
    				 else
    				 {
    					 done = WaittingMD5Checksum;
    				 }
    			 }
             }
         }

    }
    else
    {
        return DownloadGenericError;
    }

    if (done == WaittingMD5Checksum)
    {
        Md5Checkers::Md5CheckersMsg *checkmsg =
                new Md5Checkers::Md5CheckersMsg();

        checkmsg->mFilePath = work->mLocalDir + work->mLocalFile;
        checkmsg->mMd5 = work->mMd5;
        checkmsg->mData = work;

        mMd5Checkers.sendMessage(new Message(Md5Checkers::MD5Check, checkmsg));
    } else
    {
        if (work->mHandler != NULL && done == DownloadSuccess)
        {
            Message *msg = new Message(work->mTransFileType, work, done);
            work->mHandler->sendMessage(msg);
        } else
        {
            SAFE_DELETE(work);
        }
    }

    return done;
}

DataTrans::UploadStatus DataTrans::doUpload(TransWork *work)
{
    UploadStatus done = UploadSuccess;

    if (work->mTransType == DataTrans::DataTransType_String_PUT)
    {
    	LogD("[upload]########################################  %s\n",
                work->mSubPath.c_str());
        double upsize = 0;
        if (mHttpJsonTrans->upload_jsondata(
                curlwapper::HttpJsonTrans::Command_PUT, work->mSubPath,
                work->mJsonData, upsize))
        {
            done = UploadSuccess;
            work->mTransSize = upsize;
        } else
        { // upload failed.
            done = UploadRemoteError;
        }

        Message *msg = new Message(work->mTransFileType, work, done);
        work->mHandler->sendMessage(msg);

    }
    else if (work->mTransType == DataTrans::DataTransType_String_POST)
    {
    	LogD("[upload]########################################  %s\n",
                work->mSubPath.c_str());
        double upsize = 0;
        if (mHttpJsonTrans->upload_jsondata(
                curlwapper::HttpJsonTrans::Command_POST, work->mSubPath,
                work->mJsonData, upsize))
        {
            done = UploadSuccess;
            work->mTransSize = upsize;
        } else
        { // upload failed.
            done = UploadRemoteError;
        }

        Message *msg = new Message(work->mTransFileType, work, done);
        work->mHandler->sendMessage(msg);

    }
    else if (work->mTransType == DataTrans::DataTransType_String_POSTWithRply)
    {
    	LogD(
                "[upload-with reply]########################################  %s\n",
                work->mSubPath.c_str());
        double upsize = 0;
        std::string rplyBody("");

        if (mHttpJsonTrans->upload_jsondata(
                curlwapper::HttpJsonTrans::Command_POST, work->mSubPath,
                work->mJsonData, upsize, &rplyBody) && rplyBody.size() >= 2)/* new add rplyBody.size() > 0*/
        {
            work->mTransSize = upsize;
            // parse the reply
            work->mJsonRplyData = rplyBody;

            if (work->mHandler != NULL)
            {
                Message *msg = new Message(work->mTransFileType, work, done);
                work->mHandler->sendMessage(msg);
            } else
            {
                SAFE_DELETE(work);
            }
        } else
        {
            // upload failed.
            Message *msg = new Message(work->mTransFileType, work,
                    UploadRemoteError);
            work->mHandler->sendMessage(msg);
        }
    } else if (work->mTransType == DataTrans::DataTransType_Resource)
    {
        double dlsize = 0;
        if (mHttpJsonTrans->upload_resource(
                curlwapper::HttpJsonTrans::Command_POST, work->mSubPath,
                work->mLocalDir, work->mLocalFile, dlsize))
        {
            if (work->mHandler != NULL)
            {
                Message *msg = new Message(work->mTransFileType, work, done);
                work->mHandler->sendMessage(msg);
            } else
            {
                SAFE_DELETE(work);
            }
        } else
        {
            // upload failed.
            Message *msg = new Message(work->mTransFileType,work, UploadRemoteError);
            work->mHandler->sendMessage(msg);
        }
    }
    else
    {
    	LogD("[upload] unknown upload type.\n");
    }

    return done;
}

DataTrans::DownloadStatus DataTrans::downloadcheck(const std::string &filePath,
        const std::string &md5)
{
    LogI("Download %s done. Checking md5sum ...\n", filePath.c_str());

    DownloadStatus done = DownloadSuccess;
    char md5sum[MD5_DIGEST_STRING_LENGTH];

    int checkcnt = 0;
    do
    {
        char *sum = MD5File(filePath.c_str(), md5sum);
        if (sum != NULL)
        {
            if (md5.compare(sum) == 0)
            {
                LogD("Download file success : %s\n", filePath.c_str());
            } else
            {
                LogE("MD5 checksum mismatch : %s , %s,  %s\n", filePath.c_str(),md5.c_str(),sum);

                // delete the file
                std::string cmd = "rm -f " + filePath;
                system(cmd.c_str());
                done = DownloadMD5Mismatch;
            }
        } else
        {
            if (++checkcnt < 2)
            {
                SystemClock::Sleep(300);

                continue;
            } else
            {
                LogE(
                        "Failed to calculate MD5 checksum : %s [file cannot be opened]\n",
                        filePath.c_str());
                done = DownloadGenericError;
            }
        }
    } while (false);

    return done;
}

const char *DataTrans::TAG = "DataTrans";
