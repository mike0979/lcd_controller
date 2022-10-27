/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file : ChineseConver.h 
* @author : Benson
* @date : Nov 20, 2017
* @brief :
*/


#ifndef LEDPLAYER_CHINESECONVERT_H_
#define LEDPLAYER_CHINESECONVERT_H_

#include <QtCore/qbytearray.h>
#include <QtCore/qstring.h>
#include <QtCore/qtextcodec.h>
#include <cstdio>

inline void ShowContent(const char *p, int size)
{
    for (int i = 0; i < size; i++)
        printf("%02x ", (unsigned char) p[i]);

    printf("\n");
}

inline QString GBK2UTF8(const QByteArray &gbkArry)
{
    QTextCodec *gbk = QTextCodec::codecForName("gbk");
    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");

    QTextCodec::setCodecForLocale(utf8);
    QTextCodec::setCodecForCStrings(utf8);

    // 1. gbk to unicode
    QString strUnicode = gbk->toUnicode(gbkArry.data());
    // 2. unicode to gbk
    QByteArray utf8_bytes= utf8->fromUnicode(strUnicode);
    printf("%s\n",utf8_bytes.data());

    QString g2u(utf8_bytes);
    return g2u;
}

inline void InitCodecToUTF8()
{
    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(utf8);
    QTextCodec::setCodecForCStrings(utf8);
}

inline QByteArray UTF82GBK(const QString &inStr)
{
    QTextCodec *gbk = QTextCodec::codecForName("gbk");
    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(utf8);
    QTextCodec::setCodecForCStrings(utf8);

    //1. utf8 -> unicode
    QString strUnicode = utf8->toUnicode(inStr.toLocal8Bit().data());

    //2. unicode -> gbk
    QByteArray gb_bytes= gbk->fromUnicode(strUnicode);

    //printf("UTF8 String %s --> gbk array=\n",inStr.toStdString().c_str());
    //ShowContent(gb_bytes.data(),gb_bytes.size());

    return gb_bytes;
}



#endif /* LEDPLAYER_CHINESECONVERT_H_ */
