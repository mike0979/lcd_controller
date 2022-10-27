/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : LEDAdapterClient.cpp
 * @author : Benson
 * @date : Dec 12, 2017
 * @brief :
 */

#include <LEDPlayer/LEDAdapterClient.h>
#include <Log.h>
#include <Message.h>
#include <Parcel.h>
#include <cstdint>

const char *LEDAdapterClient::TAG = "LEDAdapterClient";

LEDAdapterClient::LEDAdapterClient(Handler* handler) :
        mHandler(handler)
{
}

LEDAdapterClient::~LEDAdapterClient()
{
}

void LEDAdapterClient::disconnected()
{
    mHandler->sendMessage(new Message(LEDPlayer::LED_Adapter_DisConnect));
}

bool LEDAdapterClient::answer(Parcel &parcel)
{
    //LogD("\t\t --------- LEDAdapterClient::answer!\n");

    int32_t callid;
    parcel.readInt32(&callid);
    switch (callid)
    {
    case ID_SetDeviceInfoRply:
    {
        // rslt value.
        int32_t openRslt;
        parcel.readInt32(&openRslt);

        LogD("ID_SetDeviceInfoRply = %d\n", openRslt);

        break;
    }
    case ID_ScreenOnCtrlRply:
    case ID_ScreenOffCtrlRply:
    case ID_SetBrightNessRply:
    {
        int32_t cmdId, devAddr, cmdRslt;

        parcel.readInt32(&cmdId);
        parcel.readInt32(&devAddr);
        parcel.readInt32(&cmdRslt);

        LogD("IPCALL rplyID[%d] cmd result = %d\n", callid, cmdRslt);

        LEDMsgData* msgData = new LEDMsgData();
        msgData->mCmdId = cmdId;
        msgData->mAddr = devAddr;
        msgData->mCmdExeRslt = (LEDAdapterIPCall::ExecuteRslt) cmdRslt;

        mHandler->sendMessage(
                new Message(LEDPlayer::LED_CMD_AdapterReply, msgData));

        break;
    }

    case ID_SetOPSRply:
    {
        int32_t devAddr, setOpsRslt;

        parcel.readInt32(&devAddr);
        parcel.readInt32(&setOpsRslt);

        // TODO: ignore current now.
//        mHandler->sendMessage(
//                        new Message(LEDPlayer::LED_OPS_AdapterReply, devAddr,setOpsRslt));
        break;
    }
    case ID_GetScreenStatusRply:
    {
        int32_t devAddr, screenStatus;
        parcel.readInt32(&devAddr);
        parcel.readInt32(&screenStatus);

        mHandler->sendMessage(
                new Message(LEDPlayer::LED_ScreenStatus_AdapterReply, devAddr,
                        screenStatus));
        break;
    }
    default:
        break;
    }

    return true;
}

bool LEDAdapterClient::SetDeviceInfoReq(const std::vector<LedDevInfo>& devInfos)
{
    Parcel parcel;
    parcel.writeInt32(ID_SetDeviceInfoReq);
    parcel.writeInt32(devInfos.size());
    for (int i = 0; i < devInfos.size(); ++i)
    {
        devInfos[i].writeToParcel(parcel);
    }

    return call(parcel);
}

bool LEDAdapterClient::ScreenOnOffReq(int cmdId, int addr, bool bOn)
{
    Parcel parcel;

    if (bOn)
        parcel.writeInt32(ID_ScreenOnReq);
    else
        parcel.writeInt32(ID_ScreenOffReq);
    parcel.writeInt32(cmdId);
    parcel.writeInt32(addr);
    parcel.writeInt32(bOn);
    return call(parcel);
}

bool LEDAdapterClient::SetBrightNessReq(int cmdId, int addr, int val)
{
    Parcel parcel;
    parcel.writeInt32(ID_SetBrightNessReq);
    parcel.writeInt32(cmdId);
    parcel.writeInt32(addr);
    parcel.writeInt32(val);
    return call(parcel);
}

bool LEDAdapterClient::DateTimeSyncReq(int addr)
{
    Parcel parcel;
    parcel.writeInt32(ID_DateTimeSyncReq);
    parcel.writeInt32(addr);
    return call(parcel);
}

bool LEDAdapterClient::GetScreenStatusReq(int addr)
{
    Parcel parcel;
    parcel.writeInt32(ID_GetScreenStatusReq);
    parcel.writeInt32(addr);
    return call(parcel);
}

bool LEDAdapterClient::ClearScreenReq(int addr)
{
    Parcel parcel;
    parcel.writeInt32(ID_ClearScreenReq);
    parcel.writeInt32(addr);
    return call(parcel);
}

bool LEDAdapterClient::SetSchContentReq(int addr,
        const std::map<int, ZoneInfo>& zoneMaps)
{
    Parcel parcel;

    parcel.writeInt32(ID_SetScheduleReq);
    parcel.writeInt32(addr);

    int zonCnt = zoneMaps.size();
    parcel.writeInt32(zonCnt);

    std::map<int, ZoneInfo>::const_iterator itor = zoneMaps.begin();
    for (; itor != zoneMaps.end(); ++itor)
        itor->second.writeToParcel(parcel);

    return call(parcel);
}

bool LEDAdapterClient::SetOPSReq(int addr,
        const std::map<int, ZoneInfo>& zoneMaps)
{
    Parcel parcel;

    parcel.writeInt32(ID_SetOPSReq);
    parcel.writeInt32(addr);

    int zonCnt = zoneMaps.size();
    parcel.writeInt32(zonCnt);

    std::map<int, ZoneInfo>::const_iterator itor = zoneMaps.begin();
    for (; itor != zoneMaps.end(); ++itor)
        itor->second.writeToParcel(parcel);

    return call(parcel);
}

