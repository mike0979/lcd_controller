/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file name : comment_example.h
 * @author : alex
 * @date : 2017/7/14 14:29
 * @brief : open a file
 */
#ifndef SRC_DATATRANS_H_
#define SRC_DATATRANS_H_
#include "Mutex.h"
#include "Handler.h"
#include "SyncerPipe.h"

#include <string>
#include <list>
#include "Thread.h"

#include "HttpJsonTrans.h"

class Md5Checkers: public Thread, public Handler
{
public:
    class Md5CheckersMsg
    {
    public:
        std::string mFilePath;
        std::string mMd5;

        void *mData;
    };

    enum Md5CheckersType
    {
        MD5Check = 0,
    };

private:
    virtual void run();
    virtual bool handleMessage(Message *msg);
};

class DataTrans
{
public:
    enum TransMsg
    {
        TransMsgFirst = 0,

        TransUpload = TransMsgFirst, TransDownload,

        TransMsgLast = TransDownload,
    };

    enum UploadStatus
    {                 // WARNING: do NOT change its order
        UploadSuccess = 0, UploadSkipped, // some files were skipped during the upload (due to time-stamping or resume-rules)
        UploadRemoteError, // some files failed to be transmitted due to an remote error
        UploadSkipAndFailError, // some files failed and some others were skipped
        UploadGenericError,
        UploadFileAccessError,         // local file does not exists or readable
        UploadCmdError,
        UploadInterrupt,
    };

    enum DownloadStatus
    {
        DownloadSuccess = 0,
        DownloadGenericError,
        DownloadParseError,
        DownloadFileIOError,
        DownloadNetworkFailure,
        DownloadSSLFailure,
        DownloadAuthenticationFailure,
        DownloadProtocolError,
        DownloadRemoteError,
        DownloadCmdError,
        DownloadInterrupt,
        DownloadMD5Mismatch,
        WaittingMD5Checksum,
    };

    union TransStatus
    {
        UploadStatus mUploadStatus;
        DownloadStatus mDownloadStatus;
    };

    enum DataTransType
    {
        DataTransType_String,
		DataTransType_String_PUT,
		DataTransType_String_POST,
		DataTransType_String_POSTWithRply, // post data and got reply.
        DataTransType_Resource,
        DataTransType_Unknow,
    };

    class TransWork
    {
    public:
        TransWork(const std::string &subpath, const std::string &localdir,
                const std::string &localfile,
                const DataTrans::DataTransType transtype, const int dltype,
                const std::string& content = "", const std::string &md5 = "",
                const int flag = 0, Handler *handler = NULL);

        DataTrans *getDataTrans();
        int getTransCount();

    public:
        std::string mSubPath;
        std::string mLocalDir;
        std::string mLocalFile;
        DataTransType mTransType;
        int mTransFileType;
        std::string mMd5;
        int mFlag;
        std::string mJsonData;  // Json request data.
        double mTransSize;
        std::string mJsonRplyData; // Json reply data.

        int mTransCount;
    private:
        Handler *mHandler;
        DataTrans *mDataTrans;
        int mTransCnt;

        friend class DataTrans;
        friend class Md5Checkers;
        friend class UploadWorker;
        friend class DownloadWorker;
    };

public:
    DataTrans(const std::string ip, const int port, std::string usr = "",
            std::string pwd = "");
    ~DataTrans();

    int download(Handler *handler, const std::string &subpath,
            const DataTransType mTranstype, const int dltype,
            const std::string &localdir = "", const std::string &localfile = "",
            const std::string &md5 = "", const int flag = 0,
            const unsigned delay = 0);

    int upload(Handler *handler, const std::string &subpath,
            const DataTransType mTranstype, const int dltype,
            const std::string &upContent = "", const std::string &localdir = "",
            const std::string &localfile = "", const std::string &md5 = "",
            const int flag = 0, const unsigned delay = 0);

    void setToken(const std::string& token);


    int download(TransWork *work, unsigned delay);
    int upload(TransWork *work, unsigned delay);

private:
    int downloadInner(TransWork *work);
    int uploadInner(TransWork *work);
    DownloadStatus doDownload(TransWork *work, bool md5sync);
    UploadStatus doUpload(TransWork *work);

    static DownloadStatus downloadcheck(const std::string &filePath,
            const std::string &md5);

private:
    std::list<TransWork *> mUploadList;
    std::list<TransWork *> mDownloadList;

    SyncerPipe mUploadSyncer;
    SyncerPipe mDownloadSyncer;

    Thread mUploadThread;
    Thread mDownloadThread;

    Md5Checkers mMd5Checkers;

    Handler *mAsyncHandler;

    Mutex mDownloadListMutex;
    Mutex mUploadListMutex;

    curlwapper::HttpJsonTrans* mHttpJsonTrans;
    static const int TransFlagPriorityMask = 0x000000FF;

    friend class FtpTransAsyncHandler;
    friend class DownloadWorker;
    friend class UploadWorker;
    friend class Md5Checkers;
    static const char *TAG;
};

#endif /* SRC_DATATRANS_H_ */
