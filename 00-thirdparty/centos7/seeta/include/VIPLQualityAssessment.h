#ifndef _VIPL_QUALITY_ASSESSMENT_H
#define _VIPL_QUALITY_ASSESSMENT_H

#include "VIPLStruct.h"

#define VIPL_QUALITY_ASSESSMENT_MAJOR_VERSION     2
#define VIPL_QUALITY_ASSESSMENT_MINOR_VERSION     1
#define VIPL_QUALITY_ASSESSMENT_SUBMINOR_VERSION  0

class VIPLQualityAssessmentCore;
class VIPL_API VIPLQualityAssessment
{
public:
	/**
	 * \brief 初始换质量评估模块
	 * \param model_path 姿态估计模型
	 * \param satisfied_face_size 推荐人脸大小
	 */
	explicit VIPLQualityAssessment(const char *model_path, int satisfied_face_size = 128);

	/**
	 * \brief 设置推荐人脸大小，大于其分辨率的人脸认为是合格的
	 * \param size 设置推荐人脸大小
	 */
	void setSatisfiedFaceSize(int size);

	/**
	 * \brief 获取推荐人脸大小
	 * \return 推荐人脸大小
	 */
	int getSatisfiedFaceSize() const;

	/**
	 * \brief 加载姿态估计模型，会卸载已加载模型
	 * \param model_path 姿态估计模型
	 * \return 只有当模型加载成功才返回真
	 */
	bool loadPoseEstimationModel(const char *model_path);

	/**
	 * \brief 返回质量评估分数 [0, 1]
	 * \param srcImg 输入原图
	 * \param faceInfo 原图中要进行人脸评估的人脸位置
	 * \return 质量评估分数 [0, 1]
	 */
	float Evaluate(const VIPLImageData &srcImg, const VIPLFaceInfo &faceInfo);
private:
	VIPLQualityAssessmentCore *impl;
};

#endif
