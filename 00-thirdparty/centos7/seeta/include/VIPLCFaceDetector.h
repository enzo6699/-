#ifndef VIPL_C_FACE_DETECTOR_H
#define VIPL_C_FACE_DETECTOR_H

#include "VIPLCStruct.h"

struct VIPLCFaceDetector;

struct VIPLCFaceInfoArray
{
	VIPLFaceInfo *data;
	int size;
};

/**
 * \brief 
 * \param model_path 
 * \param device 
 * \return
 * \note Remeber call VIPLCDeleteFaceDetector on return type.
 */
VIPL_C_API struct VIPLCFaceDetector *VIPLCNewFaceDetector(const char *model_path, enum VIPLCDevice device);

VIPL_C_API void VIPLCDeleteFaceDetector(const struct VIPLCFaceDetector *obj);

/**
 * \brief 
 * \param obj 
 * \param image 
 * \return 
 * \note Remeber call VIPLCDeleteFaceInfoArray on return type.
 */
VIPL_C_API struct VIPLCFaceInfoArray VIPLCFaceDetector_Detect(struct VIPLCFaceDetector *obj, VIPLImageData image);

VIPL_C_API void VIPLCDeleteFaceInfoArray(const VIPLCFaceInfoArray obj);

VIPL_C_API void VIPLCFaceDetector_SetMinFaceSize(struct VIPLCFaceDetector *obj, int32_t size);

VIPL_C_API void VIPLCFaceDetector_SetImagePyramidScaleFactor(struct VIPLCFaceDetector *obj, float factor);

VIPL_C_API void VIPLCFaceDetector_SetScoreThresh(struct VIPLCFaceDetector *obj, float thresh1, float thresh2, float thresh3);

VIPL_C_API void VIPLCFaceDetector_SetVideoStable(struct VIPLCFaceDetector *obj, int stable);

VIPL_C_API int VIPLCFaceDetector_GetVideoStable(struct VIPLCFaceDetector *obj);

VIPL_C_API void VIPLCFaceDetector_SetNumThreads(int num);

#endif // VIPL_C_FACE_DETECTOR_H
