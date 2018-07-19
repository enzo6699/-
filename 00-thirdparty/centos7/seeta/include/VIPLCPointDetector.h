#ifndef VIPL_C_POINT_DETECTOR_H
#define VIPL_C_POINT_DETECTOR_H

#include "VIPLStruct.h"

#define VIPL_C_API extern "C" VIPL_API

struct VIPLCPointDetector;
/**
* \brief
* \param model_path
* \return
* \note Remeber call VIPLCDeletePointDetector on return type.
*/
VIPL_C_API struct VIPLCPointDetector *VIPLCNewPointDetector(const char *model_path);

VIPL_C_API void VIPLCDeletePointDetector(const struct VIPLCPointDetector *obj);

VIPL_C_API void VIPLCPointDetector_LoadModel(struct VIPLCPointDetector *obj, const char *model_path);

VIPL_C_API void VIPLCPointDetector_SetStable(struct VIPLCPointDetector *obj, int is_stable);

VIPL_C_API int VIPLCPointDetector_LandmarkNum(const struct VIPLCPointDetector *obj);

VIPL_C_API int VIPLCPointDetector_DetectCroppedLandmarks(const struct VIPLCPointDetector *obj, VIPLImageData cropped_image, VIPLPoint *landmarks);

VIPL_C_API int VIPLCPointDetector_DetectLandmarks(const struct VIPLCPointDetector *obj, VIPLImageData image, VIPLFaceInfo info, VIPLPoint *landmarks);


#endif // VIPL_C_POINT_DETECTOR_H
