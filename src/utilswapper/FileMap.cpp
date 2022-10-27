/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/

#include "FileMap.h"
#include "Log.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include <string.h>
#include <memory.h>
#include <errno.h>
#include <assert.h>

/*
 * Constructor.  Create an empty object.
 */
FileMap::FileMap(void)
    : mRefCount(1), mFileName(NULL), mBasePtr(NULL), mBaseLength(0),
      mDataOffset(0), mDataPtr(NULL), mDataLength(0)
{
}

/*
 * Destructor.
 */
FileMap::~FileMap(void)
{
    assert(mRefCount == 0);

    mRefCount = -100;       // help catch double-free
    if (mFileName != NULL) {
        free(mFileName);
    }

    if (mBasePtr && munmap(mBasePtr, mBaseLength) != 0) {
        LogD("munmap(%p, %d) failed\n", mBasePtr, (int) mBaseLength);
    }
}

/*
 * Create a new mapping on an open file.
 *
 * Closing the file descriptor does not unmap the pages, so we don't
 * claim ownership of the fd.
 *
 * Returns "false" on failure.
 */
bool FileMap::create(const char* origFileName, int fd, off64_t offset, size_t length, bool readOnly)
{
    int     prot, flags, adjust;
    off64_t adjOffset;
    size_t  adjLength;

    void* ptr;

    assert(mRefCount == 1);
    assert(fd >= 0);
    assert(offset >= 0);

    struct stat fs;
    int ret = fstat(fd, &fs);
    if (ret != 0) {
    	LogE("Failed to get file size. file = %s[%d]\n", origFileName == NULL ? "" : origFileName, fd);
    	return false;
    }

    if (length <= 0) {
    	length = fs.st_size - offset;
    }

	if (length <= 0 || offset + length > (size_t)fs.st_size) {
		LogE("Request out of file size : offset = %ld length = %ld file size = %ld\n", (long)offset, (long)length, (long)fs.st_size);
		return false;
	}

    /* init on first use */
    if (mPageSize == -1) {
        /* this holds for Linux, Darwin, Cygwin, and doesn't pain the ARM */
        mPageSize = 4096;
    }

    adjust   = offset % mPageSize;
try_again:
    adjOffset = offset - adjust;
    adjLength = length + adjust;

    flags = MAP_SHARED;
    prot = PROT_READ;
    if (!readOnly)
        prot |= PROT_WRITE;

    ptr = mmap(NULL, adjLength, prot, flags, fd, adjOffset);
    if (ptr == MAP_FAILED) {
    	// Cygwin does not seem to like file mapping files from an offset.
    	// So if we fail, try again with offset zero
    	if (adjOffset > 0) {
    		adjust = offset;
    		goto try_again;
    	}

        LogE("mmap(%ld,%ld) failed: %s\n", (long) adjOffset, (long) adjLength, strerror(errno));
        return false;
    }
    mBasePtr = ptr;

    mFileName = origFileName != NULL ? strdup(origFileName) : NULL;
    mBaseLength = adjLength;
    mDataOffset = offset;
    mDataPtr = (char*) mBasePtr + adjust;
    mDataLength = length;

    assert(mBasePtr != NULL);

    LogV("MAP: base %p/%d data %p/%d\n", mBasePtr, (int) mBaseLength, mDataPtr, (int) mDataLength);

    return true;
}

const char* FileMap::getFileName(void) const
{
	return mFileName;
}

void* FileMap::getDataPtr(void) const
{
	return mDataPtr;
}

size_t FileMap::getDataLength(void) const
{
	return mDataLength;
}

off64_t FileMap::getDataOffset(void) const
{
	return mDataOffset;
}

FileMap* FileMap::acquire(void)
{
	mRefCount++;
	return this;
}

void FileMap::release(void)
{
	if (--mRefCount <= 0) {
		delete this;
	}
}

const char *FileMap::TAG = "FileMap";
long FileMap::mPageSize = -1;

