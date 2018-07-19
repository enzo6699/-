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
	 * \brief ��ʼ����������ģ��
	 * \param model_path ��̬����ģ��
	 * \param satisfied_face_size �Ƽ�������С
	 */
	explicit VIPLQualityAssessment(const char *model_path, int satisfied_face_size = 128);

	/**
	 * \brief �����Ƽ�������С��������ֱ��ʵ�������Ϊ�Ǻϸ��
	 * \param size �����Ƽ�������С
	 */
	void setSatisfiedFaceSize(int size);

	/**
	 * \brief ��ȡ�Ƽ�������С
	 * \return �Ƽ�������С
	 */
	int getSatisfiedFaceSize() const;

	/**
	 * \brief ������̬����ģ�ͣ���ж���Ѽ���ģ��
	 * \param model_path ��̬����ģ��
	 * \return ֻ�е�ģ�ͼ��سɹ��ŷ�����
	 */
	bool loadPoseEstimationModel(const char *model_path);

	/**
	 * \brief ���������������� [0, 1]
	 * \param srcImg ����ԭͼ
	 * \param faceInfo ԭͼ��Ҫ������������������λ��
	 * \return ������������ [0, 1]
	 */
	float Evaluate(const VIPLImageData &srcImg, const VIPLFaceInfo &faceInfo);
private:
	VIPLQualityAssessmentCore *impl;
};

#endif
