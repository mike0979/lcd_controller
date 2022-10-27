#include "KeyValuePair.h"
#include "Log.h"

#include <stdlib.h>
#include <string.h>

KeyValuePair::KeyValuePair(const char *key, void *value, const char valType, const char *defVal) : mValType(valType)
{
	mKey = key;
	mValue.V = value;

//	mValType = valType;
	mDefVal = defVal;
}

void KeyValuePair::setValue(const char *value,const char *key)
{
	if (value == NULL) {
		value = mDefVal;
	}

	switch (mValType) {
	case 'I':
		if (value == NULL) {
			*(mValue.I) = 0;
		}
		else {
			*(mValue.I) = strtoul(value, NULL, 0);
		}
		break;

	case 'H':
		if (value == NULL) {
			*(mValue.H) = 0x00000000;
		}
		else {
			*(mValue.H) = strtoul(value, NULL, 16);
		}
		break;

	case 'B':
		if (value == NULL) {
			*(mValue.B) = false;
		}
		else {
			if (atoi(value) != 0 || strcasecmp(value, "true") == 0 || strcasecmp(value, "enable") == 0) {
				*(mValue.B) = true;
			}
			else {
				*(mValue.B) = false;
			}
		}
		break;

	case 'F':
		if (value == NULL) {
			*(mValue.F) = 0.0f;
		}
		else {
			*(mValue.F) = atof(value);
		}
		break;

	case 'S':
		if (value == NULL) {
			*(mValue.S) = "";
		}
		else {
			*(mValue.S) = value;
		}
		break;

	case 'C':
		if (value == NULL) {
			*(mValue.C) = 0x00000000;
		}
		else {
			*(mValue.C) = strtoul(value, NULL, 16);
		}
		break;
	case 'L':
		if(key != NULL)
		{
			if(value != NULL)
			{
				(*(mValue.L))[key] = value;
			}
			else
			{
				(*(mValue.L))[key] = "";
			}
		}

		break;
	case 'M':
		if(key != NULL)
		{
			if(value != NULL)
			{
				(*(mValue.M))[key] = strtoul(value, NULL, 0);;
			}
			else
			{
				(*(mValue.M))[key] = -1;
			}
		}
		break;
	default:
		LogE("Unknown Value Type : %c. Use I(nt)/B(ool)/F(loat)/S(tring)/C(olor) only.\n", mValType);
		break;
	}
}

std::string KeyValuePair::getValue()
{
	char vbuf[32];

	switch (mValType) {
	case 'I':
		snprintf(vbuf, sizeof(vbuf), "%d", *(mValue.I));
		break;

	case 'H':
		snprintf(vbuf, sizeof(vbuf), "0x%X", *(mValue.H));
		break;

	case 'B':
		snprintf(vbuf, sizeof(vbuf), "%s", *(mValue.B) ? "true" : "false");
		break;

	case 'F':
		snprintf(vbuf, sizeof(vbuf), "%f", (*mValue.F));
		break;

	case 'S':
		return *(mValue.S);
		break;

	case 'C':
		snprintf(vbuf, sizeof(vbuf), "%X", *(mValue.C));
		break;

	default:
		vbuf[0] = '\0';

		LogE("Unknown Value Type : %c. Use i(nt)/b(ool)/f(loat)/s(tring)/c(olor) only.\n", mValType);
		break;
	}

	return vbuf;
}

const char *KeyValuePair::TAG = "KeyValuePair";
