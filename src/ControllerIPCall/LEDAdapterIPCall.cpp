/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : LEDAdapterIPCall.cpp
 * @author : Benson
 * @date : Dec 12, 2017
 * @brief :
 */
#include "LEDAdapterIPCall.h"
#include <string>

LedDevInfo::LedDevInfo() :
        mDevName(""), mAddr(0), mScreenType(OneScreen), mLedWidth(128), mLedHeight(
                32), mZoneCnt(4)
{

}

int LedDevInfo::writeToParcel(Parcel &out) const
{
    int cnt = 0;

    cnt += out.writeString(mDevName);
    cnt += out.writeInt32(mAddr);
    cnt += out.writeInt32(mScreenType);
    cnt += out.writeInt32(mLedWidth);
    cnt += out.writeInt32(mLedHeight);
    cnt += out.writeInt32(mZoneCnt);
    cnt += out.writeString(mDevicePort);
    cnt += out.writeInt32(mBaudRate);
    cnt += out.writeInt32(mDataBits);
    cnt += out.writeInt32(mStopBits);
    cnt += out.writeInt32(mParity);
    cnt += out.writeInt32(mFlowctrl);

    return cnt;
}

int LedDevInfo::readFromParcel(Parcel &in)
{
    int cnt = 0;

    mDevName = in.readString();
    cnt += mDevName.size();
    cnt += in.readInt32(&mAddr);
    cnt += in.readInt32(&mScreenType);
    cnt += in.readInt32(&mLedWidth);
    cnt += in.readInt32(&mLedHeight);
    cnt += in.readInt32(&mZoneCnt);

    mDevicePort = in.readString();
    cnt += mDevicePort.size();

    cnt += in.readInt32(&mBaudRate);
    cnt += in.readInt32(&mDataBits);
    cnt += in.readInt32(&mStopBits);
    cnt += in.readInt32(&mParity);
    cnt += in.readInt32(&mFlowctrl);

    return cnt;
}
////////////////////////////////////////////////////////////////////////////
///
ZoneInfo::ZoneInfo() :
        zoneId(0), speed(Normal), foreColor("FF0000"), backColor(
                "000000"), effect(Static), dataStr(""), xPos(
                0), yPos(0), width(128), height(64), bTimeZone(false)
{

}

int ZoneInfo::writeToParcel(Parcel &out) const
{
    int cnt = 0;
//        bool bTimeZone; // weather the zone is use to show time.

    cnt += out.writeInt32(zoneId);
    cnt += out.writeInt32(speed);
    cnt += out.writeString(foreColor);
    cnt += out.writeString(backColor);
    cnt += out.writeInt32(effect);
    cnt += out.writeString(dataStr);
    cnt += out.writeInt32(xPos);
    cnt += out.writeInt32(yPos);
    cnt += out.writeInt32(width);
    cnt += out.writeInt32(height);
    cnt += out.writeInt32(bTimeZone);
    return cnt;
}

int ZoneInfo::readFromParcel(Parcel &in)
{
    int cnt = 0;

    cnt += in.readInt32(&zoneId);

    int theVal = 0;
    cnt += in.readInt32(&theVal);
    speed = (ZoneSpeed) theVal;

    foreColor = in.readString();
    cnt += foreColor.size();

    backColor = in.readString();
    cnt += backColor.size();

    cnt += in.readInt32(&theVal);
    effect = (ZoneEffect) theVal;

    dataStr = in.readString();
    cnt += dataStr.size();

    cnt += in.readInt32(&xPos);
    cnt += in.readInt32(&yPos);
    cnt += in.readInt32(&width);
    cnt += in.readInt32(&height);

    cnt += in.readInt32(&theVal);
    bTimeZone = (bool) theVal;

    return cnt;
}
///



////////////////////////////////////////////////////////////////////////////

LEDAdapterIPCall::LEDAdapterIPCall()
{
}


LEDAdapterIPCall::LEDAdapterIPCall(int fd,const std::string &callerName)
:IPCall(fd,callerName)
{

}

LEDAdapterIPCall::~LEDAdapterIPCall()
{
}
