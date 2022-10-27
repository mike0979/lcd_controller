/**
 * @copyright : Suzhou Huaqi Intelligent Technology Co.,Ltd
 * @file : MediaHandler.h
 * @author : Benson
 * @date : Sep 7, 2017
 * @brief :
 */

#ifndef SRC_JSON_RAPIDJSON_MEDIAHANDLER_H_
#define SRC_JSON_RAPIDJSON_MEDIAHANDLER_H_

#include "FileHandlerBase.h"
#include "rapidjson/document.h"
#include <vector>
#include <map>

namespace Json
{
class MediaFileHandler: public MediaFileBase
{
public:
    MediaFileHandler();

    MediaFileHandler(const std::string& fileName);

    virtual ~MediaFileHandler();

    void SetFileName(const std::string& fileName);

    /**
     * Find MediaBasic with given id.
     * @param id[in]: The id need to be found.
     * @param type[out]: The type of the media.
     * @param data[int,out]: it new inside this function!!!!! Don't forget delete outside!!!!!!!
     * @return
     */
    virtual bool Find(int id, MediaBasic::MediaType& type, MediaBasic** data);

    /**
     * Delete MediaBasic with given id.
     * @param id[in]: The id need to be deleted.
     * @return
     */
    virtual bool Delete(int id);

    /**
     * Update media detail to mMediaDetials and new a certain type data.
     * @param jsonStr[in]: The json format string reply of 'Get media detail'[section 2.4.1].
     * @param type[out]: The type of the media.
     * @param data[in,out]: it new inside this function!!!!! Don't forget delete outside!!!!!!!
     * @return
     */
    virtual bool UpdateMediaDetail(const char* jsonStr,
            MediaBasic::MediaType& type, MediaBasic** ppData);

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
            std::map<int, MediaBasic*>& allMedias);

    /**
     * Save the to file in json format.
     */
    virtual void Save();
private:
    bool GetMediaDetail(rapidjson::Value& obj, MediaBasic::MediaType& type,
            MediaBasic** data);

    std::vector<rapidjson::Document*> mDocs;
};
}

#endif /* SRC_JSON_RAPIDJSON_MEDIAHANDLER_H_ */
