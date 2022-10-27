/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file : WeatherHandler.h
* @author : Benson
* @date : Sep 21, 2017
* @brief :
*/


#ifndef TRANSMANAGE_WEATHERHANDLER_H_
#define TRANSMANAGE_WEATHERHANDLER_H_

#include <transmanage/ITransHandler.h>

class WeatherHandler: public ITransHandler
{
public:
    WeatherHandler(TransManager* manager);
    virtual ~WeatherHandler();

    virtual void Execute(Message * msg,int flag=0);

    virtual std::string ModifyPath(const TransFileType dltype,
                const std::string& oriPath, void* param);

protected:
    virtual bool handleMessage(Message *msg);
};



#endif /* TRANSMANAGE_WEATHERHANDLER_H_ */
