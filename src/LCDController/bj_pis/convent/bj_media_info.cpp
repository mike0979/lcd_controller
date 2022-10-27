#include"bj_media_info.h"

void bj_font::Parse(JsonObject& json)
{
    name=(string)json["name"];
    size=json["size"];
    bold=json["bold"];
    italic=json["italic"];
    textColor=(string)json["textColor"];
    align=json["align"];
    effect=json["effect"];
}


int bj_partition_media::s_media_id=0;
