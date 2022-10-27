/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file name : configparser.cpp
 * @author : wangfuqiang
 * @date : 2017/8/8 14:29
 * @brief : parse config file
 */
#include "configparser.h"
#include "FileSysUtils.h"
#include "Log.h"

ConfigParser& ConfigParser::Instance()
{
	static ConfigParser s_config("./","config.xml");
	return s_config;
}

ConfigParser::ConfigParser(const std::string& path, const std::string& name)
{
    mConfigFile = "";
    if (path.size() == 0)
    {
        mConfigFile.append("./").append(name);
    } else
    {
        mConfigFile.append(path).append("/").append(name);
    }

    if (!FileSysUtils::Accessible(mConfigFile, FileSysUtils::FR_OK))
    {
        LogE("Config file don't exist.\n");
    }

	mParentNodeName = "";

    getKeyPairList();
    load();
}

ConfigParser::~ConfigParser()
{

}

bool ConfigParser::getKeyPairList()
{
    KeyValuePair kvp[] =
            {
                    // account
                    { "account", &mAccount, 'S', " " },
                    { "password", &mPassWord, 'S', " " },

                    // system
					{ "lcd_led_flag", &mLCDLEDFlag, 'I', "0" },
					{ "module_name", &mModuleName, 'S', "LCDController" },
                    { "version", &mVersion, 'S', "0.001" },
                    { "device_id", &mDeviceId, 'S', "ST01-LCD1" },
                    { "station_id", &mStationId, 'S', "ST01" },
                    { "platform_id", &mPlatformId, 'S', "PLATFORM_01" },

                    { "swlog_direct", &mSWlogDirect, 'I', "0" },
                    { "screen_width", &mScreenWidth, 'I', "1920" },
                    { "screen_height", &mScreenHeight, 'I', "1080" },

                    // server
                    { "centerserver_ip", &mCenterServerIP, 'S',
                            "192.168.250.190" },
                    { "interserver_ip", &mInterServerIP, 'S', "192.168.250.191" },
                    { "centerserver_port", &mCenterServerPort, 'I', "1984" },
					{ "websocket_port", &mWebsocketPort, 'I', "980" },
                    { "interserver_port", &mInterServerPort, 'I', "8080" },

                    //live
                    { "multicast_ip", &mMultiCastStreamIP, 'S', "232.255.42.42" },
                    { "multicast_port", &mMultiCastStreamPort, 'I', "1234" },
                    { "detect_timeout", &mMultiCastTimeout, 'I', "1" },
					{ "videosync_server", &mIsVideoSyncServer, 'I', "0" },
					{ "videosync_clientip", &mVideoSyncServerIP, 'S', "192.168.101.47" },
                    { "stream_fifosize", &mLiveStreamfifosize, 'S', "10000" },
                    { "bufferreopen_enable", &mBufferReopenEnable, 'I', "1" },

                    // json
                    { "schedules_list_filename", &mSchedulesListFilename, 'S',
                            "schedule_list.json" },
                    { "schedules_filename", &mSchedulesFilename, 'S',
                            "schedules.json" },
                    { "layout_groups_filename", &mLayoutGroupFilename, 'S',
                            "layout_groups.json" },
                    { "resources_filename", &mResourcesFilename, 'S',
                            "resources.json" },

                    // file
                    { "download_rootdir", &mDldRootDir, 'S', "/home/tvs/" },
                    { "schedule_downloadpath", &mSchDldPath, 'S',
                            "/schedule/download/" },
                    { "schedule_playpath", &mSchPlayPath, 'S', "/schedule/play/" },
                    { "swlog_path", &mSWlogPath, 'S',
                            "/home/workspace/GPIDS_Tongxing/code/LCDController/Debug" },
                    { "content_path", &mContentpath, 'S',
                            "/home/workspace/GPIDS/content/" },
                    // api
                    { "login_subpath", &mLoginPath, 'S', "/api/v1/login" },
                    { "refresh_subpath", &mRefreshPath, 'S', "/api/v1/refresh" },
                    { "schedules_list_subpath", &mSchedulesListSubpath, 'S',
                            "/api/v1/schedules?device=" },
                    { "schedule_subpath", &mScheduleSubpath, 'S',
                            "/api/v1/schedules/" },
                    { "layoutgroup_subpath", &mLayoutGroupSubpath, 'S',
                            "/api/v1/layout_group/" },
                    { "resource_subpath", &mResourcesSubpath, 'S',
                            "/api/v1/medias/" },
                    { "resource_info_subpath", &mResourceInfoSubpath, 'S',
                            "/api/v1/resource_info/" },
					{ "resource_content_subpath", &mResourceContentSubpath, 'S',
							"/api/v1/resource/" },
                    { "opsmsg_list_subpath", &mRTOPSMsgListSubpath, 'S',
                            "/api/v1/operational_messages?device={device}" },
                    { "opsmsg_detail_subpath", &mRTOPSMsgDetailSubpath, 'S',
                            "/api/v1/operational_messages?device={device}" },
                    { "ops_reply_subpath", &mRTOPSMsgReplySubpath, 'S',
                            "/api/v1/operational_messages/{operational_mesage_id}/reply" },
                    { "dev_status_subpath", &mDevStatusSubpath, 'S',
                            "/api/v1/logs/current_status/" },

                    { "download_file", &mDldFileSubpath, 'S',
                            "/api/v1/resource/{filename}" },
                    { "upload_file", &mUpldFileSubpath, 'S',
                            "/api/v1/logs?file={file_name}&usage={usage}" },
                    { "cmd_list_subpath", &mCmdlistSubpath, 'S',
                            "/api/v1/commands?device={device}" },
                    { "cmd_subpath", &mCmdSubpath, 'S',
                            "/api/v1/commands/{command_id}" },
                    { "cmd_rply_subpath", &mCmdRplySubpath, 'S',
                            "/api/v1/device_commands/{command_id}" },
                    { "arrival_info_subpath", &mArrInfoSubpath, 'S',
                            "/api/v1/arrival_information?station={station}&platform={platform}" },
                    { "server_subpath", &mServerSubpath, 'S',
                            "/api/v1/mesage/{code}" },
					{ "train_time_subpath", &mTrainTimeSubPath, 'S',
							"/api/v1/fl_train_config?device=" },
					{ "screen_onoff_subpath", &mScreenOnOffPath, 'S',
							"/api/v1/screen_on_off_config?device=" },
					{ "livesource_switch_subpath", &mLiveSwitchPath, 'S',
							"/api/v1/switch_source_config?device=" },

                    // monitor
                    { "adapter_client_name", &mAdapterClientName, 'S',
                            "LED_Client_1" },
                    { "monitor_deviceport", &mDevicePortMap, 'L', "/dev/ttyUSB0" },
                    { "monitor_baudrate", &mBaudRateMap, 'M', "115200" },
                    { "monitor_databits", &mDataBitsMap, 'M', "8" },
                    { "monitor_stopbits", &mStopBitsMap, 'M', "1" },
                    { "monitor_parity", &mParityMap, 'M', "0" },
                    { "monitor_flowctrl", &mFlowctrlMap, 'M', "0" },
                    { "monitor_addr", &mDeviceAddrMap, 'M', "0" },
                    { "monitor_screencnt", &mDeviceScreenCntMap, 'M', "1" },
                    { "monitor_width", &mDeviceWidthMap, 'M', "128" },
                    { "monitor_height", &mDeviceHeightMap, 'M', "32" },

                    // snapshot
					{ "snapshot_enable", &mSnapShotEnable, 'I', "0" },
                    { "snapshot_period", &mSnapShotPeriod, 'I', "30" },
                    { "snapshot_height", &mSnapShotH, 'I', "80" },
                    { "snapshot_width", &mSnapShotW, 'I', "60" },
                    { "snapshot_suffix", &mSnapShotSfx, 'S', ".jpg" },
                    { "snapshot_path", &mSnapShotPath, 'S',
                            "/home/workspace/snapshot" },
//
//                    // house keeping
                    { "housekeeping_period", &mHouseKeepDay, 'I', "7" },
					{ "coredump_number", &mCoreDumpNumber, 'I', "7" },
//
//                    // download_logpath
                    { "download_logpath", &mDldLogPath, 'S',
                            "/home/workspace/log/download" },
                    { "play_logpath", &mPlayLogPath, 'S',
                            "/home/workspace/log/play" },
                    { "software_logpath", &mSftLogPath, 'S',
                            "/home/workspace/log/software" },
                    { "subdevice_logpath", &mSubDevLogPath, 'S',
                            "/home/workspace/log/subdevice" },
					{ "watchdog_logpath", &mWatchdogLogPath, 'S',
							"/home/workspace/log/watchdog" },
					{ "coredump_logpath", &mCoreDumpLogPath, 'S',
							"/home/workspace/log/coredump" },

                    // report config
                    { "status_report_period", &mStatusRptPeriod, 'I', "60" },

                    //default
                    { "default_brightness", &mDefaultBrightness, 'I', "50" },
                    { "default_volumn", &mDefaultVolumn, 'I', "50" },

                    //ntpinfo
					{ "ntp_enable", &mNtpEnable, 'I', "0" },
                    { "ntp_server", &mNtpServer, 'S', "192.168.101.38" },
                    { "ntpdate_period", &mNtpDatePeriod, 'I', "600" },

					//ntpinfo
					{ "lcdpanel_enable", &mOnOffLCDPanelEnable, 'I', "1" },
                    { "shutdown_lcdpanel", &mShutdownLcdTime, 'S', "010000" },
                    { "poweron_lcdpanel", &mPoweronLcdTime, 'S', "040000" },

					 //arrivalmsg
					{ "signalinterrupt_duration", &mSignalInterruptDuration, 'I', "30" },

                    { NULL, NULL, 0, NULL } };

    for (KeyValuePair *kv = kvp; kv->mKey != NULL; kv++)
    {
        kv->setValue(NULL,NULL);
        mKvplist.push_back(*kv);
    }

    return true;
}

bool ConfigParser::load()
{
    ptree pt;
    ptree root;

    read_xml(mConfigFile, pt);

    root = pt.get_child("");

    parsechild(root);

    return true;
}

bool ConfigParser::parsechild(boost::property_tree::ptree node)
{
    ptree child;

    BOOST_FOREACH(ptree::value_type &v, node)
    {

        child = v.second.get_child("");

        if (v.first == XMLCOMMENT)
        {
            continue;
        }

        if (child.size() == 0)
        {

            for (std::list<KeyValuePair>::iterator kvi = mKvplist.begin();
                    kvi != mKvplist.end(); kvi++)
            {
                if (strncmp(v.first.c_str(), kvi->mKey, strlen(kvi->mKey)) == 0)
                {
                    if (mParentNodeName.size() > 0)
                    {
                        kvi->setValue(v.second.data().c_str(),
                                mParentNodeName.c_str());
                    } else
                    {
                        kvi->setValue(v.second.data().c_str(), NULL);
                        mKvplist.erase(kvi);
                    }

                    break;
                }
            }
            continue;
        } else
        {
            std::size_t fresLcd = v.first.find(LCDPanel_DeviceID_prefix);
            std::size_t fresLed = v.first.find(LED_DeviceID_prefix);
            if (fresLcd != std::string::npos || fresLed != std::string::npos)
            {
                std::string sdata = v.first.data();
                mParentNodeName = sdata.substr(LCD_LED_prefix_len,
                        sdata.length() - LCD_LED_prefix_len);
            } else
            {
                mParentNodeName = "";
            }

            parsechild(child);

            mParentNodeName = "";
        }
    }

//	for (std::list<KeyValuePair>::iterator kvi = mKvplist.begin(); kvi != mKvplist.end();) {
//		kvi->setValue(NULL);
//		kvi = mKvplist.erase(kvi);
//	}

    return true;
}

void ConfigParser::ModifyScreenOnOffTime(std::string ontime, std::string offtime)
{
	 ptree pt;
	 read_xml(mConfigFile, pt, boost::property_tree::xml_parser::trim_whitespace);

	 bool bfind = false;
	 BOOST_FOREACH(ptree::value_type &v, pt.get_child("config"))
	 {
		 if(v.first == "lcdpoweronoff-config")
		 {
			 bfind = true;
			 break;
		 }
	 }

	 if(bfind)
	 {
		 if(ontime.size()==6)
			 pt.put("config.lcdpoweronoff-config.poweron_lcdpanel", ontime);

		 if(offtime.size()==6)
			 pt.put("config.lcdpoweronoff-config.shutdown_lcdpanel", offtime);


		 boost::property_tree::xml_parser::xml_writer_settings<char> settings('\t', 1, "utf-8");
		 write_xml(mConfigFile, pt, std::locale(), settings);
	 }
	 else
	 {
		 LogE("--------don't find screen on-off config item\n");
	 }

}

void ConfigParser::Modify(const char* xml_path,const std::string& val)
{
	ptree pt;
	read_xml(mConfigFile, pt,
			boost::property_tree::xml_parser::trim_whitespace);

	std::string parent(xml_path);
	parent=parent.substr(parent.find('.')+1);
	parent=parent.substr(0,parent.find('.'));
	bool bfind = false;
	BOOST_FOREACH(ptree::value_type &v, pt.get_child("config"))
	{
		if (v.first == parent)
		{
			bfind = true;
			break;
		}
	}

	if (bfind)
	{
		pt.put(xml_path, val);

		boost::property_tree::xml_parser::xml_writer_settings<char> settings('\t', 1, "utf-8");
		write_xml(mConfigFile, pt, std::locale(), settings);
	}
	else
	{
		LogE("--------can't find %s\n",parent.c_str());
	}
}

void ConfigParser::GetSubPath(const TransFileType type, std::string& path) const
{
    path.clear();

    switch (type)
    {
    case DL_SchedulesUpdateList:
        path = mSchedulesListSubpath;
        break;
    case DL_Schedules:
        path = mScheduleSubpath;
        break;
    case DL_LayoutGroups:
        path = mLayoutGroupSubpath;
        break;
    case DL_Medias:
        path = mResourcesSubpath;
        break;
    case DL_MediaContent:
        path = mResourceContentSubpath;
        break;
    case DL_MediaMD5:
        path = mResourceInfoSubpath;
        break;
    case DL_CmdsUpdateList:
        path = mCmdlistSubpath;
        break;
    case DL_CmdRequest:
        path = mCmdSubpath;
        break;
    case UP_CmdExeReply:
        path = mCmdRplySubpath;
        break;
    case DL_OPMsgList:
        path = mRTOPSMsgListSubpath;
        break;
    case DL_OPMsgRequest:
        path = mRTOPSMsgDetailSubpath;
        break;
    case DL_OPSBackImage:
        path = mDldFileSubpath;
        break;
    case UP_OPSReplyResult:
        path = mRTOPSMsgReplySubpath;
    case DL_RealTimeWeather:
        break;
    case DL_RealTimeArrMsgList:
        break;
    case DL_RealTimeArrMsg:
        break;
    case DL_ArrivalInfo:
        path = mArrInfoSubpath;
        break;
    case DL_TrainTimeConfig:
    	path = mTrainTimeSubPath;
    	break;
    case DL_ScreenOnOffConfig:
    	path = mScreenOnOffPath;
    	break;
    case DL_LiveSwithConfig:
    	path = mLiveSwitchPath;
		break;
    case DL_LoadFromLocal:
        break;
    default:
        path = "";
    }
}

void ConfigParser::EnsureDirExist()
{

}

const std::string ConfigParser::XMLCOMMENT = "<xmlcomment>";
const char *ConfigParser::TAG = "ConfigParser";

