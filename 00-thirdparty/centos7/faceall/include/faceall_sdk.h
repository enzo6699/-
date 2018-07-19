/*
 * @file   faceall_sdk.h
 * @Author Faceall Group Limited
 * @brief  FaceSDK header for Faceall C API.
 *
 * Copyright (c) 2014-2015, Faceall Group Limited. All Rights Reserved.
 */

#include <faceall_common.h>
#include <opencv/cv.h>
#include <vector>
#ifndef FACEALL_SDK_INTERFACE_H_
#define FACEALL_SDK_INTERFACE_H_

#ifdef __cplusplus
extern "C"
{
#endif


FACEALL_SDK_INTERFACE
faceall_image_t
faceall_facesdk_Tofaceallimage(
const char* image_name
);
////<<<新人脸特征接口
/// @brief 该函数用于初始化人脸特征提取所需要的数据
FACEALL_SDK_INTERFACE 
faceall_handle_t 
faceall_facesdk_feature_get_instance(FACEALL_compare_type compare_type);

/// @brief 该函数用于提取一张人脸特征，请保证输入图片为单通道或者3通道，输入特征点为5点
/// @brief feature_handle 已完成初始化的人脸特征提取实例句柄
/// @paras image 用于提取特征的人脸图片
/// @paras landmark 用于提取特征的人脸图片的中某一个脸的特征点
/// @paras p_feature 指向提取的特征的指针，特征的内存空间需要由用户调用faceall_facesdk_feature_release——result函数释放
FACEALL_SDK_INTERFACE
faceall_result_t 
faceall_facesdk_feature(
	faceall_handle_t feature_handle,
	const faceall_image_t image, 
	const faceall_landmark_t landmark,
	faceall_feature_t *p_feature
);

/// @brief 该函数用于释放一张人脸提取的特征
/// @paras feature 一张人脸的特征
 FACEALL_SDK_INTERFACE
faceall_result_t 
faceall_facesdk_feature_release_result(faceall_feature_t feature);


/// @brief 该函数用于释放人脸特征提取所需要的数据
FACEALL_SDK_INTERFACE faceall_result_t faceall_facesdk_feature_destroy_instance(faceall_handle_t feature_handle);
////<<<< 人脸检测接口

FACEALL_SDK_INTERFACE faceall_handle_t faceall_facesdk_detect_and_landmark_get_instance();

FACEALL_SDK_INTERFACE faceall_result_t faceall_facesdk_detect_and_landmark(
	faceall_handle_t detect_and_landmark_handle,
	const faceall_image_t image,
	faceall_rect_t **p_face_rects_array,
	faceall_landmark_t **p_landmark_array,
	unsigned int *p_face_count
);

FACEALL_SDK_INTERFACE
faceall_result_t
faceall_facesdk_detect_and_landmark_release_result(
	faceall_rect_t *face_rects_array,
	faceall_landmark_t *face_landmark_array,
	int face_count
);

FACEALL_SDK_INTERFACE
faceall_result_t
faceall_facesdk_detect_and_landmark_destroy_instance(
	faceall_handle_t detect_landmark_handle
);

////<<<< 人脸特征比较接口
/// @brief 该函数用于初始化人脸比较所需要的数据
/// @paras compare_type 人脸比较的类型
FACEALL_SDK_INTERFACE faceall_handle_t faceall_facesdk_compare_get_instance(FACEALL_compare_type compare_type);

/// @brief 该函数用于一对特征的人脸比较, 请保证输入特征均为360维
/// @brief compare_handle 已完成初始化的人脸比对实例句柄
/// @paras feature1 第一个人脸特征
/// @paras feature2 第二个人脸特征
/// @paras p_score 指向结果的指针
FACEALL_SDK_INTERFACE
faceall_result_t
faceall_facesdk_compare(
	faceall_handle_t compare_handle,
	const faceall_feature_t feature1, 
	const faceall_feature_t feature2, 
	float *p_score
);

/// @brief 该函数用于释放人脸比较所需要的数据
FACEALL_SDK_INTERFACE faceall_result_t faceall_facesdk_compare_destroy_instance(faceall_handle_t compare_handle);




//face quality
FACEALL_SDK_INTERFACE
faceall_handle_t
faceall_facesdk_facequality_get_instance();
 
FACEALL_SDK_INTERFACE
faceall_result_t
faceall_facesdk_facequality(
      faceall_handle_t quality_handle,
      const faceall_image_t image,
      const faceall_landmark_t face_landmark,
      float *quality
);
 
FACEALL_SDK_INTERFACE
faceall_result_t
faceall_facesdk_facequality_destroy_instance(
      faceall_handle_t quality_handle
);


FACEALL_SDK_INTERFACE faceall_handle_t
faceall_facesdk_attribute_get_instance();

FACEALL_SDK_INTERFACE faceall_result_t
faceall_facesdk_attribute(
faceall_handle_t attr_handle,
const faceall_image_t image,
const faceall_rect_t rect,
faceall_attribute_t* face_attr
);

/// @brief 释放属性相关
FACEALL_SDK_INTERFACE faceall_result_t
faceall_facesdk_attribute_destroy_instance(faceall_handle_t attr_handle);






#ifdef __cplusplus 
}
#endif
#endif
