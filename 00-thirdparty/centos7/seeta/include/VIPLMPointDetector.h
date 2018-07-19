#pragma once

#include "VIPLPointDetector.h"

namespace master
{
	using PDCore = ::VIPLPointDetector;
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
		VIPLPointDetector(const char* model_path = nullptr)
		{
			if (model_path) LoadModel(model_path);
		}

		/**
		* \brief 加载定位器模型，会卸载前面加载的模型
		* \param model_path 模型路径
		*/
		void LoadModel(const char* model_path)
		{
			m_cores.clear();
			AddModel(model_path);
		}

		/**
		* \brief 加载定位器模型，会卸载前面加载的模型
		* \param model_path 模型路径
		*/
		void AddModel(const char* model_path)
		{
			if (model_path == nullptr) return;
			auto new_core = std::make_shared<PDCore>(model_path);
			if (!m_cores.empty() && new_core->LandmarkNum() != this->LandmarkNum())
				throw std::logic_error("Mixd model landmarks must same");
			m_cores.push_back(std::move(new_core));
		}

		/**
		* \brief 设定是否以稳定模型工作
		* \param is_stable 是否以稳定模式工作
		* \note
		*/
		void SetStable(bool is_stable)
		{
			for (auto &core : m_cores) core->SetStable(is_stable);
		}

		/**
		* \brief 返回当前模型预测的定位点的个数
		* \return 定位点的个数
		*/
		int LandmarkNum() const
		{
			return m_cores.empty() ? 0 : m_cores[0]->LandmarkNum();
		}

		/**
		* \brief 在裁剪好的人脸上进行特征点定位
		* \param src_img 裁剪好的人脸图像，彩色
		* \param landmarks 指向长度为定位点个数的 VIPLPoint 数组
		* \return 只有定位成功后返回真
		*/
		bool DetectCroppedLandmarks(const VIPLImageData &src_img, VIPLPoint *landmarks) const
		{
			std::vector<std::vector<VIPLPoint>> landmarks_group;
			for (auto &core : m_cores)
			{
				std::vector<VIPLPoint> local_landmarks(core->LandmarkNum());
				if (!DetectCroppedLandmarks(src_img, local_landmarks.data())) return false;
				landmarks_group.push_back(std::move(local_landmarks));
			}
			auto local_landmarks = merge_landmarks(landmarks_group);
			if (local_landmarks.size() != LandmarkNum()) return false;
			std::memcpy(landmarks, local_landmarks.data(), local_landmarks.size());
			return true;
		}

		/**
		* \brief 在裁剪好的人脸上进行特征点定位
		* \param src_img 裁剪好的人脸图像，彩色
		* \param landmarks 要存放人脸特征点的数组
		* \return 只有定位成功后返回真
		*/
		bool DetectCroppedLandmarks(const VIPLImageData &src_img, std::vector<VIPLPoint> &landmarks) const
		{
			std::vector<std::vector<VIPLPoint>> landmarks_group;
			for (auto &core : m_cores)
			{
				std::vector<VIPLPoint> local_landmarks(core->LandmarkNum());
				if (!DetectCroppedLandmarks(src_img, local_landmarks.data())) return false;
				landmarks_group.push_back(std::move(local_landmarks));
			}
			landmarks = merge_landmarks(landmarks_group);
			if (landmarks.size() != LandmarkNum()) return false;
			return true;
		}

		/**
		* \brief 在原图人脸上进行特征点定位
		* \param src_img 原始图像，彩色
		* \param face_info 人脸位置
		* \param landmarks 指向长度为定位点个数的 VIPLPoint 数组
		* \return 只有定位成功后返回真
		*/
		bool DetectLandmarks(const VIPLImageData &src_img, const VIPLFaceInfo &face_info, VIPLPoint *landmarks) const
		{
			std::vector<std::vector<VIPLPoint>> landmarks_group;
			for (auto &core : m_cores)
			{
				std::vector<VIPLPoint> local_landmarks(core->LandmarkNum());
				if (!DetectLandmarks(src_img, face_info, local_landmarks.data())) return false;
				landmarks_group.push_back(std::move(local_landmarks));
			}
			auto local_landmarks = merge_landmarks(landmarks_group);
			if (local_landmarks.size() != LandmarkNum()) return false;
			std::memcpy(landmarks, local_landmarks.data(), local_landmarks.size());
			return true;
		}

		/**
		* \brief 在原图人脸上进行特征点定位
		* \param src_img 原始图像，彩色
		* \param face_info 人脸位置
		* \param landmarks 要存放人脸特征点的数组
		* \return 只有定位成功后返回真
		*/
		bool DetectLandmarks(const VIPLImageData &src_img, const VIPLFaceInfo &face_info, std::vector<VIPLPoint> &landmarks) const
		{
			std::vector<std::vector<VIPLPoint>> landmarks_group;
			for (auto &core : m_cores)
			{
				std::vector<VIPLPoint> local_landmarks(core->LandmarkNum());
				if (!DetectLandmarks(src_img, face_info, local_landmarks.data())) return false;
				landmarks_group.push_back(std::move(local_landmarks));
			}
			landmarks = merge_landmarks(landmarks_group);
			if (landmarks.size() != LandmarkNum()) return false;
			return true;
		}

	private:
		VIPLPointDetector(const VIPLPointDetector &other) = delete;
		const VIPLPointDetector &operator=(const VIPLPointDetector &other) = delete;

	private:
		std::vector<std::shared_ptr<PDCore>> m_cores;
		
		static std::vector<VIPLPoint> merge_landmarks(const std::vector<std::vector<VIPLPoint>> &landmarks_group)
		{
			if (landmarks_group.empty()) return std::vector<VIPLPoint>();
			static const VIPLPoint zero_point = { 0, 0 };
			std::vector<VIPLPoint> landmarks = landmarks_group[0];
			for (size_t i = 1; i < landmarks_group.size(); ++i)
			{
				auto &elem = landmarks_group[i];
				for (size_t j = 0; j < landmarks.size() && j < elem.size(); ++j)
				{
					landmarks[j].x += elem[j].x;
					landmarks[j].y += elem[j].y;
				}
			}
			for (auto &point : landmarks)
			{
				point.x /= landmarks_group.size();
				point.y /= landmarks_group.size();
			}
			return std::move(landmarks);
		}
	};
}