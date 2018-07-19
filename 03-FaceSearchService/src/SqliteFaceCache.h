#ifndef _SqliteFaceCache_H_
#define _SqliteFaceCache_H_

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <log4cpp/Category.hh>
#include <sqlite3.h>
#include <atomic>

typedef struct _face_info_t {
    _face_info_t()
    {
        feature = NULL;
        featureLength = 0;
    }

    virtual~_face_info_t()
    {
        if (feature != NULL)
        {
            delete[] feature;
            feature = NULL;
        }
        featureLength = 0;
    }

    std::string personId;
    std::string cameraId;
    std::string originalUrl;
    std::string objectUrl;
    int timestamp;
    float* feature;
    int featureLength;
} face_info_t;

class SqliteFaceCache
{
public:
    SqliteFaceCache();
    ~SqliteFaceCache();

public:
    int init(const char *filename);

    /*
    * 插入数据库
    */
    int Insert(const char* personId,
               const char* cameraId,
               const char* originalUrl,
               const char* objectUrl,
               const std::string &feature,
               const std::string &objectRect,
               float objectQuality,
               int timestamp);

    /*
    * 批量插入数据库
    * vecFaceInfo 要插入数据库集合
    */
//    int Insert(std::vector<std::shared_ptr<face_info_t>> &vecFaceInfo);

    /*
    * 生成查询语句
    * startTime 开始时间
    * endTime 结束时间
    * vecCameraId 摄像头id集合
    */
    std::string Search(int startTime, int endTime, std::vector<std::string> vecCameraId);

    int SearchCount(int startTime, int endTime, std::vector<std::string> vecCameraId);

    int Delete(const char* personId);

    /*
    * 删除过期数据
    * retentionTime 保留时间数据，单位秒
    */

    sqlite3 *m_db = NULL;

private:
    void checkOverdue();
    int DeleteOverdue(int retentionTime);

    std::atomic<bool> mRunning;
    std::shared_ptr<std::thread> deleter;
};

#endif
