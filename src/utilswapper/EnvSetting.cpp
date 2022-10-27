#include "EnvSetting.h"

#include <stdlib.h>

const char * EnvSetting::Get(const char *name, const char *defValue)
{
	const char *value = getenv(name);

	return value ? value : defValue;
}

EnvSetting::EnvSetting()
{

}
