/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : UpdateListHandler.h
 * @author : Benson
 * @date : Sep 6, 2017
 * @brief :
 */

#ifndef SRC_JSON_RAPIDJSON_UPDATELISTHANDLER_H_
#define SRC_JSON_RAPIDJSON_UPDATELISTHANDLER_H_

#include "FileHandlerBase.h"
#include "rapidjson/document.h"

namespace Json
{
class UpdateListFileHandler: public UpdateListFileBase
{
public:
    UpdateListFileHandler();

    UpdateListFileHandler(const std::string& fileName);

    virtual ~UpdateListFileHandler();

    void SetFileName(const std::string& fileName);
    /**
     * Update list to mDoc and bring out the ScheduleList data.
     * @param jsonStr[in]: The json format string reply of 'Get schedule list'[section 2.2.1].
     * @param data[out]: The address of a ScheduleList which bring out the ScheduleList data.
     * @return
     */
    virtual bool UpdateList(const char* jsonStr, ScheduleList* data);

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
            const ScheduleList& oldlist, std::list<int>& keeplist, std::list<int>& updatelist,
            std::list<int>& deletlist);

    /**
     * Load all schedule list from file.
     * @param fileName[in] : Name of the file to load.
     * @param schlist[out]: The schedule list load from file.
     * @return
     */
    virtual bool LoadSchlist(const std::string& fileName, ScheduleList& schlist);

    /**
     * Save the basic schedule information to file in json format.
     */
    virtual void Save();
private:
    rapidjson::Document mDoc;
};

}
#endif /* SRC_JSON_RAPIDJSON_UPDATELISTHANDLER_H_ */
