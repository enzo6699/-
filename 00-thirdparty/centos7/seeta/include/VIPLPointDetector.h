#ifndef VIPL_POINT_DETECTOR_H_
#define VIPL_POINT_DETECTOR_H_

#define _SDK_MIN_MSC_VER 1800
#define _SDK_MAX_MSC_VER 1900
#if defined(_MSC_VER)
#if _MSC_VER < _SDK_MIN_MSC_VER || _MSC_VER > _SDK_MAX_MSC_VER
#error "Unsupported MSVC. Please use VS2013(v120) or compatible VS2015(v140)."
#endif // _MSC_VER < 1800 || _MSC_VER > 1900
#endif // defined(_MSC_VER)

#define VIPL_POINT_DETECTOR_MAJOR_VERSION     5
#define VIPL_POINT_DETECTOR_MINOR_VERSION     0
#define VIPL_POINT_DETECTOR_SUBMINOR_VERSION  0

#include <memory>
#include <vector>

#include "VIPLStruct.h"

/**
 * \brief 特征点定位核心
 */
class VIPLStablePointDetector;

#if _MSC_VER >= 1600
extern template std::shared_ptr<VIPLStablePointDetector>;
extern template std::vector<VIPLPoint>;
#endif

/**
 * \brief 特征点定位器
 */
class VIPLPointDetector {
public:
        /**
   * \brief 构造定位器，需要传入定位器模型
   * \param model_path 定位器模型
   * \note 定位器模型一般为 VIPLPointDetector5.0.pts[x].dat，[x]为定位点个数
   */
  VIPL_API VIPLPointDetector(const char* model_path = nullptr);

        /**
   * \brief 加载定位器模型，会卸载前面加载的模型
   * \param model_path 模型路径
   */
  VIPL_API void LoadModel(const char* model_path);

        /**
   * \brief 设定是否以稳定模型工作
   * \param is_stable 是否以稳定模式工作
   * \note
   */
  VIPL_API void SetStable(bool is_stable);

        /**
   * \brief 返回当前模型预测的定位点的个数
   * \return 定位点的个数
   */
  VIPL_API int LandmarkNum() const;

        /**
   * \brief 在裁剪好的人脸上进行特征点定位
   * \param src_img 裁剪好的人脸图像，彩色
   * \param landmarks 指向长度为定位点个数的 VIPLPoint 数组
   * \return 只有定位成功后返回真
   */
  VIPL_API bool DetectCroppedLandmarks(const VIPLImageData &src_img, VIPLPoint *landmarks) const;

        /**
   * \brief 在裁剪好的人脸上进行特征点定位
   * \param src_img 裁剪好的人脸图像，彩色
   * \param landmarks 要存放人脸特征点的数组
   * \return 只有定位成功后返回真
   */
  VIPL_API bool DetectCroppedLandmarks(const VIPLImageData &src_img, std::vector<VIPLPoint> &landmarks) const;

  /**
  * \brief 在原图人脸上进行特征点定位
  * \param src_img 原始图像，彩色
  * \param face_info 人脸位置
   * \param landmarks 指向长度为定位点个数的 VIPLPoint 数组
  * \return 只有定位成功后返回真
  */
  VIPL_API bool DetectLandmarks(const VIPLImageData &src_img, const VIPLFaceInfo &face_info, VIPLPoint *landmarks) const;

  /**
  * \brief 在原图人脸上进行特征点定位
  * \param src_img 原始图像，彩色
  * \param face_info 人脸位置
  * \param landmarks 要存放人脸特征点的数组
  * \return 只有定位成功后返回真
  */
  VIPL_API bool DetectLandmarks(const VIPLImageData &src_img, const VIPLFaceInfo &face_info, std::vector<VIPLPoint> &landmarks) const;

private:
        VIPLPointDetector(const VIPLPointDetector &other) = delete;
        const VIPLPointDetector &operator=(const VIPLPointDetector &other) = delete;

private:
  std::shared_ptr<VIPLStablePointDetector> vipl_stable_point_detector_;
};

#endif // VIPL_POINT_DETECTOR_H_
