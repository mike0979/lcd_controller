/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : Command.h
 * @author : Benson
 * @date : Sep 19, 2017
 * @brief :
 */
#ifndef COMMAND_H_
#define COMMAND_H_

#include "Handler.h"
#include "json/CommandObjs.h"
#include "CommonDef.h"
#include "TermiosU.h"
#include <list>
#include <map>
#include "LCDPlayer/LCDPlayer.h"

class Command;
class ConfigParser;
class CmdHandler;

#define LCD_DATA_LENTH   64

class CmdFactory
{
public:
    /**
     * Generate a certain command according to command type.
     * @param cmdType: Command
     * @return
     * Don't forget to delete the generated command!!!!
     */
    static Command* GenerateCmd(const CommandMsgType& cmdType,const ConfigParser* cfg, CmdHandler* cmdHandler);
private:
    static const char *TAG;
};

class Command : public Handler
{
public:

	enum CommandExecResult{
		RES_ReadyToExec,
		RES_ExecSuccess,
		RES_ExecFailed,
	};

	enum SerialDeviceOpMsg{
		CMD_LCDPanel = 150,
		CMD_LEDPanel ,
		CMD_LCDReply ,
		CMD_LEDReply ,
		CMD_MainDevReply ,
		CMD_ShutDownDelay ,
		CMD_MutexAndRecover,
	} ;

    Command(CmdHandler* cmdHandler,const ConfigParser* cfg);
    virtual ~Command();

    virtual void SetParam(const Json::CmdBasic* cmdInfo);

    /**
     * Execute the command.
     * This function will keep blocking until the command executed or an error occur.
     * @return The execute result.
     *         0 - Not executed.
     *         1 - Execute succeed.
     *         2 - Execute failed.
     */
    virtual int Execute() = 0;

    virtual void GenerateReply(int status,const std::string& devId,std::string& rplyStr);

    /**
     * Get command name
     */
    std::string GetCmdName();

    /**
     * Get the command id.
     */
    int GetCmdId();

    /**
     * Get command execute result.
     */

    bool setSuitableDev(std::vector<std::string>& targets, int LCD_LED_flag);

    int GetExeRslt();

    //return true: all device targets had executed.
    bool setCommandExecStatus(std::string sdid, CommandExecResult res = RES_ExecSuccess);

    bool replyExecResult(std::string strjson, std::string sdid);

    void setLCDSerialObj(LCDPlayer* lcdplayer);

protected:
    bool handleMessage(Message *msg);

    virtual int OpenDev(Termios& dev,std::string sdid = "");

    int sendLCDCommand(Handler* handler,void* data, int len, std::string sdid);

    int checkCmdRunStatus(void * data,int len);

    std::list<std::string> mSuitableDeviceList;

    CmdHandler* mCmdHandler;
    const ConfigParser* mCfg;
    std::vector<std::string> mParams;
    std::string mCmdName;
    int mCmdId;
    int mExeRslt; // command execute result!

    unsigned char mSerialReadBuf[LCD_DATA_LENTH];
    int mSeralReadLen;

    std::map<std::string,CommandExecResult> mAvailableLEDCmdMap; //string - deviceid, int - run result.
    std::map<std::string,CommandExecResult> mAvailableLCDCmdMap;
    std::map<std::string,CommandExecResult> mAvailableMainDeviceCmdMap;

    friend class LEDPlayer;

    LCDPlayer* mLCDPlayer;

    static const char *TAG;
};

class Brightness: public Command
{
public:
	Brightness(CmdHandler* cmdHandler,const ConfigParser* cfg);
    ~Brightness();

    virtual void SetParam(const Json::CmdBasic* cmdInfo);

    /**
     * Execute the command.
     * This function will keep blocking until the command executed or an error occur.
     * @return The execute result.
     *         0 - Not executed.
     *         1 - Execute succeed.
     *         2 - Execute failed.
     */
    virtual int Execute();

    int GetBrightness();
private:
    int mBrightness;
    static const char *TAG;
};

class Volume: public Command
{
public:
	Volume(CmdHandler* cmdHandler,const ConfigParser* cfg);
    ~Volume();

    virtual void SetParam(const Json::CmdBasic* cmdInfo);

    /**
     * Execute the command.
     * This function will keep blocking until the command executed or an error occur.
     * @return The execute result.
     *         0 - Not executed.
     *         1 - Execute succeed.
     *         2 - Execute failed.
     */
    virtual int Execute();

private:
    int mVolumnVal;
    DevType mDevType;
    int mDevCode;
    static const char *TAG;
};

class VolumeMute: public Command
{
public:
	VolumeMute(CmdHandler* cmdHandler,const ConfigParser* cfg);
    ~VolumeMute();

    virtual void SetParam(const Json::CmdBasic* cmdInfo);

    /**
     * Execute the command.
     * This function will keep blocking until the command executed or an error occur.
     * @return The execute result.
     *         0 - Not executed.
     *         1 - Execute succeed.
     *         2 - Execute failed.
     */
    virtual int Execute();

private:
    bool mIsMute;
    DevType mDevType;
    int mDevCode;
    static const char *TAG;
};

class StartUp: public Command
{
public:
    StartUp(CmdHandler* cmdHandler,const ConfigParser* cfg);
    ~StartUp();

    virtual void SetParam(const Json::CmdBasic* cmdInfo);

    /**
     * Execute the command.
     * This function will keep blocking until the command executed or an error occur.
     * @return The execute result.
     *         0 - Not executed.
     *         1 - Execute succeed.
     *         2 - Execute failed.
     */
    virtual int Execute();

private:
    int mPAStatus;  //102-pa cancal, 0 - common command
    DevType mDevType;
    int mDevCode;
    static const char *TAG;
};

class ShutDown: public Command
{
public:
    ShutDown(CmdHandler* cmdHandler,const ConfigParser* cfg);
    ~ShutDown();

    virtual void SetParam(const Json::CmdBasic* cmdInfo);

    /**
     * Execute the command.
     * This function will keep blocking until the command executed or an error occur.
     * @return The execute result.
     *         0 - Not executed.
     *         1 - Execute succeed.
     *         2 - Execute failed.
     */
    virtual int Execute();

private:
    int mPAStatus;  //101-pa receive, 0 - common command
    DevType mDevType;
    int mDevCode;
    static const char *TAG;
};

class Reboot: public Command
{
public:
    Reboot(CmdHandler* cmdHandler,const ConfigParser* cfg);
    ~Reboot();

    virtual void SetParam(const Json::CmdBasic* cmdInfo);

    /**
     * Execute the command.
     * This function will keep blocking until the command executed or an error occur.
     * @return The execute result.
     *         0 - Not executed.
     *         1 - Execute succeed.
     *         2 - Execute failed.
     */
    virtual int Execute();
private:
    DevType mDevType;
    int mDevCode;
};

class GetLog: public Command
{
public:
    GetLog(CmdHandler* cmdHandler,const ConfigParser* cfg);
    ~GetLog();

    virtual void SetParam(const Json::CmdBasic* cmdInfo);

    /**
     * Execute the command.
     * This function will keep blocking until the command executed or an error occur.
     * @return The execute result.
     *         0 - Not executed.
     *         1 - Execute succeed.
     *         2 - Execute failed.
     */
    virtual int Execute();
};

class CmdHandler;
class GetSnapshot: public Command
{
public:
    GetSnapshot(CmdHandler* cmdHandler,const ConfigParser* cfg);
    ~GetSnapshot();

    virtual void SetParam(const Json::CmdBasic* cmdInfo);

    /**
     * Execute the command.
     * This function will keep blocking until the command executed or an error occur.
     * @return The execute result.
     *         0 - Not executed.
     *         1 - Execute succeed.
     *         2 - Execute failed.
     */
    virtual int Execute();

private:
    virtual bool handleMessage(Message *msg);

};

class HouseKeeping: public Command
{
public:
    HouseKeeping(CmdHandler* cmdHandler,const ConfigParser* cfg);
    ~HouseKeeping();

    virtual void SetParam(const Json::CmdBasic* cmdInfo);

    /**
     * Execute the command.
     * This function will keep blocking until the command executed or an error occur.
     * @return The execute result.
     *         0 - Not executed.
     *         1 - Execute succeed.
     *         2 - Execute failed.
     */
    virtual int Execute();
};

#endif /* COMMAND_H_ */
