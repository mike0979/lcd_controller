/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : LoginObjs.h
 * @author : Benson
 * @date : Oct 11, 2017
 * @brief :
 */

#ifndef JSON_LOGINOBJS_H_
#define JSON_LOGINOBJS_H_

#include <string>

namespace Json
{
class LoginReq
{
public:
    /**
     * Format the object to json string
     * @param obj[in]: the object to be format.
     * @param jsonStr[in,out]: the formated json string.
     */
    static std::string ToJson(const std::string& account,
            const std::string& pwd);
};

class LoginReply
{
public:
    LoginReply();
    ~LoginReply();

    /**
     * Format the object to json string
     * @param obj[in]: the object to be format.
     * @param jsonStr[in,out]: the formated json string.
     */
    static bool Parse(const char* jsonStr, LoginReply* data);

    std::string mToken;
    int mExpireTime; //(s) The expire time of token.
};



}
#endif /* JSON_LOGINOBJS_H_ */
