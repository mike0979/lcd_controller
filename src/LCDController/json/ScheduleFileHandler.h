/**
* @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
* @file : ScheduleHandler.h 
* @author : Benson
* @date : Sep 6, 2017
* @brief :
*/

#ifndef SRC_JSON_RAPIDJSON_SCHEDULEHANDLER_H_
#define SRC_JSON_RAPIDJSON_SCHEDULEHANDLER_H_

#include "FileHandlerBase.h"
#include "rapidjson/document.h"
#include <vector>
#include <map>

namespace Json
{
class ScheduleFileHandler : public ScheduleFileBase
{
public:
    ScheduleFileHandler();
    ScheduleFileHandler(const std::string& fileName);
    virtual ~ScheduleFileHandler();

    void SetFileName(const std::string& fileName);

    /**
     * Update schedule to mDocs and bring out the ScheduleDetail data.
     * @param jsonStr[in]: The json format string reply of 'Get schedule detail'[section 2.2.2].
     * @param data[out]: The address of a ScheduleDetail which bring out the ScheduleDetail data.
     * @return
     */
    virtual bool UpdateScheduleDetail(const char* jsonStr, ScheduleDetail* data);

    /**
     * Find ScheduleDetail with given id.
     * @param id[in]: The id need to be found.
     * @param data[int,out]: The address of a LayoutDetail which bring out the LayoutDetail data.
     * @return
     */
    virtual bool Find(int id,ScheduleDetail* data);

    /**
     * Delete ScheduleDetail with given id.
     * @param id[in]: The id need to be deleted.
     * @return
     */
    virtual bool Delete(int id);

    /**
     * Load all schedule detail to map from file.
     * @param fileName[in] : Name of the file to load.
     * @param allSchDetail[out]: The map of schedule detail, [key] is layout id,[value] is the ScheduleDetail.
     * @return
     */
    virtual bool LoadScheduleDetail(const std::string& fileName,
            std::map<int, ScheduleDetail>& allSchDetail);

    /**
     * Load all schedule detail to map from file.
     * @param fileName[in] : Name of the file to load.
     * @param allSchDetail[out]: The map of schedule detail, [key] is layout id,
     *                           [value] is the address of ScheduleDetail.!! It is new inside,don't forget to delete!!.
     * @return
     */
    virtual bool LoadScheduleDetail(const std::string& fileName,
            std::map<int, ScheduleDetail*>& allSchDetail);

    /**
     * Save the to file in json format.
     */
    virtual void Save();

private:
   bool GetScheduleDetail(rapidjson::Value& doc, ScheduleDetail* data);

   std::vector<rapidjson::Document*> mDocs;
   static const char *TAG;
};

}



#endif /* SRC_JSON_RAPIDJSON_SCHEDULEHANDLER_H_ */
