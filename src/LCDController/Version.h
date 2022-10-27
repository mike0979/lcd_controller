/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file : Version.h 
* @author : Benson
* @date : Nov 13, 2017
* @brief :
*/


#ifndef VERSION_H_
#define VERSION_H_

#include <string>

class Version
{
public:
    static const std::string& GetVersion();
private:
    static std::string mVersion;
};




#endif /* VERSION_H_ */
