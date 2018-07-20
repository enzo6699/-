#include "FaceEngine.h"
#include <iostream>
#include <log4cpp/Category.hh>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#ifdef USE_SEETA
#include <VIPLFaceDetector.h>
#include <VIPLFaceRecognizer.h>
#include <VIPLPointDetector.h>
#include <VIPLQualityAssessment.h>
#else
#include <faceall_sdk.h>
#endif

static log4cpp::Category &logger = log4cpp::Category::getRoot();

namespace tce {

FaceEngine::FaceEngine()
{
}

FaceEngine::~FaceEngine()
{
    cleanup();
}

int FaceEngine::init(int types)
{
#ifdef USE_SEETA

    if (types & FaceDetect) {
        if (!mDetector) {
            mDetector = std::make_shared<VIPLFaceDetector>("./seeta_models/VIPLFaceDetector5.1.2.m9d6.640x480.sta", VIPLFaceDetector::GPU);
            mDetector->SetMinFaceSize(40);
        }
        if (!mPointDetector) {
            mPointDetector = std::make_shared<VIPLPointDetector>("./seeta_models/VIPLPointDetector5.0.pts5.dat");
        }
    }

    if (types & FaceFeature || types & FaceCompare) {
        if (!mRecognizer) {
            mRecognizer = std::make_shared<VIPLFaceRecognizer>("./seeta_models/VIPLFaceRecognizer5.0.RN50.49w.s4.1N.sta", VIPLFaceRecognizer::GPU);
        }
    }

//    if (!mQualityDetector && types & FaceQuality) {
//        mQualityDetector = std::make_shared<VIPLQualityAssessment>("./seeta_models/VIPLPoseEstimation1.1.0.ext.dat");
//    }

    return 0;
#else

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
        cleanup();

    return nRet;

#endif
}

void FaceEngine::cleanup()
{
#ifdef USE_SEETA
#else
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
#endif
}

std::vector<std::shared_ptr<FaceResult>> FaceEngine::detect(cv::Mat &mat, const cv::Rect &roi)
{
    std::vector<std::shared_ptr<FaceResult>> faces;

#ifdef USE_SEETA

    VIPLImageData image;
    image.channels = mat.channels();
    image.width = mat.cols;
    image.height = mat.rows;
    image.data = mat.data;

    std::vector<VIPLFaceInfo> infos = mDetector->Detect(image);

    for (auto &info: infos) {
        std::shared_ptr<FaceResult> face(new FaceResult);
        face->rect.x = info.x;
        face->rect.y = info.y;
        face->rect.width = info.width;
        face->rect.height = info.height;

        std::vector<VIPLPoint> landmarks;
        if (mPointDetector->DetectLandmarks(image, info, landmarks)) {
            for (auto &point: landmarks) {
                face->landmarks.push_back(cv::Point2d(point.x, point.y));
            }
        }

        faces.push_back(face);
    }

#else

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

    result = faceall_facesdk_detect_and_landmark(detectHandle, image, &rects, &landmarks, &count);

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
        for (int index2 = 0; index2 < landmark.n_points; index2++) {
            int x = landmark.points[index2].x;
            int y = landmark.points[index2].y;
            if (useROI) {
                x += roi.x;
                y += roi.y;
            }
            details->landmarks.push_back(cv::Point(x, y));
        }

        faces.push_back(details);
    }

    faceall_facesdk_detect_and_landmark_release_result(rects, landmarks, count);

#endif

    return faces;
}

int FaceEngine::getAttribute(cv::Mat &mat, std::shared_ptr<FaceResult> face)
{
#ifdef USE_SEETA

    return -1;

#else

    faceall_image_t image;
    image.channels = mat.channels();
    image.width = mat.cols;
    image.height = mat.rows;
    image.data = mat.data;

    if (!attributeHandle) {
        return -1;
    }
    faceall_attribute_t attribute = { 0 };
    faceall_rect_t rect = { 0 };
    rect.left = face->rect.x;
    rect.top = face->rect.y;
    rect.right = face->rect.x + face->rect.width;
    rect.bottom = face->rect.y + face->rect.height;

    int err = faceall_facesdk_attribute(attributeHandle, image, rect, &attribute);
    if (err != 0) {
        logger.error("'faceall_facesdk_attribute' failed: %d", err);
        return err;
    }

    face->age = attribute.age;
    face->gender = attribute.gender;
    face->glass = attribute.glass;
    face->color = attribute.color;
    face->hat = attribute.hat;
    face->mask = attribute.mask;

    return 0;

#endif
}

int FaceEngine::getQuality(cv::Mat &mat, std::shared_ptr<FaceResult> face)
{
#ifdef USE_SEETA

    return -1;

#else
    if (!qualityHandle) {
        return -1;
    }
    faceall_image_t image;
    image.channels = mat.channels();
    image.width = mat.cols;
    image.height = mat.rows;
    image.data = mat.data;

    faceall_landmark_t landmark;
    landmark.n_points = face->landmarks.size();
    landmark.points = new faceall_pointi_t[landmark.n_points];
    for (int index = 0; index < landmark.n_points; index++) {
        const cv::Point2d &point = face->landmarks[index];
        landmark.points[index] = { (int)point.x, (int)point.y };
    }

    int result = faceall_facesdk_facequality(qualityHandle, image, landmark, &face->quality);
    if (result != 0) {
        logger.error("'faceall_facesdk_facequality' failed: %d", result);
        face->quality = 0;
        return result;
    }

    return 0;

#endif
}

int FaceEngine::feature(cv::Mat &mat, std::shared_ptr<FaceResult> face)
{
#ifdef USE_SEETA

    if (face->landmarks.size() != 5) {
        logger.error("Wrong landmarks");
        return -1;
    }

    VIPLImageData image;
    image.channels = mat.channels();
    image.width = mat.cols;
    image.height = mat.rows;
    image.data = mat.data;

    float *feature = new float[mRecognizer->GetFeatureSize()];
    VIPLPoint landmarks[5];
    for (int index = 0; index < 5; index++) {
        const cv::Point2d &point = face->landmarks[index];
        landmarks[index] = { point.x, point.y };
    }
    mRecognizer->ExtractFeatureWithCrop(image, landmarks, feature);
    face->feature = std::vector<float>(feature, feature + mRecognizer->GetFeatureSize());
    delete feature;

#else

    if (face->landmarks.size() == 0) {
        logger.error("No landmarks for face");
        return -1;
    }

    faceall_result_t result;
    faceall_image_t faceall_image;
    faceall_image.channels = mat.channels();
    faceall_image.width = mat.cols;
    faceall_image.height = mat.rows;
    faceall_image.data = mat.data;

    faceall_feature_t feature = { 0 };
    faceall_landmark_t landmark = { 0 };
    landmark.n_points = face->landmarks.size();
    landmark.points = new faceall_pointi_t[landmark.n_points];
    for (int index = 0; index < landmark.n_points; index++) {
        cv::Point2d &point = face->landmarks[index];
        landmark.points[index] = { (int)point.x, (int)point.y };
    }

    result = faceall_facesdk_feature(featureHandle, faceall_image, landmark, &feature);
    delete [] landmark.points;

    if (result != 0)
    {
        logger.error("faceall_facesdk_feature failed: %d", result);
        faceall_facesdk_feature_release_result(feature);
        return result;
    }

    face->feature = std::vector<float>(feature.feature, feature.feature + feature.length);
    faceall_facesdk_feature_release_result(feature);

#endif

    return 0;
}

int FaceEngine::compare(std::shared_ptr<FaceResult> faceA, std::shared_ptr<FaceResult> faceB, float &similarity)
{
#ifdef USE_SEETA

    float *featureA = faceA->feature.data();
    float *featureB = faceB->feature.data();
    similarity = mRecognizer->CalcSimilarity(featureA, featureB, mRecognizer->GetFeatureSize());

    return 0;

#else

    float *featureA = faceA->feature.data();
    int lengthA = faceA->feature.size();
    float *featureB = faceB->feature.data();
    int lengthB = faceB->feature.size();

    if (!featureA || !lengthA || !featureB || !lengthB) {
        logger.error("Invalid feature");
        return -1;
    }

    if (!compareHandle) {
        logger.error("Invalid compare handle");
        return -2;
    }

    faceall_feature_t a;
    a.feature = featureA;
    a.length = lengthA;

    faceall_feature_t b;
    b.feature = featureB;
    b.length = lengthB;

    return faceall_facesdk_compare(compareHandle, a, b, &similarity);

#endif
}

int FaceEngine::compare(const std::vector<float> &featureA, const std::vector<float> &featureB, float &similarity)
{
    return 0;
}

std::shared_ptr<FaceResult> FaceEngine::getMaxFace(std::vector<std::shared_ptr<FaceResult> > &faces)
{
    int maxSize = 0;
    std::shared_ptr<FaceResult> maxFace;
    for (auto &face: faces) {
        int size = face->rect.width * face->rect.height;
        if (size > maxSize) {
            maxSize = size;
            maxFace = face;
        }
    }
    return maxFace;
}

}
