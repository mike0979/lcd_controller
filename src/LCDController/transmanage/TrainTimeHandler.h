/*
 * TrainTimeHandler.h
 *
 *  Created on: Apr 8, 2018
 *      Author: root
 */

#ifndef TRANSMANAGE_TRAINTIMEHANDLER_H_
#define TRANSMANAGE_TRAINTIMEHANDLER_H_

#include <transmanage/ITransHandler.h>
#include <CommonDef.h>
#include <DataTrans.h>

class TrainTimeHandler: public ITransHandler
{
public:
	TrainTimeHandler(TransManager* manager);
    virtual ~TrainTimeHandler();

    virtual void Execute(Message * msg,int flag=0);

    virtual std::string ModifyPath(const TransFileType dltype,
            const std::string& oriPath, void* param);

protected:
    virtual bool handleMessage(Message *msg);

private:
    void HandleTrainTimeReply(DataTrans::TransWork *work,
            DataTrans::DownloadStatus status);

    bool mFirstShowFromLocal;

    static const char *TAG;
};

#endif /* TRANSMANAGE_TRAINTIMEHANDLER_H_ */
