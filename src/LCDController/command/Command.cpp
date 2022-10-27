/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : Command.cpp
 * @author : Benson
 * @date : Sep 19, 2017
 * @brief :
 */

#include <command/Command.h>
#include <transmanage/TransManager.h>
#include <transmanage/CmdHandler.h>
#include <config/configparser.h>
#include <FileSysUtils.h>
#include <Log.h>
#include <SystemClock.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <errno.h>

Command::Command(CmdHandler* cmdHandler,const ConfigParser* cfg) :
		mCmdHandler(cmdHandler),mCfg(cfg), mCmdName(""), mCmdId(-1), mExeRslt(0),mSeralReadLen(0)
{
	mAvailableMainDeviceCmdMap.clear();
	mAvailableLEDCmdMap.clear();
	mAvailableLCDCmdMap.clear();

	mLCDPlayer = NULL;
}

Command::~Command()
{
}

void Command::SetParam(const Json::CmdBasic* cmdInfo)
{
    if ( NULL == cmdInfo)
        return;

    mCmdId = cmdInfo->mId;
    mCmdName = cmdInfo->mCmd;

    std::istringstream parmStrm(cmdInfo->mCmdParm);
    std::string param;

    while (parmStrm >> param)
        mParams.push_back(param);

    return;
}

bool Command::handleMessage(Message *msg)
{
	switch(msg->mWhat)
	{
	case CMD_MutexAndRecover:
	{
		if(msg->mArg1 == 1)
		{
			std::string scmd = "amixer sset Master mute";
			system(scmd.c_str());
		}
		else
		{
			std::string scmd = "amixer sset Master unmute";
			system(scmd.c_str());
		}

		mSeralReadLen = 0;
		Message* msg1 = new Message(CMD_LCDReply,(void*)mSerialReadBuf,mSeralReadLen);
		msg1->mStr = msg->mStr;
		sendMessage(msg1);

		break;
	}
	case CMD_LCDPanel:
	{
//		unsigned char *databuf = (unsigned char *)msg->mData;
//		for(int i=0;i<msg->mArg1;++i)
//		{
//			LogD("#######  %s  - %02x\n",msg->mStr.c_str(),databuf[i]);
//		}

		//synchronized(g_MutexForLCDSerialAccess)
		//{
		if(mLCDPlayer != NULL)
		{
			LogD("send LCD serial command\n");

			mLCDPlayer->sendMessage(new Message(LCDPlayer::LCD_WriteSerialPort, msg->mData, msg->mArg1, msg->mArg2));
			//sendLCDCommand(this,msg->mData,msg->mArg1,msg->mStr);
		}
		else
		{
			LogE("mLCDPlayer == NULL\n");
		}
			mSeralReadLen = 0;
			Message* msg1 = new Message(CMD_LCDReply,(void*)mSerialReadBuf,mSeralReadLen);
			msg1->mStr = msg->mStr;
			sendMessage(msg1);
		//}

		break;
	}
	case CMD_LEDPanel:
	{
		mCmdHandler->sendMessage(new Message(MSG_CMDToLED,mCmdId));
		break;
	}
	case CMD_LCDReply:
	{
		if(mCmdHandler != NULL)
		{
			//for LCDpanel don't return result
			//CommandExecResult resReply = (CommandExecResult)checkCmdRunStatus(msg->mData,msg->mArg1);
			CommandExecResult resReply = RES_ExecSuccess;
			LogD("#######  Recived reply data  %d\n",resReply);
			std::string jsonRply;
			GenerateReply(resReply, mCfg->mDeviceId, jsonRply);

			Message* msg1 = new Message(MSG_CmdResult,mCmdId);
			msg1->mStr = jsonRply;
			mCmdHandler->sendMessage(msg1);

			//command had exec finished
			if(setCommandExecStatus(msg->mStr,resReply))
			{
				Message* msg2 = new Message(MSG_CmdCompleted,this,mCmdId);
				mCmdHandler->sendMessage(msg2);
			}
		}
		break;
	}
	case CMD_LEDReply:
	{
		for(std::map<std::string,CommandExecResult>::iterator iter = mAvailableLEDCmdMap.begin();	iter != mAvailableLEDCmdMap.end();++iter)
		{
			std::string jsonRply = "";
			if(iter->second == RES_ExecSuccess)
			{
				GenerateReply(1, mCfg->mDeviceId, jsonRply);
			}
			else if(iter->second == RES_ExecFailed)
			{
				GenerateReply(2, mCfg->mDeviceId, jsonRply);
			}

			if(jsonRply.size() > 0)
			{
				Message* msg1 = new Message(MSG_CmdResult,mCmdId);
				msg1->mStr = jsonRply;
				mCmdHandler->sendMessage(msg1);
			}
			else
			{
				LogD("Generage cmd reply data failed.\n");
			}
		}

		//command had exec finished
		if(setCommandExecStatus(""))
		{
			Message* msg2 = new Message(MSG_CmdCompleted,this,mCmdId);
			mCmdHandler->sendMessage(msg2);
		}

		break;
	}
	case CMD_MainDevReply:
	{
		std::string jsonRply = "";
		GenerateReply(1, mCfg->mDeviceId, jsonRply);
		Message* msg1 = new Message(MSG_CmdResult,mCmdId);
		msg1->mStr = jsonRply;
		LogD("Cmd result ready to reply: %s\n",msg1->mStr.c_str());
		mCmdHandler->sendMessage(msg1);

		if(msg->mArg1 == (int)MSG_CMD_ShutDown)
		{
			//delay 3 seconds to shutdown
			sendMessage(new Message(CMD_ShutDownDelay,(int)MSG_CMD_ShutDown),3000);
		}
		else if(msg->mArg1 == (int)MSG_CMD_ReBoot)
		{
			//delay 3 seconds to shutdown
			sendMessage(new Message(CMD_ShutDownDelay,(int)MSG_CMD_ReBoot),3000);
		}


		break;
	}
	case CMD_ShutDownDelay:
	{
		LogD("--------------- System start to reboot or shutdown ---------------\n");
		if(msg->mArg1 == (int)MSG_CMD_ShutDown)
		{
			system("shutdown -h now");
		}
		else if(msg->mArg1 == (int)MSG_CMD_ReBoot)
		{
			system("reboot -f");
		}

		break;
	}
	}

	return true;
}

bool  Command::replyExecResult(std::string strjson, std::string sdid)
{
	Message* msg1 = new Message(MSG_CmdResult,mCmdId);
	msg1->mStr = strjson;
	mCmdHandler->sendMessage(msg1);

	//command had exec finished
	if(setCommandExecStatus(sdid))
	{
		Message* msg2 = new Message(MSG_CmdCompleted,this,mCmdId);
		mCmdHandler->sendMessage(msg2);
	}

	return true;
}

void Command::setLCDSerialObj(LCDPlayer* lcdplayer)
{
	if(lcdplayer != NULL)
	{
		mLCDPlayer = lcdplayer;
	}
}

//-------------------------------------------------
Brightness::Brightness(CmdHandler* cmdHandler,const ConfigParser* cfg) :
		Command(cmdHandler,cfg), mBrightness(0)
{

}
Brightness::~Brightness()
{

}

void Brightness::SetParam(const Json::CmdBasic* cmdInfo)
{
    if (NULL == cmdInfo)
        return;

    Command::SetParam(cmdInfo);

    // device type
    if(mParams.size() > 0)
    	mBrightness = atoi(mParams[0].c_str());
    else
    	mBrightness = mCfg->mDefaultBrightness;
}

int Brightness::Execute()
{
	//LCD command
	for(std::map<std::string,CommandExecResult>::iterator iter = mAvailableLCDCmdMap.begin();
			iter != mAvailableLCDCmdMap.end();++iter)
	{
		int val = mBrightness;
		LogD("setBrightness - brightness val - %d\n",val);

		char light[4] = {0};
		sprintf(light,"%02x",val);


		int index = 0;
		unsigned char *databuf = new unsigned char[9];
		databuf[index++] = 0x6B; //cmd1
		databuf[index++] = 0x68; // cmd2
		databuf[index++] = 0x20; // space1
		databuf[index++] = 0x30; // devnum1
		databuf[index++] = 0x30; // devnum2
		databuf[index++] = 0x20; // space2

		databuf[index++] = light[0]; // data1
		databuf[index++] = light[1]; // data2
		databuf[index++] = 0x0D; // CR


		Message* msg = new Message(CMD_LCDPanel,(void*)databuf,index);
		msg->mStr = iter->first;
		sendMessage(msg);
	}

	if(mAvailableLEDCmdMap.size() > 0)
	{
		sendMessage(new Message(CMD_LEDPanel));
	}

	return 0;
}

int Brightness::GetBrightness()
{
    return mBrightness;
}

//-------------------------------------------------
Volume::Volume(CmdHandler* cmdHandler,const ConfigParser* cfg) :
		Command(cmdHandler,cfg), mDevType(T_Unkown), mDevCode(-1)
{

}
Volume::~Volume()
{

}

void Volume::SetParam(const Json::CmdBasic* cmdInfo)
{
    if (NULL == cmdInfo)
        return;

    Command::SetParam(cmdInfo);

    // device type
    if(mParams.size() > 0)
    	mVolumnVal = atoi(mParams[0].c_str());
    else
    	mVolumnVal = mCfg->mDefaultBrightness;
}

int Volume::Execute()
{
	for(std::map<std::string,CommandExecResult>::iterator iter = mAvailableLCDCmdMap.begin();
				iter != mAvailableLCDCmdMap.end();++iter)
	{
		int val = mVolumnVal;

		unsigned char *databuf = new unsigned char[9];
		char vol[4] = {0};
		sprintf(vol,"%02x",val);
		int index = 0;
		databuf[index++] = 0x6B; //cmd1
		databuf[index++] = 0x66; // cmd2
		databuf[index++] = 0x20; // space1
		databuf[index++] = 0x30; // devnum1
		databuf[index++] = 0x30; // devnum2
		databuf[index++] = 0x20; // space2

		databuf[index++] = vol[0]; // data1
		databuf[index++] = vol[1]; // data2
		databuf[index++] = 0x0D; // CR

		Message* msg = new Message(CMD_LCDPanel,(void*)databuf,index);
		msg->mStr = iter->first;
		sendMessage(msg);
	}

	return 0;
}

//-------------------------------------------------
VolumeMute::VolumeMute(CmdHandler* cmdHandler,const ConfigParser* cfg) :
		Command(cmdHandler,cfg), mDevType(T_Unkown), mDevCode(-1)
{
	mIsMute = false;
}
VolumeMute::~VolumeMute()
{

}

void VolumeMute::SetParam(const Json::CmdBasic* cmdInfo)
{
    if (NULL == cmdInfo)
        return;

    Command::SetParam(cmdInfo);

    if("mute" == mCmdName )
    {
    	mIsMute = true;
    }
    else if("recover" == mCmdName)
    {
    	mIsMute = false;
    }

}

int VolumeMute::Execute()
{
	if(mIsMute)
	{
		Message* msg = new Message(CMD_MutexAndRecover,1);
		msg->mStr = mAvailableLCDCmdMap.begin()->first;
		sendMessage(msg);
	}
	else
	{
		Message* msg = new Message(CMD_MutexAndRecover,0);
		msg->mStr = mAvailableLCDCmdMap.begin()->first;
		sendMessage(msg);
	}

/*	for(std::map<std::string,CommandExecResult>::iterator iter = mAvailableLCDCmdMap.begin();
				iter != mAvailableLCDCmdMap.end();++iter)
	{
		unsigned char *databuf = new unsigned char[9];

		int index = 0;
		databuf[index++] = 0x6B; //cmd1
		databuf[index++] = 0x65; // cmd2
		databuf[index++] = 0x20; // space1
		databuf[index++] = 0x30; // devnum1
		databuf[index++] = 0x30; // devnum2
		databuf[index++] = 0x20; // space2

		databuf[index++] = 0x30; // data1
		if(mIsMute)
			databuf[index++] = 0x30; // data2
		else
			databuf[index++] = 0x31; // data2

		databuf[index++] = 0x0D; // CR

		Message* msg = new Message(CMD_LCDPanel,(void*)databuf,index);
		msg->mStr = iter->first;
		sendMessage(msg);
	}
*/
	return 0;
}

//----------------------------------------------
StartUp::StartUp(CmdHandler* cmdHandler,const ConfigParser* cfg) :
        Command(cmdHandler,cfg), mDevType(T_Unkown), mDevCode(-1),
		mPAStatus(0)
{
}

StartUp::~StartUp()
{
}

void StartUp::SetParam(const Json::CmdBasic* cmdInfo)
{
    if (NULL == cmdInfo)
        return;

    Command::SetParam(cmdInfo);

    if(mParams.size() > 0 && mParams[0] == "pa")
    	mPAStatus = 102;
    else
    	mPAStatus = 0;
}

int StartUp::Execute()
{
	for(std::map<std::string,CommandExecResult>::iterator iter = mAvailableLCDCmdMap.begin();
			iter != mAvailableLCDCmdMap.end();++iter)
	{
		//command for lcd panel
		unsigned char *databuf = new unsigned char[9];
		int index = 0;
		databuf[index++] = 0x6B; //cmd1
		databuf[index++] = 0x61; // cmd2
		databuf[index++] = 0x20; // space1
		databuf[index++] = 0x30; // devnum1
		databuf[index++] = 0x30; // devnum2
		databuf[index++] = 0x20; // space2
		databuf[index++] = 0x30; // data1
		databuf[index++] = 0x31; // data2
		databuf[index++] = 0x0D; // CR

		Message* msg = new Message(CMD_LCDPanel,(void*)databuf,index, mPAStatus);
		msg->mStr = iter->first;
		sendMessage(msg);
	}

	if(mAvailableLEDCmdMap.size() > 0)
	{
		sendMessage(new Message(CMD_LEDPanel));
	}

    return 0;
}

ShutDown::ShutDown(CmdHandler* cmdHandler,const ConfigParser* cfg) :
        Command(cmdHandler,cfg), mDevType(T_Unkown), mDevCode(-1),
		mPAStatus(0)
{
}

ShutDown::~ShutDown()
{
}

void ShutDown::SetParam(const Json::CmdBasic* cmdInfo)
{
    if (NULL == cmdInfo)
        return;

    Command::SetParam(cmdInfo);

    if(mParams.size() > 0 && mParams[0] == "pa")
    	mPAStatus = 101;
    else
    	mPAStatus = 0;
}

int ShutDown::Execute()
{
    mExeRslt = 0;

	for(std::map<std::string,CommandExecResult>::iterator iter = mAvailableLCDCmdMap.begin();
			iter != mAvailableLCDCmdMap.end();++iter)
	{
		//command for lcd panel
		unsigned char *databuf = new unsigned char[9];
		int index = 0;
		databuf[index++] = 0x6B; //cmd1
		databuf[index++] = 0x61; // cmd2
		databuf[index++] = 0x20; // space1
		databuf[index++] = 0x30; // devnum1
		databuf[index++] = 0x30; // devnum2
		databuf[index++] = 0x20; // space2
		databuf[index++] = 0x30; // data1
		databuf[index++] = 0x30; // data2
		databuf[index++] = 0x0D; // CR

		Message* msg = new Message(CMD_LCDPanel,(void*)databuf,index, mPAStatus);
		msg->mStr = iter->first;
		sendMessage(msg);
	}

	for(std::map<std::string,CommandExecResult>::iterator iter = mAvailableMainDeviceCmdMap.begin();
			iter != mAvailableMainDeviceCmdMap.end();++iter)
	{
		sendMessage(new Message(CMD_MainDevReply,(int)MSG_CMD_ShutDown));
	}

	if(mAvailableLEDCmdMap.size() > 0)
	{
		sendMessage(new Message(CMD_LEDPanel));
	}


    return mExeRslt;
}

Reboot::Reboot(CmdHandler* cmdHandler,const ConfigParser* cfg) :
        Command(cmdHandler,cfg), mDevType(T_Unkown), mDevCode(-1)
{
}

Reboot::~Reboot()
{
}

void Reboot::SetParam(const Json::CmdBasic* cmdInfo)
{
    if (NULL == cmdInfo)
        return;

    Command::SetParam(cmdInfo);
}

int Reboot::Execute()
{
    mExeRslt = 0;

	for(std::map<std::string,CommandExecResult>::iterator iter = mAvailableMainDeviceCmdMap.begin();
			iter != mAvailableMainDeviceCmdMap.end();++iter)
	{
		sendMessage(new Message(CMD_MainDevReply,(int)MSG_CMD_ReBoot));
	}

    return mExeRslt;
}

GetLog::GetLog(CmdHandler* cmdHandler,const ConfigParser* cfg) :
        Command(cmdHandler,cfg)
{
}

GetLog::~GetLog()
{
}

void GetLog::SetParam(const Json::CmdBasic* cmdInfo)
{
}

int GetLog::Execute()
{
    mExeRslt = 0;

    if (NULL == mCfg)
        return mExeRslt;

    std::string copycmd = "";
    std::string strzipcmd = "";

    std::string copedfilename = "";
    std::string strSWlogprefix = "SW_" + mCfg->mDeviceId + "_";
    std::string strzippkgpathname = mCfg->mSWlogPath + "/" + strSWlogprefix
            + SystemClock::Today(SystemClockDateFormat) + ".zip";

    bool hasZipFile = false;

    // scan the software path and store the file name.
    std::list<std::string> logList;
    int count = FileSysUtils::ScanDirFiles(mCfg->mSWlogPath, logList, false);
    if (count < 1)
        return mExeRslt;

    // TODO: judge the file name. (Just handle the log one day earlier)

    for (std::list<std::string>::iterator i = logList.begin();
            i != logList.end(); i++)
    {
        copedfilename = mCfg->mSWlogPath + "/" + strSWlogprefix
                + FileSysUtils::Path2File(*i);

        copycmd = "cp -f " + (*i) + " " + copedfilename;
        system(copycmd.c_str());
        system("sync");

        if (FileSysUtils::Accessible(copedfilename, FileSysUtils::FR_OK))
        {
            strzipcmd = "zip -qj " + strzippkgpathname + " " + copedfilename;
            LogE("zip command(%s)\n", strzipcmd.c_str());
            if (system(strzipcmd.c_str()) != -1)
            {
//                Message* msg = new Message(MoveuploadedLogs);
//                msg->mStr = copedfilename;
//                sendMessage(msg);

                // TODO:  Move the file to upload directory.
                hasZipFile = true;
            }
        } else
        {
            LogE("software log do not exist(%s)\n", copedfilename.c_str());
            mExeRslt = 2; // failed
            return mExeRslt;
        }
    }

    if (hasZipFile
            && FileSysUtils::Accessible(strzippkgpathname, FileSysUtils::FR_OK))
    {
        //mXmlSchedule->uploadFile(strzippkgpathname, XmlSchedule::SoftwareLog,
        //FtpTrans::TransFlagUploadRemoveSrc)

        // TODO:upload to server and remove the uploaded log in upload directory!

        mExeRslt = 1;
        return mExeRslt; // succeed

    } else
    {
        LogE("software log zip file do not exist(%s)\n",
                strzippkgpathname.c_str());

        mExeRslt = 2; // failed
        return mExeRslt;
    }
}

GetSnapshot::GetSnapshot(CmdHandler* cmdHandler, const ConfigParser* cfg) :
        Command(cmdHandler,cfg)
{
}

GetSnapshot::~GetSnapshot()
{
}

void GetSnapshot::SetParam(const Json::CmdBasic* cmdInfo)
{
}

int GetSnapshot::Execute()
{
// TODO: call uploader to upload snapshot.
    //if()  Upload Files!!!!!!!!!!!!!!!!!!!!

    if (mCmdHandler != NULL)
    {
        const TransManager* transMng = mCmdHandler->GetManager();

        //TODO: Generate the zip file. Send the zip file of snap shots!
        std::string fileName = "testfile.zip";
        //transMng->doUpLoad(this,DataTrans::DataTransType_Resource,UP_CmdExeReply,fileName,NULL);
    }

    mExeRslt = 0;
    return mExeRslt;
}

bool GetSnapshot::handleMessage(Message *msg)
{
    // TODO: .....

    // 1. Receive the msg from doUpload finished. -- upload success or not

    // 2. Send MSG_CmdCompleted to CMDHandler

    return false;
}

HouseKeeping::HouseKeeping(CmdHandler* cmdHandler,const ConfigParser* cfg) :
        Command(cmdHandler,cfg)
{
}

HouseKeeping::~HouseKeeping()
{
}

void HouseKeeping::SetParam(const Json::CmdBasic* cmdInfo)
{
}

int HouseKeeping::Execute()
{
// TODO: Do house-keeping work.
    int fileCnt = 0;
    mExeRslt = 0;

    if (mCfg != NULL)
    {
        fileCnt += FileSysUtils::HouseKeeping(mCfg->mDldLogPath,
                mCfg->mHouseKeepDay);
        fileCnt += FileSysUtils::HouseKeeping(mCfg->mPlayLogPath,
                mCfg->mHouseKeepDay);
        fileCnt += FileSysUtils::HouseKeeping(mCfg->mSftLogPath,
                mCfg->mHouseKeepDay);
        fileCnt += FileSysUtils::HouseKeeping(mCfg->mSubDevLogPath,
                mCfg->mHouseKeepDay);
    }

    if (fileCnt > 0)
        mExeRslt = 1;

    return mExeRslt;
}

Command* CmdFactory::GenerateCmd(const CommandMsgType& cmdType,
        const ConfigParser* cfg,CmdHandler* cmdHandler)
{
    Command* cmd = NULL;

    switch (cmdType)
    {
    case MSG_CMD_StartUp:
        cmd = new StartUp(cmdHandler,cfg);
        break;
    case MSG_CMD_ShutDown:
        cmd = new ShutDown(cmdHandler,cfg);
        break;
    case MSG_CMD_ReBoot:
        cmd = new Reboot(cmdHandler,cfg);
        break;
    case MSG_CMD_GetLog:
        cmd = new GetLog(cmdHandler,cfg);
        break;
    case MSG_CMD_GetSnapShot:
        //cmd = new GetSnapshot(cfg);
        break;
    case MSG_CMD_HouseKeeping:
        cmd = new HouseKeeping(cmdHandler,cfg);
        break;
    case MSG_CMD_SetBrightness:
    	cmd = new Brightness(cmdHandler,cfg);
    	break;
    case MSG_CMD_SetVolumn:
    	cmd = new Volume(cmdHandler,cfg);
    	break;
    case MSG_CMD_SetVolumeMute:
    	cmd = new VolumeMute(cmdHandler,cfg);
    	break;
    case MSG_CMD_Unknown:
        break;
    default:
        break;
    }

    return cmd;
}

int Command::OpenDev(Termios& dev,std::string sdid)
{
    if (NULL == mCfg || sdid.size() == 0)
        return -1;

    int fd = -1;
    // Open device.
    std::map<std::string, std::string>::const_iterator iter1 = mCfg->mDevicePortMap.find(sdid);
    std::map<std::string, int>::const_iterator iter2 = mCfg->mBaudRateMap.find(sdid);
    if(iter1 != mCfg->mDevicePortMap.end() && iter2 != mCfg->mBaudRateMap.end())
    {
    	fd = dev.open(iter1->second, Termios::RW);
		if (fd < 0)
			return -1;

		dev.setBaudRate(iter2->second);

		std::map<std::string, int>::const_iterator iter3 = mCfg->mDataBitsMap.find(sdid);
		if(iter3 != mCfg->mDataBitsMap.end())
			dev.setDataBits(iter3->second);
		else
			dev.setDataBits(8);

		iter3 = mCfg->mStopBitsMap.find(sdid);
		if(iter3 != mCfg->mStopBitsMap.end())
			dev.setStopBits(iter3->second);
		else
			dev.setStopBits(1);

		iter3 = mCfg->mParityMap.find(sdid);
		if(iter3 != mCfg->mParityMap.end())
			dev.setParityCheck((Termios::ParityCheckMode)(iter3->second));
		else
			dev.setParityCheck(Termios::PC_None);

		iter3 = mCfg->mFlowctrlMap.find(sdid);
		if(iter3 != mCfg->mFlowctrlMap.end())
			dev.setFlowControl((Termios::FlowControlMode)(iter3->second));
		else
			dev.setFlowControl(Termios::FC_None);
    }


    return fd;
}

int Command::sendLCDCommand(Handler* handler,void* data, int len, std::string sdid)
{
	Termios lcd;

	if (OpenDev(lcd,sdid) < 0)
	{
		LogE("openLCDDev error\n");
		return -1;
	}

	int sendlen = 0;
	int fd = lcd.getFD();
	if (TEMP_FAILURE_RETRY(
			sendlen = ::write(fd, data, len) != len))
	{
		LogE("write lcd monitor error [fd = %d]:%s,  sendlen - %d\n", fd, strerror(errno),sendlen);
		lcd.close();
		return -1;
	}

	usleep(1000*500);
	int recvlen = ::read(fd, mSerialReadBuf, LCD_DATA_LENTH);

	if (recvlen <= 0)
	{
		LogE("LCD Panel reply data failed1 %d.\n",errno);
		lcd.close();

		//for LCD panel don't return result
//		mSeralReadLen = 0;
//		if(handler != NULL)
//		{
//			Message* msg1 = new Message(CMD_LCDReply,(void*)mSerialReadBuf,mSeralReadLen);
//			msg1->mStr = sdid;
//			handler->sendMessage(msg1);
//		}

		return -1;
	}
	else
	{
		for(int i = 0;i<recvlen;++i)
		{
			LogD("####read data  %02x\n",mSerialReadBuf[i]);
		}

		mSeralReadLen = recvlen;
//		if(handler != NULL)
//		{
//			Message* msg1 = new Message(CMD_LCDReply,(void*)mSerialReadBuf,mSeralReadLen);
//			msg1->mStr = sdid;
//			handler->sendMessage(msg1);
//		}
	}

	lcd.close();

	return 0;
}

int Command::checkCmdRunStatus(void * data,int len)
{
	if(data == NULL || len == 0)
	{
		LogE("Reply data failed.\n");
		return RunFailed;
	}

	unsigned char* buf = (unsigned char*)data;
	if(len == 10 && buf != NULL && buf[len-1] == 0x78)
	{
		if(buf[5] == 0x4F)
			return RunSuccess;
	}

	return RunFailed;
}

void Command::GenerateReply(int status, const std::string& devId,
        std::string& rplyStr)
{
    Json::CmdExeReply rply;
    rply.mId = mCmdId;
    rply.mDevice = devId;
    rply.mStatus = status;
    rply.mRepTime = SystemClock::Today(SystemClockTMFormat);

    Json::CmdExeReply::ToJson(&rply, rplyStr);
}

std::string Command::GetCmdName()
{
    return mCmdName;
}

/**
 * Get the command id.
 */
int Command::GetCmdId()
{
    return mCmdId;
}

bool Command::setSuitableDev(std::vector<std::string>& targets, int LCD_LED_flag)
{
	std::map<std::string, std::string>::const_iterator iter_port;
	std::map<std::string, int>::const_iterator iter_rate;

	for(std::vector<std::string>::iterator iter = targets.begin();
			iter != targets.end();++iter)
	{
		iter_port = mCfg->mDevicePortMap.find(*iter);
		iter_rate = mCfg->mBaudRateMap.find(*iter);

		if((*iter) == mCfg->mDeviceId)
		{
			mAvailableMainDeviceCmdMap[*iter] = RES_ReadyToExec;
		}

		if(LCD_LED_flag == (int)LCD_Controller_flag)
		{
			if(iter_port !=mCfg->mDevicePortMap.end() && iter_rate != mCfg->mBaudRateMap.end())
			{
				mAvailableLCDCmdMap[*iter] = RES_ReadyToExec;
			}
		}else if(LCD_LED_flag == (int)LED_Controller_flag)
		{
			 if(iter_port !=mCfg->mDevicePortMap.end() && iter_rate != mCfg->mBaudRateMap.end()) //LED command
			{
				mAvailableLEDCmdMap[*iter] = RES_ReadyToExec;
			}
		}
		else
		{
			LogD("Unknown controller flag : %d.\n",LCD_LED_flag);
		}
	}

	if(mAvailableMainDeviceCmdMap.size() == 0 && mAvailableLCDCmdMap.size() == 0 && mAvailableLEDCmdMap.size() == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool Command::setCommandExecStatus(std::string sdid,CommandExecResult res)
{
	bool bret = true;

	std::map<std::string,CommandExecResult>::iterator iter = mAvailableLEDCmdMap.find(sdid);
	if(iter != mAvailableLEDCmdMap.end())
	{
		mAvailableLEDCmdMap[sdid] = res;
	}

	iter = mAvailableLCDCmdMap.find(sdid);
	if(iter != mAvailableLCDCmdMap.end())
	{
		mAvailableLCDCmdMap[sdid] = res;
	}

	iter = mAvailableMainDeviceCmdMap.find(sdid);
	if(iter != mAvailableMainDeviceCmdMap.end())
	{
		mAvailableMainDeviceCmdMap[sdid] = res;
	}

//////////////////////////
	for(iter = mAvailableLEDCmdMap.begin();	iter != mAvailableLEDCmdMap.end();++iter)
	{
		if(iter->second != RES_ExecSuccess)
		{
			bret = false;
			break;
		}
	}

	for(iter = mAvailableLCDCmdMap.begin();	iter != mAvailableLCDCmdMap.end();++iter)
	{
		if(iter->second != RES_ExecSuccess)
		{
			bret = false;
			break;
		}
	}

	for(iter = mAvailableMainDeviceCmdMap.begin();	iter != mAvailableMainDeviceCmdMap.end();++iter)
	{
		if(iter->second != RES_ExecSuccess)
		{
			bret = false;
			break;
		}
	}

	return bret;
}

/**
 * Get command execute result.
 */
int Command::GetExeRslt()
{
    return mExeRslt;
}

const char *CmdFactory::TAG = "CmdFactory";
const char *Command::TAG = "Command";
const char *Brightness::TAG = "Brightness";
const char *Volume::TAG = "Volume";
const char *VolumeMute::TAG = "VolumeMute";
const char *StartUp::TAG = "StartUp";
const char *ShutDown::TAG = "ShutDown";
