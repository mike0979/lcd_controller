#include "Parcel.h"
#include "Errors.h"
#include "Log.h"

#include <stdlib.h>
#include <string.h>

#define PAD_SIZE(s)		(((s)+3)&~3)

Parcel::Parcel()
{
	mData = NULL;

	setData(NULL, 0);
}

Parcel::Parcel(uint8_t* data, size_t len)
{
	mData = NULL;

	setData(data, len);
}

Parcel::~Parcel()
{
	if (mData != NULL) {
		free(mData);
	}
}

const uint8_t* Parcel::data() const
{
    return mData;
}

size_t Parcel::dataSize() const
{
    return (mDataSize > mDataPos ? mDataSize : mDataPos);
}

size_t Parcel::dataAvail() const
{
    return dataSize() - dataPosition();
}

size_t Parcel::dataPosition() const
{
    return mDataPos;
}

size_t Parcel::dataCapacity() const
{
    return mDataCapacity;
}

int Parcel::setData(uint8_t* data, size_t len)
{
	if (mData != NULL) {
		free(mData);
	}

    mData = data;
    mDataSize = len;
    mDataCapacity = len;
    mDataPos = 0;

    return NO_ERROR;
}

int Parcel::write(const void* data, size_t len)
{
    void* const d = writeInplace(len);
    if (d) {
        memcpy(d, data, len);
        return NO_ERROR;
    }
    return NO_MEMORY;
}

int Parcel::writeInt32(int32_t val)
{
	return writeAligned(val);
}

int Parcel::writeInt64(int64_t val)
{
	return writeAligned(val);
}

int Parcel::writeFloat(float val)
{
	return writeAligned(val);
}

int Parcel::writeDouble(double val)
{
	return writeAligned(val);
}

int Parcel::writeString(const char* str)
{
	return write(str, strlen(str) + 1);
}

int Parcel::writeString(const std::string &str)
{
	return writeString(str.c_str());
}

int Parcel::read(void* outData, size_t len)
{
    if ((mDataPos + PAD_SIZE(len)) >= mDataPos && (mDataPos + PAD_SIZE(len)) <= mDataSize) {
        memcpy(outData, mData + mDataPos, len);
        mDataPos += PAD_SIZE(len);

        return NO_ERROR;
    }
    return NOT_ENOUGH_DATA;
}

int Parcel::readInt32(int32_t *val)
{
	return readAligned(val);
}

int Parcel::readInt64(int64_t *val)
{
	return readAligned(val);
}

int Parcel::readFloat(float *val)
{
	return readAligned(val);
}

int Parcel::readDouble(double *val)
{
	return readAligned(val);
}

const char* Parcel::readString()
{
    const size_t avail = mDataSize - mDataPos;
    if (avail > 0) {
        const char* str = reinterpret_cast<const char*>(mData + mDataPos);
        // is the string's trailing NUL within the parcel's valid bounds?
        const char* eos = reinterpret_cast<const char*>(memchr(str, 0, avail));
        if (eos) {
            const size_t len = eos - str;
            mDataPos += PAD_SIZE(len+1);
            return str;
        }
    }

    return NULL;
}

int Parcel::readString(std::string &str)
{
	const char* data = readString();
	if (data != NULL) {
		str = data;
		return NO_ERROR;
	}
	else {
		return NOT_ENOUGH_DATA;
	}
}

int Parcel::seek(size_t pos)
{
	const size_t padded = PAD_SIZE(pos);

	if (padded < mDataSize) {
		mDataPos = padded;
	}

	return mDataPos;
}

void* Parcel::writeInplace(size_t len)
{
    const size_t padded = PAD_SIZE(len);

    // sanity check for integer overflow
    if (mDataPos + padded < mDataPos) {
        return NULL;
    }

    if ((mDataPos + padded) <= mDataCapacity) {
restart_write:
        uint8_t* const data = mData + mDataPos;

        // Need to pad at end?
        if (padded != len) {
#if BYTE_ORDER == BIG_ENDIAN
            static const uint32_t mask[4] = {
                0x00000000, 0xffffff00, 0xffff0000, 0xff000000
            };
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
            static const uint32_t mask[4] = {
                0x00000000, 0x00ffffff, 0x0000ffff, 0x000000ff
            };
#endif

            *reinterpret_cast<uint32_t*>(data + padded - 4) &= mask[padded - len];
        }

        finishWrite(padded);
        return data;
    }

    int err = growData(padded);
    if (err == NO_ERROR) {
    	goto restart_write;
    }

    return NULL;
}

int Parcel::finishWrite(size_t len)
{
    mDataPos += len;

    if (mDataPos > mDataSize) {
        mDataSize = mDataPos;
    }

    return NO_ERROR;
}

int Parcel::growData(size_t len)
{
    size_t newSize = ((mDataSize + len) * 3) / 2;
    if (newSize <= mDataSize) {
    	return NO_MEMORY;
    }

    uint8_t* data = (uint8_t*)realloc(mData, newSize);
    if (data == NULL) {
    	return NO_MEMORY;
    }

    mData = data;
    mDataCapacity = newSize;
    return NO_ERROR;
}

template<class T>
int Parcel::readAligned(T *pArg) const {
    if ((mDataPos + sizeof(T)) <= mDataSize) {
        const void* data = mData + mDataPos;
        mDataPos += sizeof(T);
        *pArg =  *reinterpret_cast<const T*>(data);
        return NO_ERROR;
    } else {
        return NOT_ENOUGH_DATA;
    }
}

template<class T>
T Parcel::readAligned() const {
    T result;
    if (readAligned(&result) != NO_ERROR) {
        result = 0;
    }

    return result;
}

template<class T>
int Parcel::writeAligned(T val) {
    if ((mDataPos + sizeof(val)) <= mDataCapacity) {
restart_write:
        *reinterpret_cast<T*>(mData + mDataPos) = val;
        return finishWrite(sizeof(val));
    }

    int err = growData(sizeof(val));
    if (err == NO_ERROR) {
    	goto restart_write;
    }

    return err;
}

const char *Parcel::TAG = "Parcel";

