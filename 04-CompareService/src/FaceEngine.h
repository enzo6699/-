#ifndef _FI_FACE_IDENTIFY_H_
#define _FI_FACE_IDENTIFY_H_

#include <opencv/cv.h>
#include <memory>
#include <vector>
#include <stdint.h>
#ifndef USE_SEETA
#include <faceall_common.h>
#endif

class VIPLFaceDetector;
class VIPLPointDetector;
class VIPLFaceRecognizer;
class VIPLQualityAssessment;

namespace tce
{

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
    float quality = 0;
    cv::Rect rect;
    std::vector<cv::Point2d> landmarks;
    std::vector<float> feature;

    /*
    性别: 男-1 女-0
    年龄:（婴儿0-2岁）- 0 （儿童3-12岁）- 1 （少年13-18岁）- 2 （青年19-25）-4 （中青年26-35）-5 （中年36-45）-6 （中老年46-60）-7 （老年61+）-8
    颜色: 黑-0 灰-1 白-2 红3 橙-4 黄-5 绿-6 棕-7 蓝-8 紫-9
    眼镜: 无-0 普通眼镜-1 太阳镜-2
    帽子: 无-0 普通帽子-1 头盔-2 头巾-3
    面部遮挡: 无-0 口罩-1 面纱-2 大胡子-3
     */
    int age = 0;
    int gender = 0;
    int glass = 0;
    int color = 0;
    int hat = 0;
    int mask = 0;
};

class FaceEngine
{
public:
    FaceEngine();
    ~FaceEngine();

    int init(int types);

    std::vector<std::shared_ptr<FaceResult>> detect(cv::Mat &mat, const cv::Rect &roi = cv::Rect());
    int getAttribute(cv::Mat &mat, std::shared_ptr<FaceResult> face);
    int getQuality(cv::Mat &mat, std::shared_ptr<FaceResult> face);
    int feature(cv::Mat &mat, std::shared_ptr<FaceResult> face);
    int compare(std::shared_ptr<FaceResult> faceA, std::shared_ptr<FaceResult> faceB, float &similarity);
    int compare(const std::vector<float> &featureA, const std::vector<float> &featureB, float &similarity);

    static std::shared_ptr<FaceResult> getMaxFace(std::vector<std::shared_ptr<FaceResult>> &faces);

private:  
    void cleanup();

#ifdef USE_SEETA
    std::shared_ptr<VIPLFaceDetector> mDetector;
    std::shared_ptr<VIPLPointDetector> mPointDetector;
    std::shared_ptr<VIPLFaceRecognizer> mRecognizer;
    std::shared_ptr<VIPLQualityAssessment> mQualityDetector;
#else
    faceall_handle_t detectHandle = NULL;
    faceall_handle_t featureHandle = NULL;
    faceall_handle_t compareHandle = NULL;
    faceall_handle_t attributeHandle = NULL;
    faceall_handle_t qualityHandle = NULL;
#endif
};

}

#endif
