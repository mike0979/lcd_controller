/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : ITransHandler.h
 * @author : Benson
 * @date : Sep 21, 2017
 * @brief : Interface of downloader.
 */

#ifndef TRANSMANAGE_ITRANSHANDLER_H_
#define TRANSMANAGE_ITRANSHANDLER_H_

#include <CommonDef.h>
#include <Handler.h>
#include <string>

class TransManager;

class ITransHandler: public Handler
{
public:
    ITransHandler(TransManager* manager);

    virtual ~ITransHandler();

    virtual void Execute(Message * msg,int flag) = 0;

    /**
     * Different IDownLoader class may modify it's url with given param.
     * @param dltype[in]: The type of download file.
     * @param oriPath[in]: The original url.
     * @param param[in]: The param used to modify url.It could be 'NULL'.
     * @return The modified url.
     */
    virtual std::string ModifyPath(const TransFileType dltype,
            const std::string& oriPath, void* param = NULL);

    /**
     * Get the translate manager.
     * @return
     */
    const TransManager* GetManager();

protected:
    TransManager* mTransManager;
};

#endif /* TRANSMANAGE_ITRANSHANDLER_H_ */
