/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : LEDProtocol.cpp
 * @author : Benson
 * @date : Nov 17, 2017
 * @brief :
 */

#include "ChineseConvert.h"
#include "LEDProtocol.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qstring.h>
#include <SystemClock.h>
#include <cstdio>
#include <cstring>

//#define CONTENT_SIZE 210 //100 bytes
#define CONTENT_SIZE 254 //127 chinese characters, 254 bytes

namespace VMS
{

//static const VMS_DWORD g_OffsetStep = 0x100000;
static const VMS_DWORD g_OffsetStep = 0x010000;

//static const VMS_DWORD g_SchTableOffset = 0x400000; // schedule table offset.（播放计划）
//static const VMS_DWORD g_MsgTableOffset = 0x400028; // message table offset（播放表）
//static const VMS_DWORD g_TxtTableOffset = 0x400076; // text table offset (正文)
//static const VMS_DWORD g_BitMapTableOffset = 0x4001C2; // bitmap table offset（位图）

static const VMS_DWORD g_SchTableOffset = 0x100000; // schedule table offset.（播放计划）  when only use one zone
static const VMS_DWORD g_MsgTableOffset = 0x100028; // message table offset（播放表）
static const VMS_DWORD g_TxtTableOffset = 0x100076; // text table offset (正文)
//static const VMS_DWORD g_BitMapTableOffset = 0x1000FF; //support 50中文 bitmap table offset（位图）含有中文,若字符串显示不够则适当的增加该偏移，一个中文字符2字节！！！
static const VMS_DWORD g_BitMapTableOffset = 0x10018D; //support 127中文 bitmap table offset（位图）含有中文,若字符串显示不够则适当的增加该偏移，一个中文字符2字节！！！

static VMS_WORD crc_table[256] =
{ 0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108,
        0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 0x1231, 0x0210,
        0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B,
        0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 0x2462, 0x3443, 0x0420, 0x1401,
        0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE,
        0xF5CF, 0xC5AC, 0xD58D, 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6,
        0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D,
        0xC7BC, 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 0x5AF5,
        0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC,
        0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 0x6CA6, 0x7C87, 0x4CE4,
        0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD,
        0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13,
        0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A,
        0x9F59, 0x8F78, 0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E,
        0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1,
        0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, 0xB5EA, 0xA5CB,
        0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0,
        0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xA7DB, 0xB7FA, 0x8799, 0x97B8,
        0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657,
        0x7676, 0x4615, 0x5634, 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9,
        0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882,
        0x28A3, 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
        0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 0xFD2E,
        0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07,
        0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, 0xEF1F, 0xFF3E, 0xCF5D,
        0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74,
        0x2E93, 0x3EB2, 0x0ED1, 0x1EF0 };

void ShowContent(VMS_BYTE_C *p, VMS_INT size)
{
    for (int i = 0; i < size; i++)
        printf("%02x ", p[i]);

    printf("\n");
}

VMS_WORD Gen_crc(VMS_BYTE_C *buffer, VMS_INT buffer_length)
{
    VMS_WORD crc = 0;
    VMS_INT i;

    for (i = 0; i < buffer_length; i++)
        crc = crc_table[((crc >> 8) ^ buffer[i]) & 0xFF] ^ (crc << 8);

    return crc;
}

// convert 0x02,0x03,0x15,0x1B
bool SpecialConvert(VMS_BYTE_C* src, VMS_INT srcCnt, VMS_BYTE* rslt,
        VMS_INT& rlstCnt)
{
    if (NULL == src || NULL == rslt || srcCnt < 2)
        return false;

    int maxIndex = srcCnt - 1;

    VMS_INT index = 0;
    for (VMS_INT i = 0; i < srcCnt; ++i)
    {
        if (FR_STX == src[i])
        {
            if (0 != i)
            {
                rslt[index++] = 0x1B;
                rslt[index++] = 0xE7;
            } else
                rslt[index++] = src[i]; // the frame start.
        } else if (FR_ETX == src[i])
        {
            if (i != maxIndex)
            {
                rslt[index++] = 0x1B;
                rslt[index++] = 0xE8;
            } else
                rslt[index++] = src[i]; // the frame end.

        } else if (FR_SPE == src[i])
        {
            rslt[index++] = 0x1B;
            rslt[index++] = 0xFA;
        } else if (FR_ESC == src[i])
        {
            rslt[index++] = 0x1B;
            rslt[index++] = 0x00;
        } else
        {
            rslt[index++] = src[i];
        }
    } // end for()

    rlstCnt = index;
    return true;
}

bool SpecialConverBack(VMS_BYTE_C* src, VMS_INT srcCnt, VMS_BYTE* rslt,
        VMS_INT& rlstCnt)
{
    if (NULL == src || NULL == rslt || srcCnt < 2)
        return false;

    int index = 0;
    int i = 0;

    do
    {
        if (FR_ESC == src[i])
        {
            VMS_WORD val = src[i] + src[i + 1];
            rslt[index++] = VMS_LO_BYTE(val);
            i += 2;
        } else
        {
            rslt[index++] = src[i];
            ++i;
        }
    } while (i < srcCnt);

    rlstCnt = index;
    return true;
}

bool Str2ASCII(VMS_CHAR_C* srcStr, VMS_INT srcSize, VMS_BYTE* rsltStr)
{
    if (NULL == srcStr || NULL == rsltStr)
        return false;

    for (VMS_INT i = 0; i < srcSize; ++i)
    {
        rsltStr[i] = (VMS_BYTE) srcStr[i];
    }

    return true;
}

bool Int2ASCII(VMS_INT val, VMS_INT width, VMS_BYTE* rsltStr)
{
    if (NULL == rsltStr)
        return false;

    VMS_CHAR strBuf[64] =
    { 0 };
    VMS_CHAR formatBuf[16] =
    { 0 };
    snprintf(formatBuf, sizeof(formatBuf), "%%%d%dd", 0, width);
    snprintf(strBuf, sizeof(strBuf), formatBuf, val);

    return Str2ASCII(strBuf, strlen(strBuf), rsltStr);
}

bool ParseSimpleReply(VMS_BYTE* frameBuf, VMS_INT frameSz)
{
    if (NULL == frameBuf || frameSz < 3)
        return false;

    bool ret = false;

    if (FR_STX == frameBuf[0])
        ret = true;
    else if (FR_SPE == frameBuf[0])
        ret = false;

    return ret;
}

bool FormatPingFrame(VMS_BYTE addr, VMS_BYTE* frameBuf, VMS_INT& frameSz)
{
    if (NULL == frameBuf)
        return false;

    VMS_BYTE tempBuf[FRAME_PAGE_SIZE] =
    { 0 };

    VMS_INT index = 0;
    // head
    tempBuf[index++] = FR_STX;

    tempBuf[index++] = addr;
    tempBuf[index++] = COMMAND_GET_HASPHOTOCELL;

    // crc
    VMS_WORD crcVal = Gen_crc(tempBuf + 1, index - 1);
    tempBuf[index++] = VMS_HI_BYTE(crcVal);
    tempBuf[index++] = VMS_LO_BYTE(crcVal);

    // 03
    tempBuf[index++] = FR_ETX;

    // convert 0x02,0x03,0x15,0x1B ,except frame start and frame end.
    return SpecialConvert(tempBuf, index, frameBuf, frameSz);
}

bool FormatOnOffControl(VMS_BYTE addr, bool on, VMS_BYTE* frameBuf,
        VMS_INT& frameSz)
{
    if (NULL == frameBuf)
        return false;

    VMS_BYTE tempBuf[FRAME_PAGE_SIZE] =
    { 0 };

    VMS_INT index = 0;
    // head
    tempBuf[index++] = FR_STX;

    tempBuf[index++] = addr;
    tempBuf[index++] = COMMAND_ONOFF;

    tempBuf[index++] = (VMS_BYTE) on;

    // crc
    VMS_WORD crcVal = Gen_crc(tempBuf + 1, index - 1);
    tempBuf[index++] = VMS_HI_BYTE(crcVal);
    tempBuf[index++] = VMS_LO_BYTE(crcVal);

    // 03
    tempBuf[index++] = FR_ETX;

    // convert 0x02,0x03,0x15,0x1B ,except frame start and frame end.
    return SpecialConvert(tempBuf, index, frameBuf, frameSz);
}

bool FormatOnOffGet(VMS_BYTE addr, VMS_BYTE* frameBuf, VMS_INT& frameSz)
{
    if (NULL == frameBuf)
        return false;

    VMS_BYTE tempBuf[32] =
    { 0 };

    VMS_INT index = 0;
    // head
    tempBuf[index++] = FR_STX;

    tempBuf[index++] = addr;
    tempBuf[index++] = COMMAND_GET_ONOFF;

    // crc
    VMS_WORD crcVal = Gen_crc(tempBuf + 1, index - 1);
    tempBuf[index++] = VMS_HI_BYTE(crcVal);
    tempBuf[index++] = VMS_LO_BYTE(crcVal);

    // 03
    tempBuf[index++] = FR_ETX;

    // convert 0x02,0x03,0x15,0x1B ,except frame start and frame end.
    return SpecialConvert(tempBuf, index, frameBuf, frameSz);
}

bool ParseOnOffStatus(VMS_BYTE_C* frameBuf, VMS_INT frameSz, bool& on)
{
    if (NULL == frameBuf || frameSz < 7) // this frame at least 6 bytes.
        return false;

    VMS_BYTE* rsltBuf = (VMS_BYTE*)alloca(frameSz * 2);
    VMS_INT rsltCnt = 0;

    if( !SpecialConverBack(frameBuf,frameSz,rsltBuf,rsltCnt) )
        return false;

#if 0
    printf("\t\t ####### SpecialConverBack rslt :\n");
    ShowContent(rsltBuf, rsltCnt);
#endif
    if(rsltCnt != 7 ) // after convert this frame should be 7 bytes.
        return false;

    on = rsltBuf[3]; // on/off status index is 3

    return true;
}

bool FormatBrightnessControl(VMS_BYTE addr, bool bManual, VMS_BYTE brightLevel,
        VMS_BYTE* frameBuf, VMS_INT& frameSz)
{
    if (NULL == frameBuf)
        return false;

    VMS_BYTE tempBuf[FRAME_PAGE_SIZE] =
    { 0 };

    VMS_INT index = 0;
    // head
    tempBuf[index++] = FR_STX;

    tempBuf[index++] = addr;
    tempBuf[index++] = COMMAND_SET_BRIGHT;

    tempBuf[index++] = (VMS_BYTE) bManual;

    tempBuf[index++] = brightLevel > 31 ? 31 : brightLevel;

    // crc
    VMS_WORD crcVal = Gen_crc(tempBuf + 1, index - 1);
    tempBuf[index++] = VMS_HI_BYTE(crcVal);
    tempBuf[index++] = VMS_LO_BYTE(crcVal);

    // 03
    tempBuf[index++] = FR_ETX;

    // convert 0x02,0x03,0x15,0x1B ,except frame start and frame end.
    return SpecialConvert(tempBuf, index, frameBuf, frameSz);
}

bool FormatSetCurrentTime(VMS_BYTE addr, VMS_BYTE* frameBuf, VMS_INT& frameSz)
{
    if (NULL == frameBuf)
        return false;

    VMS_BYTE tempBuf[FRAME_PAGE_SIZE] =
    { 0 };
    VMS_INT index = 0;

    // head
    tempBuf[index++] = FR_STX;

    tempBuf[index++] = addr;
    tempBuf[index++] = COMMAND_SET_TIME;

    std::string timeStr = SystemClock::Today(SystemClockTMFormatNoSpace);
    SystemClockTM clkTm;
    SystemClock::StrToTM(timeStr, clkTm, SystemClockTMFormatNoSpace);

    // current year 2
    tempBuf[index++] = VMS_HI_BYTE(clkTm.mYear); // start date, year.
    tempBuf[index++] = VMS_LO_BYTE(clkTm.mYear);
    // current month 1
    tempBuf[index++] = clkTm.mMon;
    // current day of week 1
    tempBuf[index++] = 1; // TODO: to be fixed!!!
    // current day 1
    tempBuf[index++] = clkTm.mDay;

    // hour
    tempBuf[index++] = clkTm.mHour; // start time, hour
    // minute
    tempBuf[index++] = clkTm.mMin; // min
    // second
    tempBuf[index++] = clkTm.mSec; // sec

    // milli second 2
    tempBuf[index++] = 0;
    tempBuf[index++] = 0;

    // crc
    VMS_WORD crcVal = Gen_crc(tempBuf + 1, index - 1);
    tempBuf[index++] = VMS_HI_BYTE(crcVal);
    tempBuf[index++] = VMS_LO_BYTE(crcVal);

    // 03
    tempBuf[index++] = FR_ETX;

    //ShowContent(tempBuf, index);
    // convert 0x02,0x03,0x15,0x1B ,except frame start and frame end.
    return SpecialConvert(tempBuf, index, frameBuf, frameSz);
}

/**
 * Set single zone display data.
 * @param addr: LCD address (1-20, 0 is broadcast)
 * @param zoneId:[0,7]
 * @param formatString: content format string.
 * @param contentString: should be a utf-8 string!!!!
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
        VMS_INT& frameSz)
{
    if (NULL == frameBuf || zoneId > 7) /*zone id start from 0*/
        return false;

    VMS_BYTE tempBuf[FRAME_PAGE_SIZE] =
    { 0 };
    VMS_INT index = 0;

    // head
    tempBuf[index++] = FR_STX;

    tempBuf[index++] = addr;
    tempBuf[index++] = COMMAND_SET_ZONEDISPLAYDATA;

    // frame
    tempBuf[index++] = zoneId; // zone id

    // serial Number !!!!!!!!!!  (g_SchTableOffset + g_OffsetStep)/1024
    VMS_WORD serialNumber = (g_SchTableOffset + zoneId * g_OffsetStep) / 1024;
    tempBuf[index++] = VMS_HI_BYTE(serialNumber);
    tempBuf[index++] = VMS_LO_BYTE(serialNumber);

    /*************************************************
     *          Play Schedule data
     * **********************************************/
    tempBuf[index++] = 1; //schedule to display
    index += 7; // 7 byte reserved.

    VMS_BYTE schName[8] =
    { 0x31, 0x30, 0x30, 0x39, 0x20, 0x20, 0x20, 0x20 };
    schName[3] += zoneId;

    memcpy(tempBuf + index, schName, 8); // schedule name"    "
    index += 8;

    std::string timeStr = SystemClock::Today(SystemClockTMFormatNoSpace);
    SystemClockTM clkTm;
    SystemClock::StrToTM(timeStr, clkTm, SystemClockTMFormatNoSpace);

    tempBuf[index++] = VMS_HI_BYTE(clkTm.mYear); // start date, year.
    tempBuf[index++] = VMS_LO_BYTE(clkTm.mYear);
    tempBuf[index++] = clkTm.mMon; // start month
    tempBuf[index++] = clkTm.mDay; // start day

    int endYear = clkTm.mYear + 20; // end date
    tempBuf[index++] = VMS_HI_BYTE(endYear); // end year.
    tempBuf[index++] = VMS_LO_BYTE(endYear);
    tempBuf[index++] = clkTm.mMon; // start month
    tempBuf[index++] = clkTm.mDay; // start day

    tempBuf[index++] = 0; // start time, hour
    tempBuf[index++] = 0; // min
    tempBuf[index++] = 0; // sec

    tempBuf[index++] = 0x17; // end time
    tempBuf[index++] = 0x3B;
    tempBuf[index++] = 0x3B;

    tempBuf[index++] = 0xFF; // week bit
    tempBuf[index++] = 0; // sheet index.

    memset(tempBuf + index, 0, 8); // reserved 8 byte
    index += 8;

    /*************************************************
     *          message table data
     * **********************************************/
    tempBuf[index++] = 1; // message table count
    memset(tempBuf + index, 0, 7); // reserved 7 byte
    index += 7;

    VMS_DWORD sheetOffset = g_MsgTableOffset + zoneId * g_OffsetStep + 8 + 16;
    VMS_WORD hiWord = VMS_HIWORD(sheetOffset);
    VMS_WORD loWord = VMS_LOWORD(sheetOffset);
    tempBuf[index++] = VMS_LO_BYTE(hiWord);  // message table offset
    tempBuf[index++] = VMS_HI_BYTE(loWord);
    tempBuf[index++] = VMS_LO_BYTE(loWord);

    //message table size
    VMS_WORD scheduleSheetSz = 2 + 3 + 5 + formatString.size() + 2; //displaytype(2)+speed(3)+delay(5)+formatString.size+"\p"(2)
    tempBuf[index++] = VMS_HI_BYTE(scheduleSheetSz);
    tempBuf[index++] = VMS_LO_BYTE(scheduleSheetSz);

    memcpy(tempBuf + index, schName, 8); // message table name "1000    "
    index += 8;

    tempBuf[index++] = 0x64; // message table priority
    memset(tempBuf + index, 0, 2); // reserved 2 bytes
    index += 2;

    // message table content
    if (displayType > 55)
        return false;

    VMS_BYTE dsplyBuf[2] =
    { 0 };
    Int2ASCII(displayType, 2, dsplyBuf);
    tempBuf[index++] = dsplyBuf[0]; // displayType
    tempBuf[index++] = dsplyBuf[1];

    if (speed > 999)
        return false;
    VMS_BYTE speedBuf[3] =
    { 0 };
    Int2ASCII(speed, 3, speedBuf);
    tempBuf[index++] = speedBuf[0]; // speed
    tempBuf[index++] = speedBuf[1];
    tempBuf[index++] = speedBuf[2];

    VMS_BYTE delayBuf[5] =
    { 0 };
    Int2ASCII(delay, 5, delayBuf);
    for (VMS_BYTE i = 0; i < 5; ++i)
    {
        tempBuf[index++] = delayBuf[i]; // delay time
    }

    if (formatString.size() > FRAME_PAGE_SIZE)
        return false;

    for (VMS_BYTE i = 0; i < formatString.size(); ++i)
    {
        tempBuf[index++] = formatString[i];
    }

    // "\p"
    tempBuf[index++] = 0x5C; // '\'
    tempBuf[index++] = 0x70; // 'p'

    /*************************************************
     *          text table data
     * **********************************************/
    tempBuf[index++] = 1; // content count.
    memset(tempBuf + index, 0, 7); // reserved 7 byte
    index += 7;

    VMS_DWORD textOffset = g_TxtTableOffset + zoneId * g_OffsetStep + 8 + 16;
    hiWord = VMS_HIWORD(textOffset);
    loWord = VMS_LOWORD(textOffset);
    tempBuf[index++] = VMS_LO_BYTE(hiWord);  // text table offset
    tempBuf[index++] = VMS_HI_BYTE(loWord);
    tempBuf[index++] = VMS_LO_BYTE(loWord);

    QString qstr = QString::fromStdString(contentString);
    QByteArray valArr = UTF82GBK(qstr); // content data.
    VMS_WORD cntSz =
            valArr.size() > CONTENT_SIZE ? CONTENT_SIZE : valArr.size();

    tempBuf[index++] = VMS_HI_BYTE(cntSz); // content size
    tempBuf[index++] = VMS_LO_BYTE(cntSz);

    VMS_BYTE contentName[8] =
    { 0x32, 0x32, 0x33, 0x20, 0x20, 0x20, 0x20, 0x20 };
    memcpy(tempBuf + index, contentName, 8); // content name "223    "
    index += 8;

    memset(tempBuf + index, 0, 3); // reserved 3 bytes
    index += 3;

    // content data
    memcpy(tempBuf + index, valArr.data(), cntSz);
    index += cntSz;

    /*************************************************
     *          Bitmap data
     * **********************************************/
    tempBuf[index++] = 0;  // bitmap count
    memset(tempBuf + index, 0, 7); // reserved 7 bytes
    index += 7;

//    VMS_DWORD bitMapOffset = g_BitMapTableOffset + 8 + 16;
//    hiWord = VMS_HIWORD(bitMapOffset);
//    loWord = VMS_LOWORD(bitMapOffset);
//    tempBuf[index++] = VMS_LO_BYTE(hiWord);  // bitmap0 offset
//    tempBuf[index++] = VMS_HI_BYTE(loWord);
//    tempBuf[index++] = VMS_LO_BYTE(loWord);
//
//    memcpy(tempBuf + index, contentName, 8); // bitmap table name "223    "
//    index += 8;
//
//    memset(tempBuf + index, 0, 4); // width,height
//    index += 4;
//
//    tempBuf[index++] = 0; // reserved 1 byte.

    // crc
    VMS_WORD crcVal = Gen_crc(tempBuf + 1, index - 1);
    tempBuf[index++] = VMS_HI_BYTE(crcVal);
    tempBuf[index++] = VMS_LO_BYTE(crcVal);

    // 03
    tempBuf[index++] = FR_ETX;

//    ShowContent(tempBuf, index);
    // convert 0x02,0x03,0x15,0x1B ,except frame start and frame end.
    return SpecialConvert(tempBuf, index, frameBuf, frameSz);
}

#if 0
// zoneIndex start from 0.return byte size.
static int SetZoneInfo(int zoneIndex, VMS_WORD width, VMS_WORD height,
        VMS_BYTE* tempBuf)
{
    if (NULL == tempBuf)
    return 0;

    int index = 0;

    VMS_BYTE zoneName[8] =
    {   0x31, 0x30, 0x31, 0x39};    // zone name
    zoneName[3] += zoneIndex;

    memcpy(tempBuf + index, zoneName, 8);
    index += 8;

    VMS_WORD z1x = 0;// Zone  x coordinate
    tempBuf[index++] = VMS_HI_BYTE(z1x);
    tempBuf[index++] = VMS_LO_BYTE(z1x);

    VMS_WORD z1y = zoneIndex * height;// Zone  y coordinate(so.. zone0.y:0;zone1.y:height;zone2.y:2*height)
    tempBuf[index++] = VMS_HI_BYTE(z1y);
    tempBuf[index++] = VMS_LO_BYTE(z1y);

    tempBuf[index++] = VMS_HI_BYTE(width);// Zone 1 width
    tempBuf[index++] = VMS_LO_BYTE(width);

    tempBuf[index++] = VMS_HI_BYTE(height);// Zone 1 height
    tempBuf[index++] = VMS_LO_BYTE(height);

    tempBuf[index++] = 0;// Zone 1 be transparent
    memset(tempBuf + index, 0, 7);// reserved 7 bytes
    index += 7;

    return index;
}
#endif

static int SetZoneInfo(std::vector<SingleZoneInfo> zoneInfos, VMS_BYTE* tempBuf)
{
    if (NULL == tempBuf)
        return 0;

    int index = 0;
    VMS_BYTE zoneName[8] =
    { 0x31, 0x30, 0x31, 0x30 };

    for (VMS_BYTE i = 0; i < zoneInfos.size(); ++i)
    {
        zoneName[3] += i;
        memcpy(tempBuf + index, zoneName, 8); // zone name
        index += 8;

        tempBuf[index++] = VMS_HI_BYTE(zoneInfos[i].xPos); // Zone  x coordinate
        tempBuf[index++] = VMS_LO_BYTE(zoneInfos[i].xPos);

        tempBuf[index++] = VMS_HI_BYTE(zoneInfos[i].yPos);
        tempBuf[index++] = VMS_LO_BYTE(zoneInfos[i].yPos);

        tempBuf[index++] = VMS_HI_BYTE(zoneInfos[i].width); // Zone width
        tempBuf[index++] = VMS_LO_BYTE(zoneInfos[i].width);

        tempBuf[index++] = VMS_HI_BYTE(zoneInfos[i].height); // Zone height
        tempBuf[index++] = VMS_LO_BYTE(zoneInfos[i].height);

        tempBuf[index++] = 0; // Zone  be transparent
        memset(tempBuf + index, 0, 7); // reserved 7 bytes
        index += 7;
    }

    return index;
}

#if 0
bool FormatDisplayZone(VMS_BYTE addr, VMS_BYTE zoneCnt, VMS_WORD width,
        VMS_WORD height,VMS_BYTE* frameBuf, VMS_INT& frameSz)
{
    if (NULL == frameBuf)
    return false;

    if (zoneCnt > 8)
    return false;

    VMS_BYTE tempBuf[FRAME_PAGE_SIZE] =
    {   0};
    VMS_INT index = 0;

    // head
    tempBuf[index++] = FR_STX;

    tempBuf[index++] = addr;
    tempBuf[index++] = COMMAND_SET_ZONE;

    tempBuf[index++] = zoneCnt;//zone count.
    tempBuf[index++] = 1;// scene count.

    VMS_BYTE reservedByte[6] =
    {   0x0B, 0x03, 0x16, 0x0F, 0x18, 0x18};  //  reserved 6 bytes!! !!!!!!!!!!
    memcpy(tempBuf + index, reservedByte, 6);
    index += 6;

    /*************************************************
     *          Zone info
     * **********************************************/
    int byteCnt = 0;
//    for (VMS_BYTE i = 0; i < 4; i++)
//    {
//        byteCnt = SetZoneInfo(i, width, height, tempBuf + index);
//        index += byteCnt;
//    }
    for (VMS_BYTE i = 0; i < zoneCnt; i++)
    {
        byteCnt = SetZoneInfo(i, width, height, tempBuf + index);
        index += byteCnt;
    }

    // set zone info 2
    for (VMS_BYTE i = 0; i < 1; i++)
    {
        VMS_BYTE sceneName[8] =
        {   0x31, 0x30, 0x31, 0x38};  // scene name

        memcpy(tempBuf + index, sceneName, 8);
        index += 8;

        VMS_WORD sceneStayTime = 5;

        tempBuf[index++] = VMS_HI_BYTE(sceneStayTime);// scene stay time
        tempBuf[index++] = VMS_LO_BYTE(sceneStayTime);

        memset(tempBuf + index, 0, 7);
        index += 7;
        tempBuf[index++] = ~(0xFF << zoneCnt);// scene map is 8 byte !!!!!!!!!!!! be careful

        memset(tempBuf + index, 0, 6);
        index += 6;//  reserved 6 bytes!!
    }

    // crc
    VMS_WORD crcVal = Gen_crc(tempBuf + 1, index - 1);
    tempBuf[index++] = VMS_HI_BYTE(crcVal);
    tempBuf[index++] = VMS_LO_BYTE(crcVal);

    // 03
    tempBuf[index++] = FR_ETX;

    //ShowContent(tempBuf, index);
    // convert 0x02,0x03,0x15,0x1B ,except frame start and frame end.
    return SpecialConvert(tempBuf, index, frameBuf, frameSz);
}
#endif

bool FormatDisplayZone(VMS_BYTE addr, std::vector<SingleZoneInfo> zoneInfos,
        VMS_BYTE* frameBuf, VMS_INT& frameSz)
{
    if (NULL == frameBuf)
        return false;

    if (zoneInfos.size() < 1 || zoneInfos.size() > 8)
        return false;

    VMS_BYTE tempBuf[FRAME_PAGE_SIZE] =
    { 0 };
    VMS_INT index = 0;

    // head
    tempBuf[index++] = FR_STX;

    tempBuf[index++] = addr;
    tempBuf[index++] = COMMAND_SET_ZONE;

    tempBuf[index++] = zoneInfos.size(); //zone count.
    tempBuf[index++] = 1; // scene count.

    VMS_BYTE reservedByte[6] =
    { 0x0B, 0x03, 0x16, 0x0F, 0x18, 0x18 };  //  reserved 6 bytes!! !!!!!!!!!!
    memcpy(tempBuf + index, reservedByte, 6);
    index += 6;

    /*************************************************
     *          Zone info
     * **********************************************/
    int byteCnt = 0;
    byteCnt = SetZoneInfo(zoneInfos, tempBuf + index);
    index += byteCnt;

    // set zone info 2
    for (VMS_BYTE i = 0; i < 1; i++)
    {
        VMS_BYTE sceneName[8] =
        { 0x31, 0x30, 0x31, 0x38 };  // scene name

        memcpy(tempBuf + index, sceneName, 8);
        index += 8;

        VMS_WORD sceneStayTime = 5;

        tempBuf[index++] = VMS_HI_BYTE(sceneStayTime); // scene stay time
        tempBuf[index++] = VMS_LO_BYTE(sceneStayTime);

        memset(tempBuf + index, 0, 7);
        index += 7;
        tempBuf[index++] = ~(0xFF << zoneInfos.size()); // scene map is 8 byte !!!!!!!!!!!! be careful

        memset(tempBuf + index, 0, 6);
        index += 6; //  reserved 6 bytes!!
    }

    // crc
    VMS_WORD crcVal = Gen_crc(tempBuf + 1, index - 1);
    tempBuf[index++] = VMS_HI_BYTE(crcVal);
    tempBuf[index++] = VMS_LO_BYTE(crcVal);

    // 03
    tempBuf[index++] = FR_ETX;

    //ShowContent(tempBuf, index);
    // convert 0x02,0x03,0x15,0x1B ,except frame start and frame end.
    return SpecialConvert(tempBuf, index, frameBuf, frameSz);
}

bool FormatZoneDataHeader(VMS_BYTE addr, VMS_BYTE zoneId, VMS_WORD width,
        VMS_WORD height, VMS_BYTE* frameBuf, VMS_INT& frameSz)
{
    VMS_BYTE tempBuf[FRAME_PAGE_SIZE] =
    { 0 };
    VMS_INT index = 0;

    // head
    tempBuf[index++] = FR_STX;
    tempBuf[index++] = addr;
    tempBuf[index++] = COMMAND_SET_ZONEDISPLAYHEADERDATA;

    // zone id
    tempBuf[index++] = zoneId;

    // display header
    tempBuf[index++] = 0x44; // "DISP"
    tempBuf[index++] = 0x49;
    tempBuf[index++] = 0x53;
    tempBuf[index++] = 0x50;

    VMS_WORD zoneDataCrc = 0;
    tempBuf[index++] = VMS_HI_BYTE(zoneDataCrc); // crc of display data(no effect at all,set to 0 now)
    tempBuf[index++] = VMS_LO_BYTE(zoneDataCrc);

    tempBuf[index++] = VMS_HI_BYTE(width); // width
    tempBuf[index++] = VMS_LO_BYTE(width);

    tempBuf[index++] = VMS_HI_BYTE(height); // height
    tempBuf[index++] = VMS_LO_BYTE(height);

    tempBuf[index++] = 2; //primary colors

//    if (zoneId > 4)
//        return false;
    if (zoneId > 7)
        return false;

    VMS_DWORD schOffset = g_SchTableOffset + zoneId * g_OffsetStep;
    VMS_WORD hiWord = VMS_HIWORD(schOffset);
    VMS_WORD loWord = VMS_LOWORD(schOffset);
    tempBuf[index++] = VMS_LO_BYTE(hiWord);  // schedule table offset
    tempBuf[index++] = VMS_HI_BYTE(loWord);
    tempBuf[index++] = VMS_LO_BYTE(loWord);

    VMS_DWORD msgOffset = g_MsgTableOffset + zoneId * g_OffsetStep;
    hiWord = VMS_HIWORD(msgOffset);
    loWord = VMS_LOWORD(msgOffset);
    tempBuf[index++] = VMS_LO_BYTE(hiWord); // message table offset
    tempBuf[index++] = VMS_HI_BYTE(loWord);
    tempBuf[index++] = VMS_LO_BYTE(loWord);

    VMS_DWORD textOffset = g_TxtTableOffset + zoneId * g_OffsetStep;
    hiWord = VMS_HIWORD(textOffset);
    loWord = VMS_LOWORD(textOffset);
    tempBuf[index++] = VMS_LO_BYTE(hiWord);  // text table offset
    tempBuf[index++] = VMS_HI_BYTE(loWord);
    tempBuf[index++] = VMS_LO_BYTE(loWord);

    VMS_DWORD bitMapOffset = g_BitMapTableOffset + zoneId * g_OffsetStep;
    hiWord = VMS_HIWORD(bitMapOffset);
    loWord = VMS_LOWORD(bitMapOffset);
    tempBuf[index++] = VMS_LO_BYTE(hiWord);  // bitmap table offset
    tempBuf[index++] = VMS_HI_BYTE(loWord);
    tempBuf[index++] = VMS_LO_BYTE(loWord);

    // crc
    VMS_WORD crcVal = Gen_crc(tempBuf + 1, index - 1);
    tempBuf[index++] = VMS_HI_BYTE(crcVal);
    tempBuf[index++] = VMS_LO_BYTE(crcVal);

    // 03
    tempBuf[index++] = FR_ETX;

    //ShowContent(tempBuf, index);

    // convert 0x02,0x03,0x15,0x1B ,except frame start and frame end.
    return SpecialConvert(tempBuf, index, frameBuf, frameSz);
}

void GetCenterStartPos(VMS_WORD screenWidth, VMS_BYTE perCharWidth,
        const std::string& contentString, VMS_WORD& startPos)
{
    VMS_WORD totalLen = GetStringWidth(perCharWidth,contentString);
    if (screenWidth > totalLen)
    {
        startPos = (screenWidth - totalLen) / 2;
    } else
    {
        startPos = 0;
    }
}

VMS_WORD GetStringWidth(VMS_BYTE perCharWidth,const std::string& contentString)
{
    QString qstr = QString::fromStdString(contentString);
    QByteArray valArr = UTF82GBK(qstr); // content data.
    VMS_WORD totalLen = valArr.size() * perCharWidth;

    return totalLen;
}

}

