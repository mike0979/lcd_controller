#ifndef PARCEL_H_
#define PARCEL_H_

#include <stdint.h>
#include <sys/types.h>

#include <string>

class Parcel {
public:
	Parcel();
	Parcel(uint8_t* data, size_t len);
	~Parcel();

    const uint8_t* data() const;
    size_t dataSize() const;
    size_t dataAvail() const;
    size_t dataPosition() const;
    size_t dataCapacity() const;

    int setData(uint8_t* data, size_t len);

    int write(const void* data, size_t len);
    int writeInt32(int32_t val);
    int writeInt64(int64_t val);
    int writeFloat(float val);
    int writeDouble(double val);
    int writeString(const char* str);
    int writeString(const std::string &str);

    int read(void* outData, size_t len);
    int readInt32(int32_t *val);
    int readInt64(int64_t *val);
    int readFloat(float *val);
    int readDouble(double *val);
    const char* readString();
    int readString(std::string &str);

    int seek(size_t pos);
private:
    void* writeInplace(size_t len);
    int finishWrite(size_t len);
    int growData(size_t len);

    template<class T> int readAligned(T *pArg) const;

    template<class T> T readAligned() const;

    template<class T> int writeAligned(T val);

private:
    uint8_t* mData;
    size_t mDataSize;
    size_t mDataCapacity;
    mutable size_t mDataPos;

    static const char *TAG;
};

#endif /* PARCEL_H_ */
