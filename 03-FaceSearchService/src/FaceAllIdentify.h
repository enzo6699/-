#ifndef _FI_FACE_IDENTIFY_H_
#define _FI_FACE_IDENTIFY_H_

#include <faceall_common.h>
#include <memory>
#include <vector>

#define MAX_IMAGE_RATIO_SIZE 4000 * 4000

namespace tce {
	typedef enum
	{
		face_detect_landmark5 = 0x01,
		face_feature = 0x02,
		face_compare = 0x04,
		face_all = 0x01 | 0x02 | 0x04,
	} face_type_e;

	typedef struct
	{
		int x;
		int y;
		int width;
		int height;
	}face_rect_t;

    struct FaceResult
	{
	public:
        FaceResult();
        ~FaceResult();

		faceall_rect_t m_faceRect;
		faceall_landmark_t m_landmark;
		faceall_feature_t m_feature;
		faceall_attribute_t m_attribute;
		bool m_isOk;
    };

	class FaceAllIdentify
	{
	public:
		FaceAllIdentify();
		~FaceAllIdentify();

	public:
		int Init(int types);
		void Uninit();

        int Feature(const char* pImageData, size_t nDataLength, std::vector<std::shared_ptr<FaceResult>> &vecFaceDetailed, bool bMaxFace = true);
		int Compare(float* featureA, int lengthA, float* featureB, int lengthB, float &socore);
	private:
		faceall_handle_t m_faceDetectLandmark5Handle;
		faceall_handle_t m_faceFeatureHandle;
		faceall_handle_t m_faceCompareHandle;
	};
}

#endif
