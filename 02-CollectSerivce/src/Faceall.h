#ifndef _FI_FACE_IDENTIFY_H_
#define _FI_FACE_IDENTIFY_H_

#include <faceall_common.h>
#include <memory>
#include <vector>
#include <stdint.h>
#include <log4cpp/Category.hh>
#include <opencv/cv.h>

namespace tce {
enum FaceEngineType
{
    FaceDetect = 0x01,//人脸检测
    FaceFeature = 0x02,//特征提取
    FaceCompare = 0x04,//特征比对
    FacePose = 0x08,//人脸位置函数
    FaceAttribute = 0x10,
    FaceQuality = 0x20,
    FaceAll = 0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20,//所有属性
};

struct FaceResult
{
    FaceResult();
    ~FaceResult();

    cv::Rect rect;
    float quality = 0;

    // TODO: 使用opencv数据结构以兼容非飞搜算法
    faceall_landmark_t landmark;
    faceall_feature_t feature;
    faceall_attribute_t attribute;
};

class FaceEngineWrapper
{
public:
    FaceEngineWrapper();
    ~FaceEngineWrapper();

    int Init(int types);
    void Uninit();

    std::vector<std::shared_ptr<FaceResult>> Detect(cv::Mat &mat, const cv::Rect &roi = cv::Rect(), bool detectQuality = false);
    int Feature(cv::Mat &mat, std::shared_ptr<FaceResult> &face);
    int Compare(float* featureA, int lengthA, float* featureB, int lengthB, float &similarity);

private:
    faceall_handle_t detectHandle;
    faceall_handle_t featureHandle;
    faceall_handle_t compareHandle;
    faceall_handle_t attributeHandle;
    faceall_handle_t qualityHandle;

    log4cpp::Category &logger;
};

}

#endif
