#ifndef _IMAGE_UTIL_H_
#define _IMAGE_UTIL_H_

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv/cv.h"

namespace tce {
	class ImageUtil
	{

	public:
		static int ReadFile(const std::string& filename, char *buffer, int &length);
		static int SaveCutImage(const char* pImageData, int nDataLength, cv::Rect rect, const std::string& saveFilename, bool bScale = false);
		static int SaveCutImage(const char* pImageData, int nwidth, int nHeight, int type, cv::Rect rect, const std::string& saveFilename, bool bScale = false);
		static int SaveCutImage(const cv::Mat& srcImage, const std::string& saveFilename, cv::Rect rect, bool bScale = false);
		static int SaveImage(const char* pImageData, int nwidth, int nHeight, int type, const std::string& saveFilename);
		static int SaveImage(const cv::Mat& srcImage, const std::string& saveFilename);
	};

}

#endif