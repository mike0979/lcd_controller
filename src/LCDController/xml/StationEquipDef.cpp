/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/


/*
 * StationEquipDef.cpp
 *
 *  Created on: May 8, 2017
 *      Author: root
 */
#include "StationEquipDef.h"
#include "XMLParser.h"
#include "Log.h"
#include <stdlib.h>

using namespace xml;

void StationEquip::print()
{
	LogD("mEquipID = %s, mEquipIP = %s, mText = %s\n", mEquipID.c_str(),mEquipIP.c_str(),mText.c_str());
}

void StationEquip::StationEquipTextReader(const std::string &text, void *data)
{
	StationEquip* d = (StationEquip*)data;
	d->mText = text;
}
//---------
StationDef::~StationDef()
{
	for(std::list<StationEquip*>::iterator i=mEquipList.begin();i!=mEquipList.end();++i)
	{
		delete (*i);
	}
	mEquipList.clear();

}
void StationDef::print()
{
	LogD("mCName = %s, mEName = %s, mEquipType = %s\n", mCName.c_str(),mEName.c_str(),mEquipType.c_str());

	for(std::list<StationEquip*>::iterator i=mEquipList.begin();i!=mEquipList.end();++i)
	{
		(*i)->print();
	}
}

void *StationDef::StationDefReader(const char *name, const char **atts, void *data)
{
	StationDef *station = (StationDef *)data;
	StationEquip *equip = new StationEquip();

	KeyValuePair kvp[] = {
			{"id", &(equip->mEquipID), 'S', ""},
			{"ip", &(equip->mEquipIP), 'S', ""},
			{NULL, NULL, 0, NULL}
		};
	XMLParser::FastAtts(atts, kvp);

	station->mEquipList.push_back(equip);
	return equip;
}
//-------------
StationsDef::~StationsDef()
{
	for(std::list<StationDef*>::iterator i=mStationDefList.begin();i!=mStationDefList.end();++i)
	{
		delete (*i);
	}
	mStationDefList.clear();
}
void StationsDef::print()
{
	LogD("mTimestamp = %s\n", mTimestamp.c_str());

	for(std::list<StationDef*>::iterator i=mStationDefList.begin();i!=mStationDefList.end();++i)
	{
		(*i)->print();
	}

}
bool StationsDef::StationsDefXMLParser(const std::string &file,
				StationsDef *arrivalmsg)
{
	XMLParser xp(file);

	XMLParserHandler textsHandler[] = {
		{"stations", NULL, StationsDef::SelfReader, NULL, NULL},
		{"station", "stations", StationsDef::StationDefReader, NULL, NULL},
		{"equip", "stations/station", StationDef::StationDefReader, NULL, StationEquip::StationEquipTextReader},
		{NULL, NULL, NULL, NULL, NULL},
	};

	xp.parser(textsHandler, arrivalmsg);

	return xp.isParsered();
}

void *StationsDef::SelfReader(const char *name, const char **atts, void *data)
{
	if (data == NULL) {
		data = new StationsDef();
	}

	StationsDef *arrivalmsg = (StationsDef *)data;

	KeyValuePair kvp[] = {
			{"timestamp", &(arrivalmsg->mTimestamp), 'S', NULL},
			{NULL, NULL, 0, NULL}
	};
	XMLParser::FastAtts(atts, kvp);

	return data;
}
void *StationsDef::StationDefReader(const char *name, const char **atts, void *data)
{
	StationsDef *stations = (StationsDef *)data;
	StationDef *station = new StationDef();

	KeyValuePair kvp[] = {
			{"cname", &(station->mCName), 'S', ""},
			{"ename", &(station->mEName), 'S', ""},
			{"equiptype", &(station->mEquipType), 'S', ""},
			{NULL, NULL, 0, NULL}
		};
	XMLParser::FastAtts(atts, kvp);

	stations->mStationDefList.push_back(station);
	return station;
}

const char *StationsDef::TAG = "StationsDef";
const char *StationDef::TAG = "StationDef";
const char *StationEquip::TAG = "StationEquip";



