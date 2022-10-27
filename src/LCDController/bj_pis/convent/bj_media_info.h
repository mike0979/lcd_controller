#ifndef BJ_MEDIA_INFO_H
#define BJ_MEDIA_INFO_H
#include <string>
#include "bj_pis/utils/JsonObject.h"
#include <memory>
#include <vector>

using namespace std;

enum bj_media_type
{
	VIDEO=1,
	IMAGE=2,
	TEXT=3,
	DATE=4,
	TIME=5,
	STATION_ATS=6,
	FIRST_LAST_INFO=7,
	TRAIN_ATS=8
};

class bj_font
{
public:
	string name;
	int size;
	bool bold;
	bool italic;
	string textColor;
	int align;
	int effect;
	void Parse(JsonObject& json);
};

class bj_partition_media
{
public:
	int media_id;
	int partition_id;
	bj_media_type meida_type;
	JsonObject param;
  static int GetNewMediaId(){return ++s_media_id;}
private:
  static int s_media_id;
};

#endif // BJ_MEDIA_INFO_H
