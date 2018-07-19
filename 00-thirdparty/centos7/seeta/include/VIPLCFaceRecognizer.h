#ifndef VIPL_C_FACE_RECOGNIZER_H
#define VIPL_C_FACE_RECOGNIZER_H

#include "VIPLCStruct.h"

struct VIPLCFaceRecognizer;

/**
* \brief
* \param model_path
* \param device 
* \return
* \note Remeber call VIPLCDeleteFaceRecognizer on return type.
*/
VIPL_C_API struct VIPLCFaceRecognizer *VIPLCNewFaceRecognizer(const char *model_path, enum VIPLCDevice device);

VIPL_C_API void VIPLCDeleteFaceRecognizer(const struct VIPLCFaceRecognizer *obj);

VIPL_C_API int VIPLCFaceRecognizer_LoadModel(struct  VIPLCFaceRecognizer *obj, const char *model_path, enum VIPLCDevice device);

VIPL_C_API uint32_t VIPLCFaceRecognizer_GetFeatureSize(struct  VIPLCFaceRecognizer *obj);

VIPL_C_API uint32_t VIPLCFaceRecognizer_GetCropWidth(struct  VIPLCFaceRecognizer *obj);

VIPL_C_API uint32_t VIPLCFaceRecognizer_GetCropHeight(struct  VIPLCFaceRecognizer *obj);

VIPL_C_API uint32_t VIPLCFaceRecognizer_GetCropChannels(struct  VIPLCFaceRecognizer *obj);

VIPL_C_API int VIPLCFaceRecognizer_CropFace(struct  VIPLCFaceRecognizer *obj, VIPLImageData image, const VIPLPoint *landmarks, VIPLImageData *cropped_face);

VIPL_C_API int VIPLCFaceRecognizer_ExtractFeature(struct  VIPLCFaceRecognizer *obj, VIPLImageData cropped_face, float *feats);

VIPL_C_API int VIPLCFaceRecognizer_ExtractFeatureWithCrop(struct  VIPLCFaceRecognizer *obj, VIPLImageData image, const VIPLPoint *landmarks, float *feats);

VIPL_C_API float VIPLCFaceRecognizer_CalcSimilarity(struct  VIPLCFaceRecognizer *obj, const float *fc1, const float *fc2);

VIPL_C_API int VIPLCFaceRecognizer_ExtractFeatureNormalized(struct  VIPLCFaceRecognizer *obj, VIPLImageData cropped_face, float *feats);

VIPL_C_API int VIPLCFaceRecognizer_ExtractFeatureWithCropNormalized(struct  VIPLCFaceRecognizer *obj, VIPLImageData image, const VIPLPoint *landmarks, float *feats);

VIPL_C_API float VIPLCFaceRecognizer_CalcSimilarityNormalized(struct  VIPLCFaceRecognizer *obj, const float *fc1, const float *fc2);

VIPL_C_API void VIPLCFaceRecognizer_SetNumThreads(int num);

#endif // VIPL_C_FACE_RECOGNIZER_H
