/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file name : comment_example.h
* @author : alex
* @date : 2017/7/14 14:29
* @brief : open a file
*/
#ifndef SRC_FILEMAP_H_
#define SRC_FILEMAP_H_


#include <sys/types.h>

class FileMap {
public:
    FileMap(void);

    /*
     * Create a new mapping on an open file.
     *
     * Closing the file descriptor does not unmap the pages, so we don't
     * claim ownership of the fd.
     *
     * Returns "false" on failure.
     */
    bool create(const char* origFileName, int fd, off64_t offset = 0, size_t length = 0, bool readOnly = true);

    /*
     * Return the name of the file this map came from, if known.
     */
    const char* getFileName(void) const;

    /*
     * Get a pointer to the piece of the file we requested.
     */
    void* getDataPtr(void) const;

    /*
     * Get the length we requested.
     */
    size_t getDataLength(void) const;

    /*
     * Get the data offset used to create this map.
     */
    off64_t getDataOffset(void) const;

    /*
     * Get a "copy" of the object.
     */
    FileMap* acquire(void);

    /*
     * Call this when mapping is no longer needed.
     */
    void release(void);

protected:
    // don't delete objects; call release()
    ~FileMap(void);

private:
    static const char *TAG;

    int         mRefCount;      // reference count
    char*       mFileName;      // original file name, if known
    void*       mBasePtr;       // base of mmap area; page aligned
    size_t      mBaseLength;    // length, measured from "mBasePtr"
    off64_t     mDataOffset;    // offset used when map was created
    void*       mDataPtr;       // start of requested data, offset from base
    size_t      mDataLength;    // length, measured from "mDataPtr"

    static long mPageSize;
};




#endif /* SRC_FILEMAP_H_ */
