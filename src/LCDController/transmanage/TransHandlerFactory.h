/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : TransHandlerFactory.h
 * @author : Benson
 * @date : Sep 20, 2017
 * @brief :
 */

#ifndef TRANSMANAGE_TRANSHANDLERFACOTRY_H
#define TRANSMANAGE_TRANSHANDLERFACOTRY_H

#include <CommonDef.h>
#include <Mutex.h>
#include <transmanage/ITransHandler.h>
#include <map>

class TransManager;

class TransHandlerFactory
{
public:
    static TransHandlerFactory* Instance(TransManager* manager);

    static void Destory();
    /**
     * Return related DownLoader according to notification code.
     * @param code[in]: the notification code.
     * @return The address of created DownLoader! Don't try to free outside!
     */
    ITransHandler* GetLoader(const NotifyMessageCode code);

private:
    TransHandlerFactory(TransManager* manager);
    ~TransHandlerFactory();

    TransManager* mDldManager;
    static Mutex mMutex;
    static TransHandlerFactory* mInstance;
    static std::map<NotifyMessageCode,ITransHandler*> mDownLoaders;
};

#endif /* TRANSMANAGE_TRANSHANDLERFACOTRY_H */
