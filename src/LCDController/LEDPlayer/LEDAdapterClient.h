/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : LEDAdapterClient.h
 * @author : Benson
 * @date : Dec 12, 2017
 * @brief :
 */

#ifndef LEDPLAYER_LEDADAPTERCLIENT_H_
#define LEDPLAYER_LEDADAPTERCLIENT_H_

#include <LEDAdapterIPCall.h>
#include <LEDPlayer/LEDPlayer.h>
#include <map>
#include <vector>

class Handler;

class LEDAdapterClient: public LEDAdapterIPCall
{
public:
    LEDAdapterClient(Handler* handler);
    ~LEDAdapterClient();

    bool SetDeviceInfoReq(const std::vector<LedDevInfo>& devInfos);

    /**
     * Send Control screen on/off request to Adapter server.
     * @param addr: address of screen.
     * @param bOn: true -- on
     *             false -- off
     */
    bool ScreenOnOffReq(int cmdId,int addr, bool bOn);
    bool SetBrightNessReq(int cmdId, int addr,int val);
    bool ClearScreenReq(int addr);
    bool SetSchContentReq(int addr,const std::map<int,ZoneInfo>& zoneMaps);
    bool SetOPSReq(int addr,const std::map<int,ZoneInfo>& zoneMaps);
    bool DateTimeSyncReq(int addr);
    bool GetScreenStatusReq(int addr);

private:
    virtual void disconnected();
    virtual bool answer(Parcel &parcel); // here we got the data from Adapter server.
private:
    Handler* mHandler; // the LEDPlayer!
    static const char *TAG;
};

#endif /* LEDPLAYER_LEDADAPTERCLIENT_H_ */
