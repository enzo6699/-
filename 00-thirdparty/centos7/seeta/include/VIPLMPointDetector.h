#pragma once

#include "VIPLPointDetector.h"

namespace master
{
	using PDCore = ::VIPLPointDetector;
	/**
	* \brief �����㶨λ��
	*/
	class VIPLPointDetector {
	public:
		/**
		* \brief ���춨λ������Ҫ���붨λ��ģ��
		* \param model_path ��λ��ģ��
		* \note ��λ��ģ��һ��Ϊ VIPLPointDetector5.0.pts[x].dat��[x]Ϊ��λ�����
		*/
		VIPLPointDetector(const char* model_path = nullptr)
		{
			if (model_path) LoadModel(model_path);
		}

		/**
		* \brief ���ض�λ��ģ�ͣ���ж��ǰ����ص�ģ��
		* \param model_path ģ��·��
		*/
		void LoadModel(const char* model_path)
		{
			m_cores.clear();
			AddModel(model_path);
		}

		/**
		* \brief ���ض�λ��ģ�ͣ���ж��ǰ����ص�ģ��
		* \param model_path ģ��·��
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
		* \brief �趨�Ƿ����ȶ�ģ�͹���
		* \param is_stable �Ƿ����ȶ�ģʽ����
		* \note
		*/
		void SetStable(bool is_stable)
		{
			for (auto &core : m_cores) core->SetStable(is_stable);
		}

		/**
		* \brief ���ص�ǰģ��Ԥ��Ķ�λ��ĸ���
		* \return ��λ��ĸ���
		*/
		int LandmarkNum() const
		{
			return m_cores.empty() ? 0 : m_cores[0]->LandmarkNum();
		}

		/**
		* \brief �ڲü��õ������Ͻ��������㶨λ
		* \param src_img �ü��õ�����ͼ�񣬲�ɫ
		* \param landmarks ָ�򳤶�Ϊ��λ������� VIPLPoint ����
		* \return ֻ�ж�λ�ɹ��󷵻���
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
		* \brief �ڲü��õ������Ͻ��������㶨λ
		* \param src_img �ü��õ�����ͼ�񣬲�ɫ
		* \param landmarks Ҫ������������������
		* \return ֻ�ж�λ�ɹ��󷵻���
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
		* \brief ��ԭͼ�����Ͻ��������㶨λ
		* \param src_img ԭʼͼ�񣬲�ɫ
		* \param face_info ����λ��
		* \param landmarks ָ�򳤶�Ϊ��λ������� VIPLPoint ����
		* \return ֻ�ж�λ�ɹ��󷵻���
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
		* \brief ��ԭͼ�����Ͻ��������㶨λ
		* \param src_img ԭʼͼ�񣬲�ɫ
		* \param face_info ����λ��
		* \param landmarks Ҫ������������������
		* \return ֻ�ж�λ�ɹ��󷵻���
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