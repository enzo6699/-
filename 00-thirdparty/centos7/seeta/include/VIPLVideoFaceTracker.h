#pragma once
#include "VIPLMoreStruct.h"
#include "VIPLStruct.h"

#define _SDK_MIN_MSC_VER 1800
#define _SDK_MAX_MSC_VER 1900

#if defined(_MSC_VER)
#if _MSC_VER < _SDK_MIN_MSC_VER || _MSC_VER > _SDK_MAX_MSC_VER
#error "Unsupported MSVC. Please use VS2013(v120) or compatible VS2015(v140)."
#endif // _MSC_VER < 1800 || _MSC_VER > 1900
#endif // defined(_MSC_VER)

#define VIPL_FACE_TRACKING_MAJOR_VERSION     2
#define VIPL_FACE_TRACKING_MINOR_VERSION     17
#define VIPL_FACE_TRACKING_SUBMINOR_VERSION  1

#include <vector>

class VIPLFaceTracker;

#if _MSC_VER >= 1600
extern template std::vector<VIPLTrackingInfo>;
#endif

class VIPLVideoFaceTracker {
public:
	/**
	* \brief ģ�������豸
	*/
	enum Device
	{
		AUTO,	/**< �Զ���⣬������ʹ�� GPU */
		CPU,	/**< ʹ�� CPU ���� */
		GPU,	/**< ʹ�� GPU ���� */
	};

	VIPL_API VIPLVideoFaceTracker();
	VIPL_API ~VIPLVideoFaceTracker();
	/**
	* \brief ���ò���
	* \param interval ���ټ��
	* \param vid_width ��Ƶ���
	* \param vid_height ��Ƶ�߶�
	* \param min_face_detect_size ��С�������
	* \param detector_model_path �����ģ��
	*/
	VIPL_API void setParameter(int interval, int vid_width, int vid_height, int min_face_detect_size, const char* detector_model_path);

	/**
	* \brief ���ò���
	* \param interval ���ټ��
	* \param vid_width ��Ƶ���
	* \param vid_height ��Ƶ�߶�
	* \param min_face_detect_size ��С�������
	* \param detector_model_path �����ģ��
	*/
	VIPL_API void setParameter(int interval, int vid_width, int vid_height, int min_face_detect_size, const char* detector_model_path, Device device);
	/**
	 * \brief ���뵥֡��Ƶ
	 * \param bgr24 ����ͼƬ���� setParameter ����Ĵ�С��������Ĳ�ɫͼƬ(�� BGR 24bit)
	 * \param frameID �����֡���
	 * \return ���ط����ʾ������ȷ
	 */
	VIPL_API int inputBGR24(unsigned char* bgr24, int frameID);
	/**
	 * \brief ���ػ��ڵ�ǰ֡���ٵĽ��
	 * \return ���ٵĽ��
	 */
	VIPL_API std::vector<VIPLTrackingInfo> getFaceInfo();

private:
	VIPLVideoFaceTracker(const VIPLVideoFaceTracker &other) = delete;
	const VIPLVideoFaceTracker &operator=(const VIPLVideoFaceTracker &other) = delete;

private:
	VIPLFaceTracker *_face_tracker;

};