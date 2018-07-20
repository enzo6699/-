#ifndef __DATACACHE_H__
#define __DATACACHE_H__

#include "FaceEngine.h"
#include "PropertyConfigurator.h"
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <log4cpp/Category.hh>

struct SearchResult {
    std::string personId;
    std::string personType;
    std::string personName;
    std::string faceId;
    std::string objectUrl;
    float confidence;
};

struct FaceAttribute {
    float age;
    int race;
    int gender;
};

struct CollectFace {
    std::string collectFaceId;
    std::string cameraId;
    std::string feature;
    std::string originalUrl;
    std::string objectUrl;
    std::string timestamp;
    FaceAttribute attribute;
};

class DataCache
{
public:

    struct Face {
        std::string faceId;
        std::string objectUrl;
        std::string feature;
    };

    struct Person {
        std::string personId;
        std::string personType;
        std::string personName;
        std::map<std::string, std::shared_ptr<Face>> faces;
    };

    DataCache();
    ~DataCache();

    // 返回添加的人脸个数
    int add(std::shared_ptr<DataCache::Person> person);
    // 返回删除的人脸个数
    int remove(const std::string &personId, const std::vector<std::string> &faceIds);
    int updatePerson(const std::string &personId, const std::string &personType, const std::string &personName);
    std::vector<std::shared_ptr<SearchResult>> findFace(const std::vector<float> &feature, const std::string &personType, int topN);
    std::string getLastError() { return errorMessage; }
    std::vector<std::string> getTypes();

private:
    std::map<std::string, std::shared_ptr<DataCache::Person>> persons;
    std::mutex lock;
    std::string errorMessage;
    log4cpp::Category &logger;
    PropertyConfigurator &config;
    std::shared_ptr<tce::FaceEngine> faceall;
};

#endif // __DATACACHE_H__
