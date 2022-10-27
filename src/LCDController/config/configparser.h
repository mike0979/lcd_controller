/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : configparser.h
* @author : wangfuqiang
* @date : 2017/8/8 14:29
* @brief : parse config file
*/
#ifndef CONFIG_CONFIGPARSER_H_
#define CONFIG_CONFIGPARSER_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <CommonDef.h>
#include <string>
#include "KeyValuePair.h"
#include <list>

using boost::property_tree::ptree;

class ConfigParser {

public:
	static ConfigParser& Instance();
	ConfigParser(const std::string& path,const std::string& name);
	~ConfigParser();

	/**
	 * Get the sub path according to download file type.
	 * @param type[in]:
	 * @param path[out]:
	 */
	void GetSubPath(const TransFileType type,std::string& path) const;

	/**
	 * Ensure directory in config file exist.If a directory is not exist,
	 * then create it.
	 */
	void EnsureDirExist();

	void ModifyScreenOnOffTime(std::string ontime, std::string offtime);
	void Modify(const char* xml_path,const std::string& val);

private:
	bool getKeyPairList();
	bool load();
	bool parsechild(boost::property_tree::ptree node);
public:
	//account-config
	std::string mAccount;
	std::string mPassWord;
	//system-config
	int mLCDLEDFlag;
	std::string mModuleName;
	std::string mVersion;
	std::string mDeviceId;
	std::string mStationId;
	std::string mPlatformId;
    int mScreenWidth;
    int mScreenHeight;

	//server-config
	std::string mCenterServerIP;
	std::string mInterServerIP;
	int mCenterServerPort;
	int mWebsocketPort;
	int mInterServerPort;
	int mSWlogDirect;

	//live-config
    std::string mMultiCastStreamIP;
    int mMultiCastStreamPort;
    int mMultiCastTimeout;
    int mIsVideoSyncServer;
    std::string mVideoSyncServerIP;
    std::string mLiveStreamfifosize;
    int mBufferReopenEnable;

	//json-config
	std::string mSchedulesListFilename;
	std::string mSchedulesFilename;
	std::string mLayoutGroupFilename;
	std::string mResourcesFilename;
	std::string mDldRootDir;
	std::string mSchDldPath;
	std::string mSchPlayPath;

	//api-config
	std::string mLoginPath;
	std::string mRefreshPath;
	std::string mSWlogPath;
	std::string mSchedulesListSubpath;
	std::string mScheduleSubpath;
	std::string mLayoutGroupSubpath;
	std::string mResourcesSubpath;
	std::string mResourceInfoSubpath;
	std::string mResourceContentSubpath;
	std::string mRTOPSMsgListSubpath;
	std::string mRTOPSMsgDetailSubpath;
	std::string mRTOPSMsgReplySubpath;
	std::string mCmdlistSubpath;
	std::string mCmdSubpath;
	std::string mCmdRplySubpath;
	std::string mDldFileSubpath; // url of download file
	std::string mUpldFileSubpath; // url of upload file.
	std::string mArrInfoSubpath;
	std::string mDevStatusSubpath;
	std::string mServerSubpath;
	std::string mTrainTimeSubPath;
	std::string mScreenOnOffPath;
	std::string mLiveSwitchPath;

	//file-config
	std::string mContentpath; // path of all content file.


	//snapshot-config
	int mSnapShotEnable;
	int mSnapShotPeriod;
	int mSnapShotH;
	int mSnapShotW;
	std::string mSnapShotSfx;
	std::string mSnapShotPath;

	//housekeeping-config
	int mHouseKeepDay;
	int mCoreDumpNumber;

	//log-config
	std::string mDldLogPath;
	std::string mPlayLogPath;
	std::string mSftLogPath;
	std::string mSubDevLogPath;
	std::string mWatchdogLogPath;
	std::string mCoreDumpLogPath;

	//monitor-config
	std::string mAdapterClientName;
	std::map<std::string,int> mDeviceAddrMap;
    std::map<std::string,int> mDeviceScreenCntMap;
    std::map<std::string,int> mDeviceWidthMap;
	std::map<std::string,int> mDeviceHeightMap;

	std::map<std::string,std::string> mDevicePortMap;
	std::map<std::string,int> mBaudRateMap;
	std::map<std::string,int> mDataBitsMap;
	std::map<std::string,int> mStopBitsMap;
	std::map<std::string,int> mParityMap;
	std::map<std::string,int> mFlowctrlMap;

	std::string mDevicePort;
	int mBaudRate;
	int mDataBits;
	int mStopBits;
	int mParity;
	int mFlowctrl;

	//report-config
	int mStatusRptPeriod;

	//defaultval
	int mDefaultBrightness;
	int mDefaultVolumn;

	//ntpinfo
	int mNtpEnable;
	std::string mNtpServer;
	int mNtpDatePeriod;

	int mOnOffLCDPanelEnable;
	std::string mShutdownLcdTime;
	std::string mPoweronLcdTime;

	//arrivalmsg-config
	int mSignalInterruptDuration;

private:

	std::list<KeyValuePair> mKvplist;
	std::string mConfigFile;
	std::string mParentNodeName;
	static const std::string XMLCOMMENT;
	static const char *TAG;
};

#endif /* CONFIG_CONFIGPARSER_H_ */
