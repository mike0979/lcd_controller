/*
 * ScreenOnOffHandler.h
 *
 *  Created on: Sep 27, 2018
 *      Author: root
 */

#ifndef TRANSMANAGE_SCREENONOFFHANDLER_H_
#define TRANSMANAGE_SCREENONOFFHANDLER_H_

#include <transmanage/ITransHandler.h>
#include <CommonDef.h>
#include <DataTrans.h>

class ScreenOnOffHandler: public ITransHandler
{
public:
	ScreenOnOffHandler(TransManager* manager);
    virtual ~ScreenOnOffHandler();

    virtual void Execute(Message * msg,int flag=0);

    virtual std::string ModifyPath(const TransFileType dltype,
            const std::string& oriPath, void* param);

protected:
    virtual bool handleMessage(Message *msg);

private:
    void HandleScreenOnOffReply(DataTrans::TransWork *work,
            DataTrans::DownloadStatus status);

    static const char *TAG;
};



#endif /* TRANSMANAGE_SCREENONOFFHANDLER_H_ */
