/*
 * LiveSwitchHandler.h
 *
 *  Created on: Nov 12, 2018
 *      Author: root
 */

#ifndef TRANSMANAGE_LIVESWITCHHANDLER_H_
#define TRANSMANAGE_LIVESWITCHHANDLER_H_


#include <transmanage/ITransHandler.h>
#include <CommonDef.h>
#include <DataTrans.h>

class LiveSwitchHandler: public ITransHandler
{
public:
	LiveSwitchHandler(TransManager* manager);
    virtual ~LiveSwitchHandler();

    virtual void Execute(Message * msg,int flag=0);

    virtual std::string ModifyPath(const TransFileType dltype,
            const std::string& oriPath, void* param);

protected:
    virtual bool handleMessage(Message *msg);

private:
    void HandleLiveSwitchReply(DataTrans::TransWork *work,
            DataTrans::DownloadStatus status);

    LiveSourceSwitchConfig mLiveSourceConfig;

    static const char *TAG;
};




#endif /* TRANSMANAGE_LIVESWITCHHANDLER_H_ */
