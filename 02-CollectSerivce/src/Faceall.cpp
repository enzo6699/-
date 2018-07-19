#include "Faceall.h"
#include <faceall_sdk.h>
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv/cv.h"
#include "utils/TimeUtil.h"


namespace tce {

/////////////////////////////////////////////////////////////////////////////////
FaceResult::FaceResult()
{
    landmark.n_points = 0;
    landmark.points = NULL;
    feature.length = 0;
    feature.feature = NULL;
}

FaceResult::~FaceResult()
{
    if (landmark.n_points != 0)
    {
        if (landmark.points != NULL)
        {
            delete[] landmark.points;
            landmark.points = NULL;
        }
    }

    if (feature.feature != NULL)
    {
        feature.length = 0;
        faceall_facesdk_feature_release_result(feature);
    }
}
/////////////////////////////////////////////////////////////////////////////////

FaceEngineWrapper::FaceEngineWrapper()
    : detectHandle(NULL)
    , featureHandle(NULL)
    , compareHandle(NULL)
    , attributeHandle(NULL)
    , logger(log4cpp::Category::getRoot())
{
}

FaceEngineWrapper::~FaceEngineWrapper()
{
    Uninit();
}

int FaceEngineWrapper::Init(int types)
{
    int nRet = 0;
    if (types & FaceDetect)
    {
        logger.info("face detect and landmark");
        detectHandle = faceall_facesdk_detect_and_landmark_get_instance();
        if (detectHandle == NULL)
        {
            logger.info("face_detect_landmark failed");
            nRet = 1;
        }
    }

    if (types & FaceFeature)
    {
        logger.info("face_feature");
        featureHandle = faceall_facesdk_feature_get_instance(FACEALL_compare_type_idfea);
        if (featureHandle == NULL)
        {
            logger.info("face_feature failed");
            nRet = 2;
        }
    }

    if (types & FaceCompare)
    {
        logger.info("face_compare");
        compareHandle = faceall_facesdk_compare_get_instance(FACEALL_compare_type_idfea);
        if (compareHandle == NULL)
        {
            logger.info("face_compare failed");
            nRet = 3;
        }
    }

    if (types & FaceAttribute) {
        logger.info("faceall_attribute");
        attributeHandle = faceall_facesdk_attribute_get_instance();
        if (attributeHandle == NULL)
        {
            logger.error("faceall_attribute failed");
            nRet = 4;
        }
    }

    if (types & FaceQuality) {
        logger.info("faceall_quality");
        qualityHandle = faceall_facesdk_facequality_get_instance();
        if (qualityHandle == NULL) {
            logger.error("faceall_quality failed");
            nRet = 5;
        }
    }

    if (nRet != 0)
        Uninit();

    return nRet;
}

void FaceEngineWrapper::Uninit()
{
    if (detectHandle != NULL)
    {
        logger.info("Destroy detect and landmark instance");
        faceall_facesdk_detect_and_landmark_destroy_instance(detectHandle);
        detectHandle = NULL;
    }

    if (featureHandle != NULL)
    {
        logger.info("Destroy feature instance");
        faceall_facesdk_feature_destroy_instance(featureHandle);
        featureHandle = NULL;
    }

    if (compareHandle != NULL)
    {
        logger.info("Destroy compare instance");
        faceall_facesdk_compare_destroy_instance(compareHandle);
        compareHandle = NULL;
    }

    if (attributeHandle != NULL)
    {
        logger.info("Destroy attribute instance");
        faceall_facesdk_attribute_destroy_instance(attributeHandle);
        attributeHandle = NULL;
    }

    if (qualityHandle != NULL)
    {
        logger.info("Destroy quality instance");
        faceall_facesdk_facequality_destroy_instance(qualityHandle);
        qualityHandle = NULL;
    }

    logger.info("Finished destroy faceall");
}

std::vector<std::shared_ptr<FaceResult>> FaceEngineWrapper::Detect(cv::Mat &mat, const cv::Rect &roi, bool detectQuality)
{
    std::vector<std::shared_ptr<FaceResult>> faces;

    bool useROI = false;
    unsigned int count = 0;
    faceall_result_t result = 0;
    faceall_image_t image;
    faceall_rect_t *rects = NULL;
    faceall_landmark_t *landmarks = NULL;
    image.channels = mat.channels();
    image.width = mat.cols;
    image.height = mat.rows;
    image.data = mat.data;

    if (roi.width > 0 && roi.height >  0) {
        logger.debug("Using roi: %d,%d,%d,%d", roi.x, roi.y, roi.width, roi.height);
        useROI = true;
        cv::Mat target;
        mat(roi).copyTo(target);
        image.channels = target.channels();
        image.width = target.cols;
        image.height = target.rows;
        image.data = target.data;
    }

#ifdef USE_PRINTF_TIME
    int64_t t1 = TimeUtil::GetTimestamp();
#endif
    result = faceall_facesdk_detect_and_landmark(detectHandle, image, &rects, &landmarks, &count);
#ifdef USE_PRINTF_TIME
    int64_t t2 = TimeUtil::GetTimestamp();
    logger.info("Time on detect and landmark: %lldms", t2 - t1);
#endif

    if (result != 0)
    {
        faceall_facesdk_detect_and_landmark_release_result(rects, landmarks, count);
        logger.error("faceall_facesdk_detect_and_landmark %d", result);
        return faces;
    }

	for (unsigned int index = 0; index < count; index++) {
        std::shared_ptr<FaceResult> details(new FaceResult);

        const faceall_rect_t &rect = rects[index];

        details->rect.width = rect.right - rect.left;
        details->rect.height = rect.bottom - rect.top;
        if (useROI) {
            details->rect.x = rect.left + roi.x;
            details->rect.y = rect.top + roi.y;
        } else {
            details->rect.x = rect.left;
            details->rect.y = rect.top;
        }

        const faceall_landmark_t &landmark = landmarks[index];
        details->landmark.n_points = landmark.n_points;
        details->landmark.points = new faceall_pointi_t[landmark.n_points];
        for (int index2 = 0; index2 < landmark.n_points; index2++) {
            if (useROI) {
                details->landmark.points[index2].x = landmark.points[index2].x + roi.x;
                details->landmark.points[index2].y = landmark.points[index2].y + roi.y;
            } else {
                details->landmark.points[index2] = landmark.points[index2];
            }
        }

        if (attributeHandle) {
#ifdef USE_PRINTF_TIME
    int64_t t1 = TimeUtil::GetTimestamp();
#endif
            result = faceall_facesdk_attribute(attributeHandle, image, rect, &details->attribute);
#ifdef USE_PRINTF_TIME
    int64_t t2 = TimeUtil::GetTimestamp();
    logger.info("Time on attribute: %lldms", t2 - t1);
#endif
            if (result != 0) {
                logger.error("'faceall_facesdk_attribute' failed: %d", result);
                memset(&details->attribute, 0, sizeof(details->attribute));
                continue;
            }
        }

        if (detectQuality && qualityHandle) {
            result = faceall_facesdk_facequality(qualityHandle, image, landmark, &details->quality);
            if (result != 0) {
                logger.error("'faceall_facesdk_facequality' failed: %d", result);
                details->quality = 0;
                continue;
            }
        }

        faces.push_back(details);
    }

    faceall_facesdk_detect_and_landmark_release_result(rects, landmarks, count);

    return faces;
}

int FaceEngineWrapper::Feature(cv::Mat &mat, std::shared_ptr<FaceResult> &face)
{
    faceall_result_t result;
    faceall_image_t faceall_image;
    faceall_image.channels = mat.channels();
    faceall_image.width = mat.cols;
    faceall_image.height = mat.rows;
    faceall_image.data = mat.data;

    faceall_feature_t faceall_feature;

#ifdef USE_PRINTF_TIME
    int64_t t1 = tce::TimeUtil::GetTimestamp();
#endif
    result = faceall_facesdk_feature(featureHandle, faceall_image, face->landmark, &faceall_feature);
#ifdef USE_PRINTF_TIME
    int64_t t2 = tce::TimeUtil::GetTimestamp();
    logger.info("Time on feature: %lldms", t2 - t1);
#endif

    if (result != 0)
    {
        faceall_facesdk_feature_release_result(faceall_feature);
        logger.error("faceall_facesdk_feature failed: %d", result);
        return result;
    }

    face->feature = faceall_feature;
    return 0;
}

int FaceEngineWrapper::Compare(float* featureA, int lengthA, float* featureB, int lengthB, float &similarity)
{
    faceall_feature_t tfeatureA;
    tfeatureA.feature = featureA;
    tfeatureA.length = lengthA;

    faceall_feature_t tfeatureB;
    tfeatureB.feature = featureB;
    tfeatureB.length = lengthB;

    return faceall_facesdk_compare(compareHandle, tfeatureA, tfeatureB, &similarity);
}
}
