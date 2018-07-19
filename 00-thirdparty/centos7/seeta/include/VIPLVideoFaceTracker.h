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
	* \brief 模型运行设备
	*/
	enum Device
	{
		AUTO,	/**< 自动检测，会优先使用 GPU */
		CPU,	/**< 使用 CPU 计算 */
		GPU,	/**< 使用 GPU 计算 */
	};

	VIPL_API VIPLVideoFaceTracker();
	VIPL_API ~VIPLVideoFaceTracker();
	/**
	* \brief 设置参数
	* \param interval 跟踪间隔
	* \param vid_width 视频宽度
	* \param vid_height 视频高度
	* \param min_face_detect_size 最小检测人脸
	* \param detector_model_path 检测器模型
	*/
	VIPL_API void setParameter(int interval, int vid_width, int vid_height, int min_face_detect_size, const char* detector_model_path);

	/**
	* \brief 设置参数
	* \param interval 跟踪间隔
	* \param vid_width 视频宽度
	* \param vid_height 视频高度
	* \param min_face_detect_size 最小检测人脸
	* \param detector_model_path 检测器模型
	*/
	VIPL_API void setParameter(int interval, int vid_width, int vid_height, int min_face_detect_size, const char* detector_model_path, Device device);
	/**
	 * \brief 输入单帧视频
	 * \param bgr24 输入图片，以 setParameter 输入的大小，行先序的彩色图片(以 BGR 24bit)
	 * \param frameID 输入的帧序号
	 * \return 返回非零表示运行正确
	 */
	VIPL_API int inputBGR24(unsigned char* bgr24, int frameID);
	/**
	 * \brief 返回基于当前帧跟踪的结果
	 * \return 跟踪的结果
	 */
	VIPL_API std::vector<VIPLTrackingInfo> getFaceInfo();

private:
	VIPLVideoFaceTracker(const VIPLVideoFaceTracker &other) = delete;
	const VIPLVideoFaceTracker &operator=(const VIPLVideoFaceTracker &other) = delete;

private:
	VIPLFaceTracker *_face_tracker;

};