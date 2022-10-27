/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : FileHandlerBase.h
 * @author : Benson
 * @date : Sep 6, 2017
 * @brief : All base classes for json file handler.
 */

#ifndef SRC_JSON_FILEHANDLERBASE_H_
#define SRC_JSON_FILEHANDLERBASE_H_

#include "ScheduleObjs.h"
#include <string>
#include <list>
#include <map>

namespace Json
{

class FileBase
{
public:
    FileBase() :
            mFileName("")
    {
    }

    FileBase(const std::string& fileName) :
            mFileName(fileName)
    {
    }

    virtual ~FileBase()
    {
    }

    void SetFileName(const std::string& fileName)
    {
        mFileName = fileName;
    }

    /**
     * Save the file in json format.
     */
    virtual void Save() = 0;

protected:
    std::string mFileName;
};

class UpdateListFileBase: public FileBase
{
public:
    UpdateListFileBase()
    {
    }

    UpdateListFileBase(const std::string& fileName) :
            FileBase(fileName)
    {
    }

    virtual ~UpdateListFileBase()
    {
    }

    /**
     * Update list to mSchedulesTree and bring out the ScheduleList data.
     * @param jsonStr[in]: The json format string reply of 'Get schedule list'[section 2.2.1].
     * @param data[out]: The address of a ScheduleList which bring out the ScheduleList data.
     * @return
     */
    virtual bool UpdateList(const char* jsonStr, ScheduleList* data) = 0;

    /**
     * Compare the new list with old list,check weather schedule updated and bring out the updated schedule id.
     * @param newlist[in]: the new schedule list.
     * @param oldlist[in]: the old schedule list.
     * @param updatelist[out]: the updated schedule id list.
     * @param deletlist[out]: the deleted schedule id list.
     * @return false: no schedule changed.
     *         true: schedule updated or deleted.
     */
    virtual bool CheckUpdateStatus(const ScheduleList& newlist,
            const ScheduleList& oldlist,std::list<int>& keeplist, std::list<int>& updatelist,
            std::list<int>& deletlist) = 0;

    /**
     * Load all schedule list from file.
     * @param fileName[in] : Name of the file to load.
     * @param schlist[out]: The schedule list load from file.
     * @return
     */
    virtual bool LoadSchlist(const std::string& fileName,
            ScheduleList& schlist) = 0;
};

class ScheduleFileBase: public FileBase
{
public:
    ScheduleFileBase()
    {
    }
    ScheduleFileBase(const std::string& fileName) :
            FileBase(fileName)
    {
    }
    virtual ~ScheduleFileBase()
    {
    }
    /**
     * Update schedule detail to mScheduleDetails and bring out the ScheduleDetail data.
     * @param jsonStr[in]: The json format string reply of 'Get schedule detail'[section 2.2.2].
     * @param data[out]: The address of a ScheduleDetail which bring out the ScheduleDetail data.
     * @return
     */
    virtual bool UpdateScheduleDetail(const char* jsonStr,
            ScheduleDetail* data) = 0;

    /**
     * Load all schedule detail to map from file.
     * @param fileName[in] : Name of the file to load.
     * @param allSchDetail[out]: The map of schedule detail, [key] is layout id,[value] is the ScheduleDetail.
     * @return
     */
    virtual bool LoadScheduleDetail(const std::string& fileName,
            std::map<int, ScheduleDetail>& allSchDetail) = 0;

    /**
     * Load all schedule detail to map from file.
     * @param fileName[in] : Name of the file to load.
     * @param allSchDetail[out]: The map of schedule detail, [key] is layout id,
     *                           [value] is the address of ScheduleDetail.!! It is new inside,don't forget to delete!!.
     * @return
     */
    virtual bool LoadScheduleDetail(const std::string& fileName,
            std::map<int, ScheduleDetail*>& allSchDetail) = 0;
};

class LayoutGroupFileBase: public FileBase
{
public:
    LayoutGroupFileBase()
    {
    }
    LayoutGroupFileBase(const std::string& fileName) :
            FileBase(fileName)
    {
    }

    virtual ~LayoutGroupFileBase()
    {
    }

    /**
     * Update layout group detail to mLayoutDetails and bring out the LayoutDetails data.
     * @param jsonStr[in]: The json format string reply of 'Get layout group detail'.
     * @param data[in,out]: The address of a LayoutGroupDetail which bring out the LayoutGroupDetail data.
     * @return
     */
    virtual bool UpdateLayoutGroupDetail(const char* jsonStr,
            LayoutGroupDetail* data) = 0;

    /**
     * Load all layout group detail to map from file.
     * @param fileName[in] : Name of the file to load.
     * @param allGroupDetail[out]: The map of layout group detail, [key] is layout group id,[value] is the LayoutGroupDetail.
     * @return
     */
    virtual bool LoadLayoutGroupDetail(const std::string& fileName,
            std::map<int, LayoutGroupDetail>& allGroupDetail) = 0;

    /**
     * Load all layout group detail to map from file.
     * @param fileName[in] : Name of the file to load.
     * @param allGroupDetail[out]: The map of layout group detail, [key] is layout group id,
     *                              [value] is the address of LayoutGroupDetail.!! It is new inside,don't forget to delete!!.
     * @return
     */
    virtual bool LoadLayoutGroupDetail(const std::string& fileName,
            std::map<int, LayoutGroupDetail*>& allGroupDetail) = 0;

};

class LayoutFileBase: public FileBase
{
public:
    LayoutFileBase()
    {
    }
    LayoutFileBase(const std::string& fileName) :
            FileBase(fileName)
    {
    }
    virtual ~LayoutFileBase()
    {
    }

    /**
     * Update layout detail to mLayoutDetails and bring out the LayoutDetails data.
     * @param jsonStr[in]: The json format string reply of 'Get layout detail'[section 2.3.1].
     * @param data[in,out]: The address of a LayoutDetail which bring out the LayoutDetail data.
     * @return
     */
    virtual bool UpdateLayoutDetail(const char* jsonStr,
            LayoutDetail* data) = 0;

    /**
     * Load all layout detail to map from file.
     * @param fileName[in] : Name of the file to load.
     * @param allLayoutDetail[out]: The map of layout detail, [key] is layout id,[value] is the LayoutDetail.
     * @return
     */
    virtual bool LoadLayoutDetail(const std::string& fileName,
            std::map<int, LayoutDetail>& allLayoutDetail) = 0;

    /**
     * Load all layout detail to map from file.
     * @param fileName[in] : Name of the file to load.
     * @param allLayoutDetail[out]: The map of layout detail, [key] is layout id,
     *                              [value] is the address of LayoutDetail.!! It is new inside,don't forget to delete!!.
     * @return
     */
    virtual bool LoadLayoutDetail(const std::string& fileName,
            std::map<int, LayoutDetail*>& allLayoutDetail) = 0;
};

class MediaFileBase: public FileBase
{
public:
    MediaFileBase()
    {
    }

    MediaFileBase(const std::string& fileName) :
            FileBase(fileName)
    {
    }

    virtual ~MediaFileBase()
    {
    }

    /**
     * Update media detail to mMediaDetials and new a certain type data.
     * @param jsonStr[in]: The json format string reply of 'Get media detail'[section 2.4.1].
     * @param type[out]: The type of the media.
     * @param data[in,out]: it new inside this function!!!!! Don't forget delete outside!!!!!!!
     * @return
     */
    virtual bool UpdateMediaDetail(const char* jsonStr,
            MediaBasic::MediaType& type, MediaBasic** ppData) = 0;

    /**
     * Load all media information to map from file.
     * @param fileName[in] : Name of the file to load.
     * @param allMedias[out]: The map of media detail,
     *                      [key] is media id;
     *                      [value] is the MediaDetail*,
     *                      it new inside this function!!!! Don't forget delete outside!!!!
     * @return
     */
    virtual bool LoadMediaDetail(const std::string& fileName,
            std::map<int, MediaBasic*>& allMedias) = 0;
};

}

#endif /* SRC_JSON_FILEHANDLERBASE_H_ */
