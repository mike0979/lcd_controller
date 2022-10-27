/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : LEDProtocol.h
 * @author : Benson
 * @date : Nov 17, 2017
 * @brief :
 */

#ifndef LEDPLAYER_LEDPROTOCOL_H_
#define LEDPLAYER_LEDPROTOCOL_H_

#include <string>
#include <vector>

#define FR_STX          0x02
#define FR_ETX          0x03
#define FR_ESC          0x1B
#define FR_SPE          0x15
#define FR_ACK          0x00
#define FR_NAK          0x01

// Error code
#define NAK_CHECK           0x00
#define NAK_UNKNOW_COMMAND  0x01
#define NAK_FRAME_LENGTGH   0x02
#define NAK_FRAME_DATA      0x03

#define COMMAND_GET_CONFIG              0x00
#define COMMAND_GET_CLOCKATTR           0x01
#define COMMAND_SET_DAYLIGHTSAVING      0x04
#define COMMAND_SET_TIME                0x06
#define COMMAND_GET_TIME                0x07
#define COMMAND_GET_HASPHOTOCELL        0x08
#define COMMAND_SET_BRIGHT              0x09
#define COMMAND_GET_CURPHOTOCELLLEVEL   0x0A
#define COMMAND_TEST                    0x0f
#define COMMAND_IS_TESTING              0x10
#define COMMAND_RESET                   0x11
#define COMMAND_GET_FONTPARAM           0x12
#define COMMAND_SET_FONTTABLE           0x13
#define COMMAND_READ_FONT               0x14
#define COMMAND_GET_DISPLAYMANAGEPARA   0x18
#define COMMAND_SET_DISPLAYHEADERDATA   0x19
#define COMMAND_SET_DISPLAYDATA         0x1B
#define COMMAND_ACTIVE_MESSAGE          0x1D
#define COMMAND_GET_ACTIVE_MESSAGE      0x1E
#define COMMAND_SET_COLORBASE_ADJUSTMENT 0x2b
#define COMMAND_GET_COLORBASE_ADJUSTMENT 0x2c

#define COMMAND_SET_ZONE                    0x2d
#define COMMAND_GET_ZONE                    0x2e
#define COMMAND_SET_ZONEDISPLAYHEADERDATA   0x2f
#define COMMAND_SET_ZONEDISPLAYDATA         0x31
#define COMMAND_ZONE_ACTIVE_MESSAGE         0x33

#define COMMAND_SET_IP                      0x35
#define COMMAND_GET_IP                      0x36

#define COMMAND_ONOFF                       0x37
#define COMMAND_GET_ONOFF                   0x38

#define COMMAND_SET_ZONEDISPLAYDATA_FLASH   0x39
#define COMMAND_SHOW_FLASH                  0x3b

#define COMMAND_GET_BADPIXELS               0x3e

#define COMMAND_ISP                         0xff


typedef enum
{
    E_Static = 0,
    E_LeftScroll = 46,
    E_RightScroll = 48,
    E_UpScroll = 30,
}VMS_PlayMode;


typedef unsigned char VMS_BYTE;
typedef const unsigned char VMS_BYTE_C;

typedef char VMS_CHAR;
typedef const char VMS_CHAR_C;
typedef int VMS_INT;
typedef unsigned short VMS_WORD;
typedef unsigned int VMS_DWORD;

#define FRAME_PAGE_SIZE 1024
#define BUFFER_SIZE     2048
#define BLOCK_SIZE      512

#define VMS_HI_BYTE(w)    ( (((VMS_WORD)w) >> 8) & 0x00FF )
#define VMS_LO_BYTE(w)    ( ((VMS_WORD)w) & 0x00FF )

#define VMS_HIWORD(dw)      ((((VMS_DWORD)dw) >> 16) & 0x0000FFFF)
#define VMS_LOWORD(dw)      (((VMS_DWORD)dw) & 0x0000FFFF)

namespace VMS
{

typedef struct
{
    int xPos; // left-top x coordinate
    int yPos; // left-top y coordinate
    int width; // the width of single zone
    int height; // the height of single zone
}SingleZoneInfo;

/**
 * Parse the simple reply of VMS LED.
 * @param frameBuf[in]]: the reply frame.
 * @param frameSz[in]: the size of the reply frame.
 * @return true - reply is OK.
 *         false - reply is NAK or wrong format.
 */
bool ParseSimpleReply(VMS_BYTE* frameBuf,VMS_INT frameSz);

/**
 * Format the ping frame to detect weather LED is connected. Use COMMAND_GET_HASPHOTOCELL(0x08) to ping.
 * @param addr: LED address (1-20, 0 is broadcast).
 * @param frameBuf[out]: to store the protocol frame.
 * @param frameSz[out]: the size of frameBuf.
 * @return
 */
bool FormatPingFrame(VMS_BYTE addr, VMS_BYTE* frameBuf, VMS_INT& frameSz);

/**
 * Format the frame to turn on/off LED screen. Func code COMMAND_ONOFF(0x37)
 * @param addr: LED address (1-20, 0 is broadcast).
 * @param on: true -- on, false -- off.
 * @param frameBuf[out]: to store the protocol frame.
 * @param frameSz[out]: the size of frameBuf.
 * @return
 */
bool FormatOnOffControl(VMS_BYTE addr, bool on, VMS_BYTE* frameBuf,
        VMS_INT& frameSz);

/**
 * Format the frame to get LED screen on/off status. Func code COMMAND_GET_ONOFF(0x38)
 * @param addr: LED address (1-20, 0 is broadcast).
 * @param on: true -- on, false -- off.
 * @param frameBuf[out]: to store the protocol frame.
 * @param frameSz[out]: the size of frameBuf.
 * @return
 */
bool FormatOnOffGet(VMS_BYTE addr, VMS_BYTE* frameBuf,VMS_INT& frameSz);

/**
 * Parse the frame to get LED screen on/off status.
 * @param frameBuf
 * @param frameSz
 * @param on[out]: take out the on/off status.
 *                 true -- on; false -- off.
 * @return
 */
bool ParseOnOffStatus(VMS_BYTE_C* frameBuf,VMS_INT frameSz,bool& on);


/**
 * Format the frame to adjust brightness. Func code COMMAND_SET_BRIGHT(0x09)
 * @param addr: LED address (1-20, 0 is broadcast).
 * @param bManual: if true, adjust the brightness of LED according to the param 'brightLevel'.
 *               otherwise,the LED will adjust the brightness automatically.
 * @param brightLevel: the level of brightness.[0,31]
 * @param frameBuf[out]: to store the protocol frame.
 * @param frameSz[out]: the size of frameBuf.
 * @return
 */
bool FormatBrightnessControl(VMS_BYTE addr, bool bManual,VMS_BYTE brightLevel, VMS_BYTE* frameBuf,
        VMS_INT& frameSz);

/**
 * Format the frame to set current time. Func code COMMAND_SET_TIME(0x06)
 * @param addr: LED address (1-20, 0 is broadcast)
 * @param frameBuf[out]: to store the protocol frame.
 * @param frameSz[out]: the size of frameBuf.
 * @return
 */
bool FormatSetCurrentTime(VMS_BYTE addr, VMS_BYTE* frameBuf, VMS_INT& frameSz);

/**
 * Format the frame to set single zone display data. Func code COMMAND_SET_ZONEDISPLAYDATA(0x31)
 * @param addr: LED address (1-20, 0 is broadcast)
 * @param zoneId:[0,7]
 * @param formatString: content format string.
 * @param contentString: should be a utf-8 string!!!!(You may use QTextCodec with utf8 to set QString)
 * @param displayType:See protocol document.(Appendix 2)
 * @param speed:(unit - 0.01s) See protocol document.
 * @param delay:(uint - 0.01s)See protocol document.
 * @param frameBuf[out]: to store the protocol frame.
 * @param frameSz[out]: the size of frameBuf.
 * @return
 */
bool FormatLongStr2Zone(VMS_BYTE addr, VMS_BYTE zoneId,
        const std::string& formatString, const std::string& contentString,
        short displayType, short speed, short delay, VMS_BYTE* frameBuf,
        VMS_INT& frameSz);

/**
 * Format the frame to set display zone information. Func code COMMAND_SET_ZONE(0x2d)
 * @param addr:LED address (1-20, 0 is broadcast)
 * @param zoneCnt: the count of zone.(should not large than 4 zone at present)
 * @param width: single zone width.
 * @param height: single zone height.
 * @param frameBuf[out]: to store the protocol frame.
 * @param frameSz[out]: the size of frameBuf.
 * @return
 * Zone direction
 *  |     -------------------------
 *  |       Zone 0
 *  |     -------------------------
 *  |       Zone 1
 * \  /   -------------------------
 *  \/      Zone n
 *        -------------------------
 */
//bool FormatDisplayZone(VMS_BYTE addr, VMS_BYTE zoneCnt, VMS_WORD width,
//        VMS_WORD height, VMS_BYTE* frameBuf, VMS_INT& frameSz);

/**
 * Format the frame to set display zone information. Func code COMMAND_SET_ZONE(0x2d)
 * @param addr:LED address (1-20, 0 is broadcast)
 * @param zoneInfos: vector of zone information.(no more than 8 zone)
 * @param frameBuf[out]: to store the protocol frame.
 * @param frameSz[out]: the size of frameBuf.
 * @return
 */
bool FormatDisplayZone(VMS_BYTE addr,std::vector<SingleZoneInfo>zoneInfos, VMS_BYTE* frameBuf, VMS_INT& frameSz);

/**
 * Format the frame to set the header of a single zone.Func code COMMAND_SET_ZONEDISPLAYHEADERDATA(0x2f)
 * @param addr:LED address (1-20, 0 is broadcast)
 * @param zoneId:[0,3]
 * @param width: VMS total width
 * @param height: VMS total height(Not single one)
 * @param frameBuf[out]: to store the protocol frame.
 * @param frameSz[out]: the size of frameBuf.
 * @return
 */
bool FormatZoneDataHeader(VMS_BYTE addr, VMS_BYTE zoneId, VMS_WORD width,
        VMS_WORD height, VMS_BYTE* frameBuf, VMS_INT& frameSz);

/**
 * Calculate the data string center align start x position.
 * @param screenWidth: the total width of screen.
 * @param perCharWidth: the width of a single character.
 * @param contentString: the string to be center display.
 * @param startPos[out]: the x coordinate of first character.
 */
void GetCenterStartPos(VMS_WORD screenWidth,VMS_BYTE perCharWidth,const std::string& contentString,VMS_WORD& startPos);

/**
 * Calculate the data string total width.
 * @param perCharWidth: the width of each char.
 * @param contentString: the string to be calculated.
 * @return the total width of the contentString.
 */
VMS_WORD GetStringWidth(VMS_BYTE perCharWidth,const std::string& contentString);
}
;

#endif /* LEDPLAYER_LEDPROTOCOL_H_ */
