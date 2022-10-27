/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : ScheduleObjs.h
 * @author : Benson
 * @date : Aug 17, 2017
 * @brief :
 */

#ifndef SRC_JSON_SCHEDULEOBJS_H_
#define SRC_JSON_SCHEDULEOBJS_H_

#include <CommonDef.h>
#include <rapidjson/document.h>
#include <string>
#include <vector>
#include <map>
#include "bj_pis/convent/bj_layout_info.h"

struct OpmMsg;

namespace Json
{

/**
 * Basic information of a schedule.
 */
class ScheduleBasic
{
public:
    ScheduleBasic();
    virtual ~ScheduleBasic();

    int mId;
    std::string mName;
    std::string mDescrp;  // description
    std::string mStartTime;
    std::string mEndTime;
    std::string mPublishTime;
    std::string mUpdateTime;
    int mServerLevel;
    int mPriority;
};

/**
 * class for schedule lists
 * Desc:
 */
class ScheduleList
{
public:
    ScheduleList();
    virtual ~ScheduleList();

    std::vector<ScheduleBasic> mSchedules;
};

class PartitionMedias
{
public:
    PartitionMedias();
    virtual ~PartitionMedias();

    int mPartitionId;
    std::vector<int> mMediaIds;
    int basMediaId{0};//add by guorenjing 20220809_1604
};

class SchLayoutGroupBasic
{
public:
    SchLayoutGroupBasic();
    ~SchLayoutGroupBasic();

    int mId;
    std::string mDscrp; // description
    std::string mStartTime;
    std::string mEndTime;
    int mSwitchTime; // switch time between layouts of layout group.
    std::string mUpdatedTime;
    std::vector<PartitionMedias> mPartionMedias;
};

/**
 * Detail information of a schedule.
 * Desc:
 */
class ScheduleDetail
{
public:
    ScheduleDetail();
    ~ScheduleDetail();

    ScheduleBasic mScheduleBasic;
    int mPriority;
    std::vector<SchLayoutGroupBasic> mLayoutGroups;
};

class PartitionDetail
{
public:
    PartitionDetail();
    virtual ~PartitionDetail();

    int mId;
    int mMediaType;
    bool mIsSoundable;
    int mXpos;
    int mYpos;
    int mWidth;
    int mHeight;
    int mZorder;
    bool mIsTransparent;
    bool mIsMaster;
    //int mBGid; // background media id
    std::string mBkgroudFile; // back ground file
    bool mOPSflag;
    //std::vector<int> mMediaIds;
    std::string mUpdateTime;
};

/**
 * Detail information of a layout.
 * Desc:
 */
class LayoutDetail
{
public:
    LayoutDetail();
    virtual ~LayoutDetail();

    void sortPartationByZOrder();

    int mId;
    std::string mName;
    std::string mDescription;
    std::string mUpdateTime;
    bj_layout_emer emer_;
    int ch_en_switch_{ 5 };
    string back_image_;
    std::vector<PartitionDetail> mPartitions;

private:
    static bool cmp_by_value(const PartitionDetail& lhs, const PartitionDetail& rhs) {
      return lhs.mZorder < rhs.mZorder;
    }
};

class LayoutGroupDetail
{
public:
    LayoutGroupDetail();
    ~LayoutGroupDetail();

    int mId;
    std::string mDscrp; // description
    std::string mResolution;
    std::vector<LayoutDetail> mLayoutDetails;
};

class FileInfo
{
public:
    FileInfo();
    ~FileInfo();

    static bool Parse(const char* jsonStr, FileInfo* data);

    std::string mFilePath; // file name with path.
    std::string mName;
    std::string mMD5;
};


/**
 * Below classes all belong to media detail.
 */
class MediaBasic
{
public:
    typedef enum
    {
        Text = 0,
        Image,
        Video,
        Live,
        ArrivalMsg,
        DigitalClock,
        AnalogClock,
        Flash,
        Weather,
        MediaTypeUnknown,
    } MediaType;

    MediaBasic();
    virtual ~MediaBasic();

    int mId;
    MediaType mType;
    std::string mUpdateTime; //format "%Y%M%D %H%M%S"
    int mDuration; // -1 means permanent.Invalid for video.
};

class FontInfo
{
public:
    FontInfo();
    virtual ~FontInfo();

    std::string mName;
    int mSize;
    bool mIsBold;
    bool mIsItalic;
};

class TextInfo
{
public:
    typedef enum
    {
        Left, Center, Right,
    } Align;
    typedef enum
    {
        Static, LeftScroll, RightScroll, UpScroll
    } Effect;
    typedef enum
    {
        VeryFast = 0, Fast = 1, Normal = 2, Slow = 3, VerySlow = 4,
    } Speed;

    TextInfo();
    virtual ~TextInfo();

    std::string mContent;
    std::string mContentEn;
    FontInfo mFont;
    FontInfo mFontEn;
    std::string mForeColor;
    std::string mBackColor;
    std::string mBackImage;
    std::string mBackImageDir;
    Effect mEffect;
    Speed mSpeed;
    int mPixelPerSecond;
    Align mAlign;
};

/**
 * Detail information of a Text.
 * Desc:Protocol-section[2.4.1.1].
 */
class MediaText: public MediaBasic
{
public:
    MediaText();
    virtual ~MediaText();

    TextInfo mParams;
};

class VaribleInfo
{
public:
//    typedef enum
//    {
//        Operational_car_count,
//        Operational_terminals,
//        Operational_arrival_time,
//        Operational_arrival_left_time,
//        Operational_direction,
//    } VaribleType;

    VaribleInfo();
    virtual ~VaribleInfo();

    std::string mTypeStr;
    int mTrainIndex;
    std::string mTimeFormat;
    bool mUseIcon; // used for weather.
    std::string mLanguage; //used for arrimsg
    std::string mDirection;
//private:
//    VaribleType mType;
};

class VarTextInfo
{
public:
    VarTextInfo();
    virtual ~VarTextInfo();

    TextInfo mText;
    std::vector<VaribleInfo> mVaribles;
};

class LabelInfo
{
public:
    LabelInfo();
    virtual ~LabelInfo();

    /**
     * Get rectangle information from mRect.
     * @param x[out]: x position.
     * @param y[out]: y position.
     * @param w[out]: width.
     * @param h[out]: height.
     */
    void GetRectInfo(int& x, int& y, int& w, int& h);

    std::string mRect; // format: "0,0,200,200"
    VarTextInfo mVarText;
};

/**
 * Detail information of a Video.
 * Desc:Protocol-section[2.4.1.5].
 */
class MediaCommon1: public MediaBasic
{
public:
    MediaCommon1();
    virtual ~MediaCommon1();

    std::vector<LabelInfo> mParams;
};

typedef class MediaCommon1 MediaArrivalMsg; //Protocol-section[2.4.1.5]
typedef class MediaCommon1 MediaDigitalClock; //Protocol-section[2.4.1.6]
typedef class MediaCommon1 MediaWeather; //Protocol-section[2.4.1.9]

class MediaCommon2: public MediaBasic
{
public:
    MediaCommon2();
    virtual ~MediaCommon2();

    std::string mFile;
    std::string mUrl; //only for live media
};

typedef class MediaCommon2 MediaAnalogClock; //Protocol-section[2.4.1.7]
typedef class MediaCommon2 MediaFlash; //Protocol-section[2.4.1.8]
typedef class MediaCommon2 MediaLive; // Protocol-section[2.4.1.4]
typedef class MediaCommon2 MediaVideo; // Protocol-section[2.4.1.3]
typedef class MediaCommon2 MediaImage; // Protocol-section[2.4.1.2]


// Layout full information for qt to display.
class LayoutInfo4Qt
{
public:
    // key: media id, value: media content information.
    typedef std::map<int, Json::MediaBasic*> MediaContents; // the map of media the layout needed.

    LayoutDetail layoutDtl;                 // the layout detail.
    // key: partition id, value: media content information map.
    std::map<int,MediaContents> mPartitonInfos;
};


class OPSMsgBasic
{
public:
    OPSMsgBasic();
    virtual ~OPSMsgBasic();

    int mId;
    int mLevelId;
    std::string mLevelName;
    std::string mContent;
    int mStatus;
    std::string mStartTime;
    std::string mEndTime;
    int mPlayCnt;
    std::string mCreateTime;
    std::string mUpdateTime;
};

/**
 * class for operation message list.
 * Desc:Protocol-section[2.7.1]
 */
class OPSMsgList
{
public:
    OPSMsgList();
    virtual ~OPSMsgList();

    static bool Parse(const char* jsonStr, OPSMsgList* data);

    std::vector<OPSMsgBasic> mOPSMsgs;
private:
    static const char *TAG;
};

/**
 * class for operation message detail.
 * Desc:Protocol-section[2.7.2]
 */
class OPSMsgDetail
{
public:
    OPSMsgDetail();
    virtual ~OPSMsgDetail();

    static bool Parse(const char* jsonStr, OPSMsgDetail* data);
    static void Parse(const OpmMsg& msg, OPSMsgDetail* data, int opm_id);

    /**
     * Judge weather the priority is higher than another.
     * @param b
     * @return true: priority is higher than 'b',
     *         false: priority is not higher than 'b'.
     */
    bool PriorityHigherThan(const OPSMsgDetail& b) const;

    /**
     * Judge weather the priority is equal to 'b'.
     * @param b
     * @return true: priority is equal to 'b',
     */
    bool PriorityEqual(const OPSMsgDetail& b) const;

    /**
     * Judge weather the priority is lower than another.
     * @param b
     * @return true: priority is lower than 'b',
     *         false: priority is not lower than 'b'.
     */
    bool PriorityLowerThan(const OPSMsgDetail& b) const;

    OPSMsgBasic mBasic;
    int mPriority { -1 };
    //std::vector<std::string> mObjs;
    //int mQueueFlag;
    int mPlayMode;
    int mDisplayRegion;
    std::string mContent;
    TextInfo mText;
private:
    static const char *TAG;
};

class OPSReply
{
public:
    OPSReply();
    ~OPSReply();

    /**
     * Format the object to json string
     * @param obj[in]: the object to be format.
     * @param jsonStr[in,out]: the formated json string.
     */
    static void ToJson(const OPSReply* obj, std::string& jsonStr);

    int mId;
    std::string mDevice;
    int mStatus;
    std::string mRepTime; // Reply time
};

class TrainInfo
{
public:
    TrainInfo();
    virtual ~TrainInfo();

    std::string mTrainCode;
    int mCarCount;
    bool mHoldFlag;
    bool mSkipFlag;
    bool mIsFirstTrain;
    bool mIsLastTrain;
    int mArrivalStatus; // 0-Arriving 1-Arrived 2-Leaved
    //std::string mArrivalTime;
    int mArrivalTime;
    std::string mDepartureTime;
    std::string mTerminal;
};

/**
 * class for arrival message detail.
 * Desc:Protocol-section[2.8.1]
 */
class ArrivalDetail
{
public:
    ArrivalDetail();
    virtual ~ArrivalDetail();

    static bool Parse(const char* jsonStr, ArrivalDetail* data);

    std::string mStationCode;
    std::string mPlatformCode;
    std::vector<TrainInfo> mTrains;
private:
    static const char *TAG;
};


class TrainTime
{
public:
	TrainTime();
    virtual ~TrainTime();

    typedef enum {
    	UP = 0,
    	Down = 1,
		Unknown = 2,
    } StationUporDown;
    std::string mDirectionCode;
    std::string mFirstTrainTime;
    std::string mLastTrainTime;
    std::string mUpDown;
};

class TrainTimeDetail
{
public:
	TrainTimeDetail();
	virtual ~TrainTimeDetail();

	static bool Parse(const char* jsonStr, TrainTimeDetail* data);

	static bool Load(const std::string file, TrainTimeDetail* data);

	static bool Save(const std::string file, std::string sdata);

	std::string mStationCode;
	std::string mUpdateTime;
	std::vector<TrainTime> mTrainsTime;

private:
	static const char *TAG;
};

class ScreenOnOffTime
{
public:
	ScreenOnOffTime();
    virtual ~ScreenOnOffTime();

    std::string mStationCode;
    std::string mUpdateTime;
    std::string mScreenOnTime;
    std::string mScreenOffTime;
};

class ScreenOnOffDetail
{
public:
	ScreenOnOffDetail();
	virtual ~ScreenOnOffDetail();

	static bool Parse(const char* jsonStr, ScreenOnOffDetail* data);

	std::vector<ScreenOnOffTime> mScreenOnOff;

private:
	static const char *TAG;
};

class LiveSourceSwitch
{
public:
	LiveSourceSwitch();
    virtual ~LiveSourceSwitch();

    std::string mStationCode;
    std::string mUpdateTime;
    int mPlaySource;
};

class LiveSwitchDetail
{
public:
	LiveSwitchDetail();
	virtual ~LiveSwitchDetail();

	static bool Parse(const char* jsonStr, LiveSwitchDetail* data);

	std::vector<LiveSourceSwitch> mLiveSource;

private:
	static const char *TAG;
};

class TemperatureInfo
{
public:
    TemperatureInfo();
    virtual ~TemperatureInfo();

    int mMax;
    int mMin;
    int mCurrent;
};

class WindInfo
{
public:
    WindInfo();
    virtual ~WindInfo();

    int mDirection;
    int mSpeed;
};

/**
 * class for weather message detail.
 * Desc:Protocol-section[2.9.1]
 */
class WeatherDetail
{
public:
    WeatherDetail();
    virtual ~WeatherDetail();

    static bool Parse(const char* jsonStr, WeatherDetail* data);

    TemperatureInfo mTemperature;
    int mAtmosphere;
    WindInfo mWind;
private:
    static const char *TAG;
};

class StatusLog
{
public:
    StatusLog();
    ~StatusLog();

private:
    std::string mDevCode;
    std::string mUpdateTime;
};

/**
 * Get text information from object value.
 * @param obj[in]: object value.
 * @param textInfo[in,out]: the address of a TextInfo which bring out the TextInfo data.
 * @return
 */
bool GetTextInfo(rapidjson::Value& obj, TextInfo* textInfo);

/**
 * Get varible information from object value.
 * @param obj[in]: object value.
 * @param vblInfo[in,out]: the address of a VaribleInfo which bring out the VaribleInfo data.
 * @return
 */
bool GetVaribleInfo(rapidjson::Value& obj, VaribleInfo* vblInfo);

/**
 * Get varible text from object value.
 * @param obj[in]: object value.
 * @param varTextInfo[in,out]: the address of a VarTextInfo which bring out the VarTextInfo data.
 * @return
 */
bool GetVarTextInfo(rapidjson::Value& obj, VarTextInfo* varTextInfo);

/**
 * Get label information from object value.
 * @param obj[in]: object value.
 * @param lblInfo[in,out]: the address of a LabelInfo which bring out the LabelInfo data.
 * @return
 */
bool GetLabelInfo(rapidjson::Value& obj, LabelInfo* lblInfo);

/**
 * Parse the notification code.
 * @param jsonStr
 * @param code
 * @return
 */
bool ParseNtCode(const char* jsonStr, NotifyMessageCode& code);
}

#endif /* SRC_JSON_SCHEDULEOBJS_H_ */
