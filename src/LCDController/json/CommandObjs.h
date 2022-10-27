/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : CommandObjs.h
 * @author : Benson
 * @date : Sep 18, 2017
 * @brief :
 */

#include <string>
#include <vector>

#ifndef JSON_COMMANDOBJS_H_
#define JSON_COMMANDOBJS_H_

namespace Json
{

class CmdBasic
{
public:
    CmdBasic();
    ~CmdBasic();

    int mId;
    std::string mDscrp;
    std::string mCmd;
    std::string mCmdParm;
    int mStatus; // Should be Ignored in LCD-Player
    std::string mStartTime;
    std::string mUpdateTime;
};

class CmdList
{
public:
    CmdList();
    ~CmdList();

    static bool Parse(const char* jsonStr, CmdList* data);

    std::vector<CmdBasic> mCmds;
};

class SpecificObj
{
public:
	std::string mMaster;
	std::vector<std::string> mTargets; // the specific object.
};

class CmdDetail
{
public:
    CmdDetail();
    ~CmdDetail();

    static bool Parse(const char* jsonStr, CmdDetail* data);

    CmdBasic mBasic;
    SpecificObj mSpecObj;
};


class CmdExeReply
{
public:
    CmdExeReply();
    CmdExeReply(const CmdBasic& bsc );

    ~CmdExeReply();

    /**
     * Format the object to json string
     * @param obj[in]: the object to be format.
     * @param jsonStr[in,out]: the formated json string.
     */
    static void ToJson(const CmdExeReply* obj,std::string& jsonStr);

    int mId; // command id

    std::string mDevice;
    int mStatus;
    std::string mRepTime; // Reply time
};

}

#endif /* JSON_COMMANDOBJS_H_ */
