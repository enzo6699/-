#ifndef _SEETA_FACE_RECOGNIZER_H
#define _SEETA_FACE_RECOGNIZER_H

#define VIPL_FACE_RECOGNIZER_MAJOR_VERSION 5
#define VIPL_FACE_RECOGNIZER_MINOR_VERSION 0
#define VIPL_FACE_RECOGNIZER_SUBMINOR_VERSION 0

#define _SDK_MIN_MSC_VER 1800
#define _SDK_MAX_MSC_VER 1900
#if defined(_MSC_VER)
#if _MSC_VER < _SDK_MIN_MSC_VER || _MSC_VER > _SDK_MAX_MSC_VER
#error "Unsupported MSVC. Please use VS2013(v120) or compatible VS2015(v140)."
#endif // _MSC_VER < 1800 || _MSC_VER > 1900
#endif // defined(_MSC_VER)

#include "VIPLStruct.h"

typedef float* FaceFeatures;

typedef VIPLPoint VIPLPoint5[5];

class VIPLFaceRecognizerModel
{
public:
        friend class VIPLFaceRecognizer;

        VIPL_API VIPLFaceRecognizerModel(const char *model_path, int device);
        VIPL_API ~VIPLFaceRecognizerModel();
private:
        void *m_impl;
};

class VIPLFaceRecognizer {
public:
        class Param;
        VIPL_API const Param *GetParam() const;

        VIPL_API explicit VIPLFaceRecognizer(const Param *param);

        VIPL_API explicit  VIPLFaceRecognizer(const VIPLFaceRecognizerModel &model);

        /**
        * \brief 模型运行设备
        */
        enum Device
        {
                AUTO,	/**< 自动检测，会优先使用 GPU */
                CPU,	/**< 使用 CPU 计算 */
                GPU,	/**< 使用 GPU 计算 */
        };

        /**
        * \brief 构造识别器
        * \param [in] modelPath 识别器模型路径
        * \note 识别器模型一般为 VIPLFaceRecognizer5.0.XXX.dat
        * \note 如果只是使用人脸裁剪的部分，则不需要加载模型
        * \note 默认会以 AUTO 模式使用计算设备
        */
        VIPL_API explicit VIPLFaceRecognizer(const char* modelPath = NULL);

        /**
        * \brief 构造识别器
        * \param [in] modelPath 识别器模型路径
        * \param [in] device 使用的计算设备
        * \note 识别器模型一般为 VIPLFaceRecognizer5.0.XXX.dat
        * \note 如果只是使用人脸裁剪的部分，则不需要加载模型
        */
        VIPL_API explicit VIPLFaceRecognizer(const char* modelPath, Device device);

        /**
        * \brief 构造识别器
        * \param [in] modelBuffer 识别模型的内存对象
        * \param [in] bufferLength 识别模型的内存长度
        * \note 识别器模型一般为 VIPLFaceRecognizer5.0.XXX.dat
        * \note 如果只是使用人脸裁剪的部分，则不需要加载模型
        */
        VIPL_API explicit VIPLFaceRecognizer(const char* modelBuffer, size_t bufferLength, Device device = AUTO);

        VIPL_API ~VIPLFaceRecognizer();

        /**
        * \brief 为识别器加载模型，会卸载原加载模型
        * \param [in] modelPath 识别模型路径
        * \return 加载成功后返回真
        * \note 此函数是为了在构造的时候没有加载模型的情况下调用
        * \note 默认会以 AUTO 模式使用计算设备
        */
        VIPL_API bool LoadModel(const char* modelPath);

        /**
        * \brief 为识别器加载模型，会卸载原加载模型
        * \param [in] modelPath 识别模型路径
        * \param [in] device 使用的计算设备
        * \return 加载成功后返回真
        * \note 此函数是为了在构造的时候没有记载模型的情况下调用
        */
        VIPL_API bool LoadModel(const char* modelPath, Device device);

        /**
        * \brief 为识别器加载模型，会卸载原加载模型
        * \param [in] modelBuffer 识别模型的内存对象
        * \param [in] bufferLength 识别模型的内存长度
        * \param [in] device 使用的计算设备
        * \return 加载成功后返回真
        * \note 此函数是为了在构造的时候没有记载模型的情况下调用
        */
        VIPL_API bool LoadModel(const char* modelBuffer, size_t bufferLength, Device device = AUTO);

        /**
         * \brief 获取识别器提取特征维度
         * \return 特征维度
         */
        VIPL_API uint32_t GetFeatureSize();

        /**
         * \brief 获取用于识别而裁剪的人脸图片宽度
         * \return 人脸图片宽度
         */
        VIPL_API uint32_t GetCropWidth();

        /**
        * \brief 获取用于识别而裁剪的人脸图片高度
        * \return 人脸图片高度
        */
        VIPL_API uint32_t GetCropHeight();

        /**
        * \brief 获取用于识别而裁剪的人脸图片通道数
        * \return 人脸图片通道数
        */
        VIPL_API uint32_t GetCropChannels();

        /**
         * \brief 裁剪人脸
         * \param [in] srcImg 原始图像，彩色
         * \param [in] llpoint 原始图像中人脸特征点，5个
         * \param [out] dstImg 目标图像，根据裁剪信息预先申请好内存
         * \param [in] posNum 人脸姿态，保留参数，此版本无意义
         * \return 只有裁剪成功后才返回真
         */
        VIPL_API bool CropFace(const VIPLImageData &srcImg,
                const VIPLPoint5 &llpoint,
                const VIPLImageData &dstImg,
                uint8_t posNum = 1);

        /**
         * \brief 在裁剪好的人脸上提取特征
         * \param [in] cropImg 裁剪好的人脸（彩色），使用 CropFace 或者对应方法
         * \param [out] feats 待提取特征存放空间
         * \return 只有提取成功后才返回真
         */
        VIPL_API bool ExtractFeature(const VIPLImageData &cropImg,
                FaceFeatures const feats);

        /**
         * \brief 在裁剪好的人脸上提取特征（归一化的）
         * \param [in] cropImg 裁剪好的人脸（彩色），使用 CropFace 或者对应方法
         * \param [out] feats 待提取特征存放空间
         * \return 只有提取成功后才返回真
         */
        VIPL_API bool ExtractFeatureNormalized(const VIPLImageData &cropImg,
                FaceFeatures const feats);

        /**
         * \brief 在原始图像上，根据定位点提取特征
         * \param [in] srcImg 原始图像，彩色
         * \param [in] llpoint 原始图像中人脸特征点，5个
         * \param [out] feats 待提取特征存放空间
         * \param [in] posNum 人脸姿态，保留参数，此版本无意义
         * \return 只有提取成功后才返回真
         */
        VIPL_API bool ExtractFeatureWithCrop(const VIPLImageData &srcImg,
                const VIPLPoint5 &llpoint,
                FaceFeatures const feats,
                uint8_t posNum = 1);

        /**
         * \brief 在原始图像上，根据定位点提取特征（归一化的）
         * \param [in] srcImg 原始图像，彩色
         * \param [in] llpoint 原始图像中人脸特征点，5个
         * \param [out] feats 待提取特征存放空间
         * \param [in] posNum 人脸姿态，保留参数，此版本无意义
         * \return 只有提取成功后才返回真
         */
        VIPL_API bool ExtractFeatureWithCropNormalized(const VIPLImageData &srcImg,
                const VIPLPoint5 &llpoint,
                FaceFeatures const feats,
                uint8_t posNum = 1);

        /**
         * \brief 计算特征 fc1 和 fc2 的相似度
         * \param [in] fc1 特征向量1
         * \param [in] fc2 特征向量2
         * \param [in] dim 特征维度
         * \return 相似度
         * \note 默认特征维度应该是 GetFeatureSize() 的返回值，如果不是，则需要传入对应的特征长度
         */
        VIPL_API float CalcSimilarity(FaceFeatures const fc1,
                FaceFeatures const fc2,
                long dim = -1);

        /**
         * \brief 计算特征 fc1 和 fc2 的相似度
         * \param [in] fc1 特征向量1（归一化的）
         * \param [in] fc2 特征向量2（归一化的）
         * \param [in] dim 特征维度
         * \return 相似度
         * \note 默认特征维度应该是 GetFeatureSize() 的返回值，如果不是，则需要传入对应的特征长度
         * \note 此计算相似度函数必须输入归一化的特征向量，由后缀 Normalized 的函数提取的特征
         */
        VIPL_API float CalcSimilarityNormalized(FaceFeatures const fc1,
                FaceFeatures const fc2,
                long dim = -1);

private:
        VIPLFaceRecognizer(const VIPLFaceRecognizer &other) = delete;
        const VIPLFaceRecognizer &operator=(const VIPLFaceRecognizer &other) = delete;

private:
        class Recognizer;
        friend class VIPLFaceRecognizerModel;
        Recognizer* recognizer;
};
#endif // _SEETA_FACE_RECOGNIZER_H
