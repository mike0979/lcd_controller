/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file name : TransManager.h
 * @author :
 * @date :
 * @brief :
 */
#ifndef TRANSMANAGE_TRANSMANAGER_H_
#define TRANSMANAGE_TRANSMANAGER_H_

#include <CommonDef.h>
#include <transmanage/ITransHandler.h>
#include "DataTrans.h"
#include "Handler.h"
#include "Message.h"
#include "Thread.h"
#include "config/configparser.h"
#include "LCDController.h"

class TransManager: public Thread, public Handler
{

public:

    TransManager(LCDController* controller);
    ~TransManager();

    /**
     * Send request to server.
     * @param transHandler
     * @param dltype
     * @param param
     */
    void sendRequest(ITransHandler* transHandler, const TransFileType dltype,
            void* param);

    /**
     * Download file from server.
     * @param transHandler
     * @param dltype
     * @param fileNewName: The local name of download file.
     * @param md5: The md5 of file to download.
     * @param param
     * @param delay
     */
    void downloadFile(ITransHandler* transHandler, const TransFileType dltype,
            const std::string &fileNewName,const std::string& md5="", void* param=NULL);

    /**
     * Download file from server.
     * @param work
     */
    void downloadTransWork(DataTrans::TransWork *work);

    /**
     * Send reply to server.
     * @param transHandler
     * @param mTranstype
     * @param dltype
     * @param upData
     * @param param
     */
    void sendReply(ITransHandler* transHandler,
            const DataTrans::DataTransType mTranstype,
            const TransFileType dltype, const std::string& upData, void* param);

    /**
     * Upload file to server.
     */
    void UploadFile();

    // Get the device id of LCD Controller.
    std::string GetCtrlerDevId();

    const ConfigParser* GetConfig() const;

    const LCDLEDControllerFlag GetControllerFlag() const;

    LCDController* getLCDController();
private:
    virtual void run();

    virtual bool handleMessage(Message *msg);

    // Send log in request
    void handleLogInReq();

    // Handle the reply of log in.
    void handleLogInReply(DataTrans::TransWork *work);

    // Send refresh token request.
    void handleRfshTokenReq();

    // Handle the reply of refresh token.
    void handleRfshTokenReply(DataTrans::TransWork *work);

    // Send report status request.
    void handleReportStatusReq();

    // report local device status.
    void reportLocalDevStatus();

    // report sub-device status.
    void reportSubDevStatus();

private:
    LCDController* mLCDController;
    DataTrans* mDataTrans;

    std::string mToken; // https token
    int mExpireTime; //(s) expire time of token.

    bool mBLogStatus;
    static const char *TAG;
};

#endif /* TRANSMANAGE_TRANSMANAGER_H_ */
