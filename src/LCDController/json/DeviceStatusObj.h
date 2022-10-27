/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : DeviceStatusObj.h
 * @author : Benson
 * @date : Nov 11, 2017
 * @brief :
 */

#ifndef JSON_DEVICESTATUSOBJ_H_
#define JSON_DEVICESTATUSOBJ_H_

#include <vector>
#include <string>

namespace Json
{

struct HddInfo
{
    double mHddRatio;
    int mHddSize;
};

struct MemoryInfo
{
    double mMemoryRatio;
    int mMemorySize;
};

class HardwareStatus
{
public:
    typedef enum
    {
        S_ON = 0, S_OFF = 1, S_OFF_LINE = 2
    } HdStatus;

    HardwareStatus();
    ~HardwareStatus();

    HdStatus mStatus;
    double mCpuRadio;

    std::vector<HddInfo> mHddInfoVec;

    MemoryInfo mMemoryInfo;
private:
	static const char *TAG;
};

class SoftwareStatus
{
public:
    typedef enum
    {
        S_Normal = 0, S_NotRunning = 1, S_Abnormal = 2,
    } SftStatus;

    typedef enum
    {
        M_Live = 0, M_Local = 1,
    } WorkMode;

    SoftwareStatus();
    ~SoftwareStatus();

    /**
     * Get local device software status.
     * include:
     *          1. this software used cpu ration.
     *          2. this software used memory ration.
     * @param[out]
     */
    static bool GetBasicStatus(const std::string& processName,
            SoftwareStatus& basicInfo);

    std::string mSysVersion;
    SftStatus mStatus;
    WorkMode mMode;
    double mCpuRatio;

    HddInfo mHddInfo;

    MemoryInfo mMemoryInfo;

private:
	static const char *TAG;
};

class DeviceStatus
{
public:
    DeviceStatus();
    ~DeviceStatus();

    /**
     * Format the device status to json string.
     * @param status
     * @param[out] jsonStr - the formated json string.
     * @return true - success.
     *         false - failed.
     */
    static bool ToJson(const DeviceStatus& status,std::string& jsonStr);

    HardwareStatus mHardware;
    SoftwareStatus mSoftware;

private:
	static const char *TAG;
};

}

#endif /* JSON_DEVICESTATUSOBJ_H_ */
