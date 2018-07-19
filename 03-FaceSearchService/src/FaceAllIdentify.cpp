#include "FaceAllIdentify.h"
#include "faceall_sdk.h"
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv/cv.h"
#include <log4cpp/Category.hh>

//#define USE_PRINTF_TIME 1

#ifdef USE_PRINTF_TIME
#include "utils/TimeUtil.h"
#endif

static log4cpp::Category &logger = log4cpp::Category::getRoot();

namespace tce {



	/////////////////////////////////////////////////////////////////////////////////
    FaceResult::FaceResult()
	{
		m_landmark.n_points = 0;
		m_landmark.points = NULL;
		m_feature.length = 0;
		m_feature.feature = NULL;
		m_isOk = true;
	}

    FaceResult::~FaceResult()
	{
		if (m_landmark.n_points != 0)
		{
			if (m_landmark.points != NULL)
			{
				delete[] m_landmark.points;
				m_landmark.points = NULL;
			}
		}

		if (m_feature.feature != NULL)
		{
			m_feature.length = 0;
			faceall_facesdk_feature_release_result(m_feature);
		}
	}
	/////////////////////////////////////////////////////////////////////////////////

	FaceAllIdentify::FaceAllIdentify()
		: m_faceDetectLandmark5Handle(NULL)
		, m_faceFeatureHandle(NULL)
		, m_faceCompareHandle(NULL)
	{
	}

	FaceAllIdentify::~FaceAllIdentify()
	{

	}

	int FaceAllIdentify::Init(int types)
	{

		int nRet = 0;
		if (types & face_detect_landmark5)
		{
            logger.info("init face detect and landmark");
			m_faceDetectLandmark5Handle = faceall_facesdk_detect_and_landmark_get_instance();
			if (m_faceDetectLandmark5Handle == NULL)
			{
                logger.error("face_detect_landmark failed!");
				nRet = 1;
			}
		}

		if (types & face_feature)
		{
            logger.info("init face feature");
			m_faceFeatureHandle = faceall_facesdk_feature_get_instance(FACEALL_compare_type_idfea);
			if (m_faceFeatureHandle == NULL)
			{
                logger.error("face_feature failed!");
				nRet = 2;
			}
		}

		if (types & face_compare)
		{
            logger.info("init face compare");
			m_faceCompareHandle = faceall_facesdk_compare_get_instance(FACEALL_compare_type_idfea);
			if (m_faceCompareHandle == NULL)
			{
                logger.error("face_compare failed!");
				nRet = 3;
			}
		}

		if (nRet != 0)
			Uninit();

		return nRet;
	}

	void FaceAllIdentify::Uninit()
	{
		if (m_faceDetectLandmark5Handle != NULL)
		{
			faceall_facesdk_detect_and_landmark_destroy_instance(m_faceDetectLandmark5Handle);
			m_faceDetectLandmark5Handle = NULL;
		}

		if (m_faceFeatureHandle != NULL)
		{
			faceall_facesdk_feature_destroy_instance(m_faceFeatureHandle);
			m_faceFeatureHandle = NULL;
		}

		if (m_faceCompareHandle != NULL)
		{
			faceall_facesdk_compare_destroy_instance(m_faceCompareHandle);
			m_faceCompareHandle = NULL;
		}
	}

	static faceall_rect_t* GetCenterRect(faceall_rect_t *pFaceRectsArray, unsigned int nFaceRectsCount, int width, int height)
	{

		faceall_rect_t *pFaceall_rect = NULL;
		int dvalue = width;
		for (unsigned int i = 0; i < nFaceRectsCount; i++)
		{
			int centerx = (pFaceRectsArray[i].left + pFaceRectsArray[i].right) / 2;
			int dvaluet = abs(centerx - width / 2);
			if (dvalue > dvaluet)
			{
				pFaceall_rect = &pFaceRectsArray[i];
				dvalue = dvaluet;
			}
		}
		return pFaceall_rect;

	}

    static int GetMaxFace(std::vector<std::shared_ptr<FaceResult>> &tFaceDetailed, faceall_rect_t *pFaceRectsArray,
		faceall_landmark_t *pFaceall_landmarkArray, unsigned int nFaceRectsCount)
	{
		int index = -1;
		int value = 0;
		for (size_t i = 0; i < nFaceRectsCount; i++)
		{
			int height = pFaceRectsArray[i].bottom - pFaceRectsArray[i].top;
			int width = pFaceRectsArray[i].right - pFaceRectsArray[i].left;
			if (value < height * width)
			{
				value = height * width;
				index = i;
			}

		}

		if (index != -1)
		{
            std::shared_ptr<FaceResult> face_detailed(new FaceResult());
			face_detailed->m_faceRect = pFaceRectsArray[index];
			face_detailed->m_landmark.n_points = pFaceall_landmarkArray[index].n_points;
			if (face_detailed->m_landmark.n_points > 0)
			{
				face_detailed->m_landmark.points = new faceall_pointi_t[face_detailed->m_landmark.n_points];
				for (int k = 0; k < face_detailed->m_landmark.n_points; k++)
				{
					face_detailed->m_landmark.points[k].x = pFaceall_landmarkArray[index].points[k].x;
					face_detailed->m_landmark.points[k].y = pFaceall_landmarkArray[index].points[k].y;
				}
			}
			tFaceDetailed.push_back(face_detailed);
			return 1;
		}

		return 0;
	}

    int FaceAllIdentify::Feature(const char* pImageData, size_t nDataLength, std::vector<std::shared_ptr<FaceResult>> &vecFaceDetailed, bool bMaxFace)
	{
		if (nDataLength == 0)
		{
			printf("image data length %d", nDataLength);
			return 1;
		}

		std::vector<uchar> dataImg(pImageData, pImageData + nDataLength);
		cv::Mat srcImage = cv::imdecode(cv::Mat(dataImg), CV_LOAD_IMAGE_COLOR | CV_LOAD_IMAGE_ANYCOLOR);
		if (srcImage.cols * srcImage.rows > MAX_IMAGE_RATIO_SIZE || srcImage.channels() > 3)
		{
			printf("image is not suppot: ratio:%d, channel:%d", srcImage.cols * srcImage.rows, srcImage.channels());
			return 2;
		}

		unsigned int nFaceRectsCount = 0;
		faceall_result_t result;
		faceall_image_t faceall_image;
		faceall_rect_t *pFaceRectsArray = NULL;
		faceall_landmark_t *pFaceall_landmarkArray = NULL;
		faceall_image.channels = srcImage.channels();
		faceall_image.width = srcImage.cols;
		faceall_image.height = srcImage.rows;
		faceall_image.data = srcImage.data;

#ifdef USE_PRINTF_TIME
		long long tvpre, tvafter;
		tvpre = tce::TimeUtil::GetTimestamp();
#endif

		result = faceall_facesdk_detect_and_landmark(m_faceDetectLandmark5Handle, faceall_image, &pFaceRectsArray, &pFaceall_landmarkArray, &nFaceRectsCount);

#ifdef USE_PRINTF_TIME
		tvafter = tce::TimeUtil::GetTimestamp();
		std::cerr << "####detect and landmark time:" << (tvafter - tvpre) << "ms" << std::endl;
#endif
		if (result != 0)
		{
			faceall_facesdk_detect_and_landmark_release_result(pFaceRectsArray, pFaceall_landmarkArray, nFaceRectsCount);
			printf("faceall_facesdk_detect_and_landmark %d", result);
			return 3;
		}
		if (nFaceRectsCount == 0)
			return 0;

		int nCount = GetMaxFace(vecFaceDetailed, pFaceRectsArray, pFaceall_landmarkArray, nFaceRectsCount);
		faceall_facesdk_detect_and_landmark_release_result(pFaceRectsArray, pFaceall_landmarkArray, nFaceRectsCount);
		if (nCount == 0)
		{
			printf("roi can't find face!\n");
			return 0;
		}

		for (size_t i = 0; i < nCount; i++)
		{
            std::shared_ptr<FaceResult> face_detailed = vecFaceDetailed.at(i);

			faceall_feature_t faceall_feature;
			result = faceall_facesdk_feature(m_faceFeatureHandle, faceall_image, face_detailed->m_landmark, &faceall_feature);
			if (result != 0)
			{
				faceall_facesdk_feature_release_result(faceall_feature);
				face_detailed->m_isOk = false;
				printf("faceall_facesdk_feature %d", result);
				break;
			}

			face_detailed->m_feature = faceall_feature;
		}

#ifdef USE_PRINTF_TIME
		tvafter = tce::TimeUtil::GetTimestamp();
		std::cerr << "####detect and landmark feature time:" << (tvafter - tvpre) << "ms" << std::endl;
#endif
		return 0;
	}



	int FaceAllIdentify::Compare(float* featureA, int lengthA, float* featureB, int lengthB, float &socore)
	{
		faceall_feature_t tfeatureA;
		tfeatureA.feature = featureA;
		tfeatureA.length = lengthA;

		faceall_feature_t tfeatureB;
		tfeatureB.feature = featureB;
		tfeatureB.length = lengthB;

		return faceall_facesdk_compare(m_faceCompareHandle, tfeatureA, tfeatureB, &socore);
	}
}
