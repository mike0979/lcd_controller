/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : LayoutGroupHandler.h
 * @author : Benson
 * @date : Oct 30, 2017
 * @brief :
 */

#ifndef JSON_LAYOUTGROUPFILEHANDLER_H_
#define JSON_LAYOUTGROUPFILEHANDLER_H_

#include <json/FileHandlerBase.h>
#include <json/ScheduleObjs.h>
#include <rapidjson/document.h>
#include <map>
#include <string>
#include <vector>

namespace Json
{
class LayoutGroupFileHandler: public LayoutGroupFileBase
{
public:
    LayoutGroupFileHandler();

    LayoutGroupFileHandler(const std::string& fileName);

    virtual ~LayoutGroupFileHandler();

    void SetFileName(const std::string& fileName);

    /**
     * Find LayoutDetail with given id.
     * @param id[in]: The id need to be found.
     * @param data[int,out]: The address of a LayoutDetail which bring out the LayoutDetail data.
     * @return
     */
    virtual bool Find(int id, LayoutGroupDetail* data);

    /**
     * Delete LayoutDetail with given id.
     * @param id[in]: The id need to be deleted.
     * @return
     */
    virtual bool Delete(int id);

    /**
     * Update layout group detail to mLayoutDetails and bring out the LayoutDetails data.
     * @param jsonStr[in]: The json format string reply of 'Get layout group detail'.
     * @param data[in,out]: The address of a LayoutGroupDetail which bring out the LayoutGroupDetail data.
     * @return
     */
    virtual bool UpdateLayoutGroupDetail(const char* jsonStr,
            LayoutGroupDetail* data);

    /**
     * Load all layout group detail to map from file.
     * @param fileName[in] : Name of the file to load.
     * @param allGroupDetail[out]: The map of layout group detail, [key] is layout group id,[value] is the LayoutGroupDetail.
     * @return
     */
    virtual bool LoadLayoutGroupDetail(const std::string& fileName,
            std::map<int, LayoutGroupDetail>& allGroupDetail);

    /**
     * Load all layout group detail to map from file.
     * @param fileName[in] : Name of the file to load.
     * @param allGroupDetail[out]: The map of layout group detail, [key] is layout group id,
     *                              [value] is the address of LayoutGroupDetail.!! It is new inside,don't forget to delete!!.
     * @return
     */
    virtual bool LoadLayoutGroupDetail(const std::string& fileName,
            std::map<int, LayoutGroupDetail*>& allGroupDetail);

    /**
     * Save the to file in json format.
     */
    virtual void Save();

private:
    bool GetLayoutGroupDetail(rapidjson::Value& obj, LayoutGroupDetail* data);

    std::vector<rapidjson::Document*> mDocs;
};
}

#endif /* JSON_LAYOUTGROUPFILEHANDLER_H_ */
