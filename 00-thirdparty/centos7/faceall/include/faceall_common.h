/*
 * @file   faceall_common.h
 * @Author Faceall Group Limited
 * @brief  Common header for Faceall C API.
 *
 * Copyright (c) 2015-2016, Faceall Group Limited. All Rights Reserved.
 */

#ifndef FACEALL_COMMON_H_
#define FACEALL_COMMON_H_


#ifdef WIN32
#ifdef FACEALL_EXPORT
#define FACEALL_SDK_INTERFACE __declspec(dllexport)
#else
#define FACEALL_SDK_INTERFACE __declspec(dllimport)
#endif 
#else
#define FACEALL_SDK_INTERFACE
#endif

#define NET_FEATURE_SIZE (128)
#define ID_FEATURE_SIZE (128)

typedef int faceall_result_t;
typedef void* faceall_handle_t;

typedef struct {
	int quality;
}faceall_quality_t;

typedef struct{
	 int x;		///< 点的水平方向坐标，无符号整形
	 int y;		///< 点的竖直方向坐标，无符号整形
}faceall_pointi_t;

typedef struct{
    int left;		///< 矩形最左边的坐标
	int right;		///< 矩形最右边的坐标
	int top;		///< 矩形最上边的坐标
	int bottom;	///< 矩形最下边的坐标
    float score;  ///< 矩形得分
}faceall_rect_t;

typedef struct{
	unsigned int height;	///< 图片高度
	unsigned int width;		///< 图片宽度
	unsigned int channels;	///< 图片通道数, 只支持单通道和三通道(RGB)
	unsigned char *data;	///< 图片数据, 大小为width*height*channels字节，Linux上存储顺序与OpenCV一致
}faceall_image_t;

typedef struct{
	int n_points;				///< 特征点个数，目前只支持5点
	faceall_pointi_t* points;	///< 特征点
}faceall_landmark_t;




typedef struct{
  int length;
  float* feature;
}faceall_feature_t;




typedef enum{
    FACEALL_compare_type_idfea,
    FACEALL_compare_type_netfea 
}FACEALL_compare_type;



/*typedef struct{
    float age;
    int gender;
    int race;
    int expression;
    float beauty;
}faceall_attribute_t;*/

typedef struct{
    int age;
    int gender;
    int glass;
    int color;
    int hat;
    int mask;
}faceall_attribute_t;






#define FACEALL_LIVENESS_LIVE (1)
#define FACEALL_LIVENESS_SPOOF (0)

#define FACEALL_OBJ_NORMAL (20)
#define FACEALL_OBJ_SUSPECT (21)
#define FACEALL_OBJ_SEXY (22)
#define FACEALL_OBJ_PORN (23)
#define FACEALL_OBJ_CARTOON (24)
#define FACEALL_OBJ_IDCARD (25)
#define FACEALL_OBJ_MASK (26)

#define FACEALL_OK (0)
#define FACEALL_E_FAIL (-1)
#define FACEALL_E_FAIL_TO_ALLOCATE_MEMORY (-2)
#define FACEALL_E_CANT_OPEN_FILE (-3)
#define FACEALL_E_WRONG_VALUE (-4)
#define FACEALL_E_WRONG_TYPE (-5)
#define FACEALL_E_HANDLE (-6)
#define FACEALL_E_LICENSE (-7)

#define FACEALL_ENV_NAME "FACE_MODEL_PATH"

#endif
