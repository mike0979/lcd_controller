/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : LEDProtocolTestMain.cpp
 * @author : root
 * @date : Nov 18, 2017
 * @brief : xxxx.
 */
#if 0

#include <LEDPlayer/ChineseConvert.h>
#include <LEDPlayer/LEDProtocol.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qstring.h>
#include <QtCore/qtextcodec.h>
#include <Termios.h>
#include <Thread.h>
#include <unistd.h>
#include <cstdio>
#include <string>

using namespace VMS;

#define VMS_ADDR 0
//#define VMS_TOTAL_WIDTH 256
//#define VMS_TOTAL_HEIGHT 64
#define VMS_TOTAL_WIDTH 384
#define VMS_TOTAL_HEIGHT 64
#define VMS_SINGLE_WIDTH 384
#define VMS_SINGLE_HEIGHT 16

int g_displaytype = 0;
int g_speed = 0;
int g_delay = 0;
int g_bright = 0;

void printContent(const char *p, int size)
{
    for (int i = 0; i < size; i++)
        printf("%02x ", (unsigned char) p[i]);

    printf("\n");
}

void printContent(const unsigned char *p, int size)
{
    for (int i = 0; i < size; i++)
        printf("%02x ", p[i]);

    printf("\n");
}

void UTF82GbkTest()
{
    QTextCodec *utf8All = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(utf8All);
    QTextCodec::setCodecForCStrings(utf8All);
    QString a = "测试01";
    char *p = a.toLocal8Bit().data(); // get char*
    printContent(p, a.toLocal8Bit().size());

    QByteArray gbArray = UTF82GBK(a);
    printContent(gbArray.data(), gbArray.size());

    QString utf8rslt = GBK2UTF8(gbArray);
    printf("%s\n", utf8rslt.toStdString().c_str());
}

void FormatZoneDataHeaderTest()
{
    VMS_BYTE addr = 0;
    VMS_BYTE zoneId = 0;
    VMS_WORD width = 256;
    VMS_WORD height = 64;

    VMS_BYTE frameBuf[FRAME_PAGE_SIZE] =
    { 0 };
    VMS_INT frameSz = 0;

    bool rlst = FormatZoneDataHeader(addr, zoneId, width, height, frameBuf,
            frameSz);
    if (rlst)
    {
        printf(" FormatZone0DataHeader rslt,size=%d:\n", frameSz);
        printContent((VMS_CHAR_C*) frameBuf, frameSz);
    }

    zoneId = 1;
    memset(frameBuf, 0, sizeof(frameBuf));

    rlst = FormatZoneDataHeader(addr, zoneId, width, height, frameBuf, frameSz);
    if (rlst)
    {
        printf(" FormatZone1DataHeader rslt,size=%d:\n", frameSz);
        printContent((VMS_CHAR_C*) frameBuf, frameSz);
    }

}

void FormatDisplayZoneTest()
{
//    VMS_BYTE zoneCnt = 2;
//
//    VMS_BYTE frameBuf[FRAME_PAGE_SIZE] =
//    { 0 };
//    VMS_INT frameSz = 0;
//
//    bool rlst = FormatDisplayZone(VMS_ADDR, zoneCnt, VMS_SINGLE_WIDTH,VMS_SINGLE_HEIGHT, frameBuf, frameSz);
//
//    if (rlst)
//    {
//        printf(" FormatDisplayZoneTest rslt,size=%d:\n", frameSz);
//        printContent((VMS_CHAR_C*) frameBuf, frameSz);
//    }
}

void FormatLongStr2ZoneTest()
{
    VMS_BYTE zoneId = 0;
    const std::string formatStr =
            "\\C00000000\\cb000000000\\cf255000000\\F00\\T00";
    QTextCodec *utf8All = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(utf8All);
    QTextCodec::setCodecForCStrings(utf8All);
    QString contentStr = "测试01";

    short displayType = 32;
    short speed = 2; // 2*0.01s
    short delay = 300; // 300*0.01s

    VMS_BYTE frameBuf[FRAME_PAGE_SIZE] =
    { 0 };
    VMS_INT frameSz = 0;

    bool rlst = FormatLongStr2Zone(VMS_ADDR, zoneId, formatStr,
            contentStr.toStdString(), displayType, speed, delay, frameBuf,
            frameSz);

    if (rlst)
    {
        printf(" FormatLongStr2Zone0Test rslt,size=%d:\n", frameSz);
        printContent((VMS_CHAR_C*) frameBuf, frameSz);
    }

    zoneId = 1;
    memset(frameBuf, 0, sizeof(frameBuf));
    contentStr = "abc";

    rlst = FormatLongStr2Zone(VMS_ADDR, zoneId, formatStr,
            contentStr.toStdString(), displayType, speed, delay, frameBuf,
            frameSz);

    if (rlst)
    {
        printf(" FormatLongStr2Zone1Test rslt,size=%d:\n", frameSz);
        printContent((VMS_CHAR_C*) frameBuf, frameSz);
    }

}

int SetLongStr2Zone(int fd, VMS_BYTE zoneId)
{
    if (fd < 0)
        return -1;

    const std::string formatStr =
            "\\C00000000\\cb000000000\\cf255000000\\F03\\T00";
    QTextCodec *utf8All = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(utf8All);
    QTextCodec::setCodecForCStrings(utf8All);
    QString contentStr = "严禁携带易燃、易爆、剧毒等危险品上车！！";
//    QString contentStr = "本站 木渎站";
//    QString contentStr = "1234";

    short displayType = g_displaytype; //0，46
    short speed = g_speed; // 1*0.01s
    short delay = g_delay; // 300*0.01s

//    short displayType = g_displaytype; //0，46
//    short speed = 2; // 1*0.01s
//    short delay = 300; // 300*0.01s

    VMS_BYTE frameBuf[FRAME_PAGE_SIZE] =
    { 0 };
    VMS_INT frameSz1 = 0;

    /*Zone 0*/
    bool rlst = FormatLongStr2Zone(VMS_ADDR, zoneId, formatStr,
            contentStr.toStdString(), displayType, speed, delay, frameBuf,
            frameSz1);
    if (!rlst)
    {
        printf("SetLongStr2Zone0 failed!\n");
        return -1;
    }

    int sendSz = write(fd, frameBuf, frameSz1);
    if (sendSz == frameSz1)
    {
        printf("SetLongStr2Zone0 Write Data:\n");
        printContent(frameBuf, frameSz1);
    }
    Thread::sleep(500);
    VMS_BYTE readBuf[1024] =
    { 0 };
    int readSz = read(fd, readBuf, 1024);
    if (readSz > 0)
    {
        printf("SetLongStr2Zone0 Read Data:\n");
        printContent(readBuf, readSz);
    }

    return readSz;
}

int SetTime2Zone(int fd, VMS_BYTE zoneId)
{
    if (fd < 0)
        return -1;

    const std::string formatStr =
            "\\C00320000\\cf000255000\\F02\\dY-\\dmz-\\ddz\\C01200000\\cf000255000\\F02\\tHz:\\tMz:\\tSz";
    QTextCodec *utf8All = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(utf8All);
    QTextCodec::setCodecForCStrings(utf8All);
    QString contentStr = "";
    ;

    short displayType = 0; //0，46
    short speed = 1; // 1*0.01s
    short delay = 0; // 300*0.01s

    VMS_BYTE frameBuf[FRAME_PAGE_SIZE] =
    { 0 };
    VMS_INT frameSz1 = 0;

    /*Zone 0*/
    bool rlst = FormatLongStr2Zone(VMS_ADDR, zoneId, formatStr,
            contentStr.toStdString(), displayType, speed, delay, frameBuf,
            frameSz1);
    if (!rlst)
    {
        printf("SetLongStr2Zone1 failed!\n");
        return -1;
    }

    int sendSz = write(fd, frameBuf, frameSz1);
    if (sendSz == frameSz1)
    {
        printf("SetLongStr2Zone1 Write Data:\n");
        printContent(frameBuf, frameSz1);
    }
    Thread::sleep(500);
    VMS_BYTE readBuf[1024] =
    { 0 };
    int readSz = read(fd, readBuf, 1024);
    if (readSz > 0)
    {
        printf("SetLongStr2Zone1 Read Data:\n");
        printContent(readBuf, readSz);
    }

    return readSz;
}

int SetDisplayZone(int fd, std::vector<SingleZoneInfo> zoneInfos)
{
    VMS_BYTE frameBuf[FRAME_PAGE_SIZE] =
    { 0 };
    VMS_INT frameSz = 0;

    bool rlst = FormatDisplayZone(VMS_ADDR, zoneInfos, frameBuf, frameSz);

    if (!rlst)
    {
        printf("SetDisplayZone false!\n");
        return -1;
    }

    int sendSz = write(fd, frameBuf, frameSz);
    if (sendSz == frameSz)
    {
        printf("SetDisplayZone Write Data:\n");
        printContent(frameBuf, frameSz);
    }

    Thread::sleep(500);
    VMS_BYTE readBuf[1024] =
    { 0 };
    int readSz = read(fd, readBuf, 1024);
    if (readSz > 0)
    {
        printf("SetDisplayZone Read Data:\n");
        printContent(readBuf, readSz);
    }

    return readSz;
}

int SetZoneHeader(int fd, VMS_BYTE zoneId)
{
    VMS_BYTE frameBuf[FRAME_PAGE_SIZE] =
    { 0 };
    VMS_INT frameSz = 0;

    bool rlst = FormatZoneDataHeader(VMS_ADDR, zoneId,
    VMS_TOTAL_WIDTH, VMS_TOTAL_HEIGHT, frameBuf, frameSz);
    if (!rlst)
    {
        printf(" SetZoneHeader1 failed!\n");
        return -1;
    }

    int sendSz = write(fd, frameBuf, frameSz);
    if (sendSz == frameSz)
    {
        printf("SetZoneHeader1 Write Data:\n");
        printContent(frameBuf, frameSz);
    }

    Thread::sleep(500);
    VMS_BYTE readBuf[1024] =
    { 0 };
    int readSz = read(fd, readBuf, 1024);
    if (readSz > 0)
    {
        printf("SetZoneHeader1 Read Data:\n");
        printContent(readBuf, readSz);
    }

    return readSz;
}

int SetCurrentTime(int fd)
{
    VMS_BYTE frameBuf[] =
    { 0x02, 0x00, 0x33, 0x00, 0x00, 0x9c, 0xf5, 0x03 };
    VMS_INT frameSz = sizeof(frameBuf);

    int sendSz = write(fd, frameBuf, frameSz);
    if (sendSz == frameSz)
    {
        printf("SetCurrentTime Write Data:\n");
        printContent(frameBuf, frameSz);
    }

    Thread::sleep(500);
    VMS_BYTE readBuf[1024] =
    { 0 };
    int readSz = read(fd, readBuf, 1024);
    if (readSz > 0)
    {
        printf("SetCurrentTime Read Data:\n");
        printContent(readBuf, readSz);
    }

    return readSz;
}

int TurnOnOffScreen(int fd, bool bOn)
{
    VMS_BYTE frameBuf[FRAME_PAGE_SIZE] =
    { 0 };
    VMS_INT frameSz = 0;

    bool rlst = FormatOnOffControl(VMS_ADDR, bOn, frameBuf, frameSz);
    if (!rlst)
    {
        printf(" TurnOnOffScreen failed!\n");
        return -1;
    }

    int sendSz = write(fd, frameBuf, frameSz);
    if (sendSz == frameSz)
    {
        printf("TurnOnOffScreen Write Data:\n");
        printContent(frameBuf, frameSz);
    }

    Thread::sleep(500);

    VMS_BYTE readBuf[1024] =
    { 0 };
    int readSz = read(fd, readBuf, 1024);
    if (readSz > 0)
    {
        printf("TurnOnOffScreen Read Data:\n");
        printContent(readBuf, readSz);
    }

    return readSz;
}

int AdjustBrightness(int fd, bool bManual, VMS_BYTE level)
{
    VMS_BYTE frameBuf[FRAME_PAGE_SIZE] =
    { 0 };
    VMS_INT frameSz = 0;

    printf("bManual =%d, level=%d(%02x)\n", bManual, level, level);

    bool rlst = FormatBrightnessControl(VMS_ADDR, bManual, level, frameBuf,
            frameSz);
    if (!rlst)
    {
        printf(" AdjustBrightness failed!\n");
        return -1;
    }

    int sendSz = write(fd, frameBuf, frameSz);
    if (sendSz == frameSz)
    {
        printf("AdjustBrightness Write Data:\n");
        printContent(frameBuf, frameSz);
    }

    VMS_BYTE readBuf[1024] =
    { 0 };
    int readSz = read(fd, readBuf, 1024);
    if (readSz > 0)
    {
        printf("AdjustBrightness Read Data:\n");
        printContent(readBuf, readSz);
    }

    return readSz;
}

void DataTest2()
{
    Termios tmios;
    int fd = tmios.open("/dev/ttyUSB0", Termios::RW);
    if (fd > 0)
    {
        tmios.setBaudRate(115200);
        tmios.setDataBits(8);
        tmios.setStopBits(1);
        tmios.setParityCheck(Termios::PC_None);
        tmios.setFlowControl(Termios::FC_None);
    }

    SetLongStr2Zone(fd, 0);
    SetLongStr2Zone(fd, 1);
    SetLongStr2Zone(fd, 2);
    SetTime2Zone(fd, 3);

    std::vector<SingleZoneInfo> zoneInfos;
    SingleZoneInfo singleZone0; // str 0
    singleZone0.xPos = 192;
    singleZone0.yPos = 0;
    singleZone0.height = 16;
    singleZone0.width = 192;

    SingleZoneInfo singleZone1; // str 1
    singleZone1.xPos = 192;
    singleZone1.yPos = 16;
    singleZone1.height = 16;
    singleZone1.width = 192;

    SingleZoneInfo singleZone2; // str 2
    singleZone2.xPos = 192;
    singleZone2.yPos = 32;
    singleZone2.height = 16;
    singleZone2.width = 192;

    SingleZoneInfo singleZone3; // time 1, zone 3
    singleZone3.xPos = 192;
    singleZone3.yPos = 48;
    singleZone3.height = 16;
    singleZone3.width = 192; //192;

    zoneInfos.push_back(singleZone0);
    zoneInfos.push_back(singleZone1);
    zoneInfos.push_back(singleZone2);
    zoneInfos.push_back(singleZone3);

    SetDisplayZone(fd, zoneInfos);

    for (int i = 0; i < zoneInfos.size(); ++i)
    {
        SetZoneHeader(fd, i);
    }
}

void DataTest3()
{
    Termios tmios;
    int fd = tmios.open("/dev/ttyUSB0", Termios::RW);
    if (fd > 0)
    {
        tmios.setBaudRate(115200);
        tmios.setDataBits(8);
        tmios.setStopBits(1);
        tmios.setParityCheck(Termios::PC_None);
        tmios.setFlowControl(Termios::FC_None);
    }

    SetLongStr2Zone(fd, 0);
    SetLongStr2Zone(fd, 1);
    SetLongStr2Zone(fd, 2);
    SetTime2Zone(fd, 3);
    SetLongStr2Zone(fd, 4);
    SetLongStr2Zone(fd, 5);
    SetLongStr2Zone(fd, 6);
    SetTime2Zone(fd, 7);

    std::vector<SingleZoneInfo> zoneInfos;
    SingleZoneInfo singleZone0; // str 0
    singleZone0.xPos = 0;
    singleZone0.yPos = 0;
    singleZone0.height = 16;
    singleZone0.width = 192;

    SingleZoneInfo singleZone1; // str 1
    singleZone1.xPos = 0;
    singleZone1.yPos = 16;
    singleZone1.height = 16;
    singleZone1.width = 192;

    SingleZoneInfo singleZone2; // str 2
    singleZone2.xPos = 0;
    singleZone2.yPos = 32;
    singleZone2.height = 16;
    singleZone2.width = 192;

    SingleZoneInfo singleZone3; // time 1, zone 3
    singleZone3.xPos = 0;
    singleZone3.yPos = 48;
    singleZone3.height = 16;
    singleZone3.width = 192; //192;

    SingleZoneInfo singleZone4; // str 4
    singleZone4.xPos = 192;
    singleZone4.yPos = 0;
    singleZone4.height = 16;
    singleZone4.width = 192;

    SingleZoneInfo singleZone5; // str 5
    singleZone5.xPos = 192;
    singleZone5.yPos = 16;
    singleZone5.height = 16;
    singleZone5.width = 192;

    SingleZoneInfo singleZone6; // str 6
    singleZone6.xPos = 192;
    singleZone6.yPos = 32;
    singleZone6.height = 16;
    singleZone6.width = 192;

    SingleZoneInfo singleZone7; // time 2,zone 7
    singleZone7.xPos = 192;
    singleZone7.yPos = 48;
    singleZone7.height = 16;
    singleZone7.width = 192;

//    zoneInfos.push_back(singleZone0);
//    zoneInfos.push_back(singleZone1);
//    zoneInfos.push_back(singleZone2);
//    zoneInfos.push_back(singleZone3);
//    zoneInfos.push_back(singleZone4);
//    zoneInfos.push_back(singleZone5);
//    zoneInfos.push_back(singleZone6);
//    zoneInfos.push_back(singleZone7);

    zoneInfos.push_back(singleZone0);
    zoneInfos.push_back(singleZone1);
    zoneInfos.push_back(singleZone2);
    zoneInfos.push_back(singleZone3);
    zoneInfos.push_back(singleZone4);
    zoneInfos.push_back(singleZone5);
    zoneInfos.push_back(singleZone6);
    zoneInfos.push_back(singleZone7);

    SetDisplayZone(fd, zoneInfos);

    for (int i = 0; i < zoneInfos.size(); ++i)
    {
        SetZoneHeader(fd, i);
    }
}

// get screen status.
void GetScreenStatus(int fd)
{
    VMS_BYTE frameBuf[32] =
    { 0 };
    int frameSz = 0;

    bool rlst = FormatOnOffGet(VMS_ADDR, frameBuf, frameSz);
    if (!rlst)
    {
        printf(" FormatOnOffGet failed!\n");
    }

    int sendSz = write(fd, frameBuf, frameSz);
    if (sendSz == frameSz)
    {
        printf("FormatOnOffGet Write Data:\n");
        printContent(frameBuf, frameSz);
    }

    Thread::sleep(500);
    VMS_BYTE readBuf[1024] =
    { 0 };
    int readSz = read(fd, readBuf, 1024);
    if (readSz > 0)
    {
        printf("FormatOnOffGet Read Data:\n");
        printContent(readBuf, readSz);

        // parse rlst
        bool on = false;
        if (ParseOnOffStatus(readBuf, readSz, on))
        {
            printf("ParseOnOffStatus rlst = %d\n", (VMS_BYTE) on);
        }
    }

}

int MAIN_ARGC;
char **MAIN_ARGV;
int main(int argc, char *argv[])
{
    MAIN_ARGC = argc;
    MAIN_ARGV = argv;

    if (argc < 5)
    {
        printf("Please set displaytype,speed,delay, ScreenCnt!\n");
        return -1;
    }

    g_displaytype = atoi(argv[1]);
    g_speed = atoi(argv[2]);
    g_delay = atoi(argv[3]);

    int screenType = atoi(argv[4]);

    printf("displaytype is %d\n", g_displaytype);
    printf("speed is %d\n", g_speed);
    printf("delay is %d\n", g_delay);
    printf("ScreenCnt is %d\n", screenType);

    /**
     * Test crc
     */
//CrcTest();
    /**
     * Test UTF82GBK
     */
//UTF82GbkTest();
    /**
     * Test SpecialConvert(),convert 0x02,0x03,0x15,0x1B
     */
//SpecialConvertTest();
    /**
     * Test FormatZoneDataHeader
     */
//    FormatZoneDataHeaderTest();
    /**
     * Test FormatDisplayZone
     */
//    FormatDisplayZoneTest();
    /**
     * Test FormatLongStr2Zone
     */
//    FormatLongStr2ZoneTest();
//    DataTest();
    if (1 == screenType)
    {
        DataTest2();
    } else
    {
        DataTest3();
    }

//    DataTest4();
//    DataTest5();
    return 0;
}

#endif
