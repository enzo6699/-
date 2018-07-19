#include "ImageUtil.h"
//#include <log4cpp/Category.hh>

namespace tce
{
	int ScaleRect(cv::Rect &srcRect, cv::Rect &dstRect, cv::Rect imageRect)
	{
		dstRect.x = srcRect.x - (srcRect.width * 0.8);
		dstRect.y = srcRect.y - (srcRect.height * 0.6);
		dstRect.width = srcRect.width * 2.4;
		dstRect.height = srcRect.height * 2.0;

		dstRect.x = dstRect.x < 0 ? 0 : dstRect.x;
		dstRect.y = dstRect.y < 0 ? 0 : dstRect.y;
		dstRect.width = (dstRect.x + dstRect.width) > imageRect.width ? (imageRect.width - dstRect.x) : dstRect.width;
		dstRect.height = (dstRect.y + dstRect.height) > imageRect.height ? (imageRect.height - dstRect.y) : dstRect.height;

		return 0;
	}

	int ImageUtil::SaveCutImage(const char* pImageData, int nDataLength, cv::Rect rect, const std::string& saveFilename, bool bScale)
	{
		//当roi的x,y小于0的时候，x,y设置为0
		rect.x = rect.x < 0 ? 0 : rect.x;
		rect.y = rect.y < 0 ? 0 : rect.y;
		if (rect.width <= 0 || rect.height <= 0)
			return 1;

		std::vector<uchar> dataImg(pImageData, pImageData + nDataLength);
		cv::Mat srcImage = cv::imdecode(cv::Mat(dataImg), CV_LOAD_IMAGE_COLOR | CV_LOAD_IMAGE_ANYCOLOR);

		rect.width = rect.width > srcImage.cols ? srcImage.cols : rect.width;
		rect.height = rect.height > srcImage.rows ? srcImage.rows : rect.height;

		cv::Rect destRect(rect.x, rect.y, rect.width, rect.height);

		if (bScale)
		{
			//当此值为true的情况下，图片裁剪区域扩大
			ScaleRect(rect, destRect, cv::Rect(0, 0, srcImage.cols, srcImage.rows));
		}
		if (destRect.x < 0 || destRect.width < 0 || (destRect.x + destRect.width) > srcImage.cols
			|| destRect.y < 0 || destRect.height < 0 || (destRect.y + destRect.height) > srcImage.rows)
			return 3;

		cv::Mat pDstMat;
		srcImage(destRect).copyTo(pDstMat);

		std::vector<int> compression_params;
		compression_params.push_back(CV_IMWRITE_JPEG_QUALITY); //PNG格式图片的压缩级别    
		compression_params.push_back(100);

		if (!cv::imwrite(saveFilename, pDstMat, compression_params))
		{
			std::cout << "save image failed!" << std::endl;
			return 2;
		}

		return 0;
	}

	int ImageUtil::SaveCutImage(const char* pImageData, int nwidth, int nHeight, int type, cv::Rect rect, const std::string& saveFilename, bool bScale)
	{
		//当roi的x,y小于0的时候，x,y设置为0
		rect.x = rect.x < 0 ? 0 : rect.x;
		rect.y = rect.y < 0 ? 0 : rect.y;
		if (rect.width <= 0 || rect.height <= 0)
			return 1;

		//Mat(int rows, int cols, int type, void* data, size_t step=AUTO_STEP);
		cv::Mat srcImage(nHeight, nwidth, CV_8UC3, (void*)pImageData);

		rect.width = (rect.width + rect.x) > srcImage.cols ? srcImage.cols - rect.x : rect.width;
		rect.height = (rect.height + rect.y) > srcImage.rows ? srcImage.rows - rect.y : rect.height;

		cv::Rect destRect(rect.x, rect.y, rect.width, rect.height);
		if (bScale)
		{
			//当此值为true的情况下，周边图片放大
			ScaleRect(rect, destRect, cv::Rect(0, 0, srcImage.cols, srcImage.rows));
		}

		cv::Mat pDstMat;
		srcImage(destRect).copyTo(pDstMat);

		std::vector<int> compression_params;
		compression_params.push_back(CV_IMWRITE_JPEG_QUALITY); //PNG格式图片的压缩级别    
		compression_params.push_back(100);

		if (!cv::imwrite(saveFilename, pDstMat, compression_params))
		{
			std::cout << "save image failed!" << std::endl;
			return 2;
		}

		return 0;
	}

	int ImageUtil::SaveCutImage(const cv::Mat& srcImage, const std::string& saveFilename, cv::Rect rect, bool bScale)
	{
		//当roi的x,y小于0的时候，x,y设置为0
		rect.x = rect.x < 0 ? 0 : rect.x;
		rect.y = rect.y < 0 ? 0 : rect.y;
		if (rect.width <= 0 || rect.height <= 0)
			return 1;

		rect.width = (rect.width + rect.x) > srcImage.cols ? srcImage.cols - rect.x : rect.width;
		rect.height = (rect.height + rect.y) > srcImage.rows ? srcImage.rows - rect.y : rect.height;

		
		cv::Rect destRect(rect.x, rect.y, rect.width, rect.height);
		if (bScale)
		{
			//当此值为true的情况下，图片裁剪区域扩大
			ScaleRect(rect, destRect, cv::Rect(0, 0, srcImage.cols, srcImage.rows));
		}

		cv::Mat pDstMat;
		srcImage(destRect).copyTo(pDstMat);

		std::vector<int> compression_params;
		compression_params.push_back(CV_IMWRITE_JPEG_QUALITY); //PNG格式图片的压缩级别    
		compression_params.push_back(100);

		if (!cv::imwrite(saveFilename, pDstMat, compression_params))
		{
			std::cout << "save image failed!" << std::endl;
			return 2;
		}

		return 0;
	}

	int ImageUtil::SaveImage(const char* pImageData, int nwidth, int nHeight, int type, const std::string& saveFilename)
	{
		cv::Mat srcImage(nHeight, nwidth, CV_8UC3, (void*)pImageData);

		std::vector<int> compression_params;
		compression_params.push_back(CV_IMWRITE_JPEG_QUALITY); //PNG格式图片的压缩级别    
		compression_params.push_back(95);

		if (!cv::imwrite(saveFilename, srcImage, compression_params))
		{
			std::cout << "save image failed!" << std::endl;
			return 1;
		}

		return 0;
	}

	int ImageUtil::SaveImage(const cv::Mat& srcImage, const std::string& saveFilename)
	{
		std::vector<int> compression_params;
		compression_params.push_back(CV_IMWRITE_JPEG_QUALITY); //PNG格式图片的压缩级别    
		compression_params.push_back(95);

		if (!cv::imwrite(saveFilename, srcImage, compression_params))
		{
			std::cout << "save image failed!" << std::endl;
			return 1;
		}

		return 0;
	}

	int ImageUtil::ReadFile(const std::string& filename, char *buffer, int &length)
	{
		FILE *stream = fopen(filename.c_str(), "rb");

		if (stream == nullptr)
			return 1;

		fseek(stream, 0, SEEK_END);
		int fileLen = ftell(stream);

		if (fileLen <= 0 || length < fileLen)
		{
			fclose(stream);
			return 2;
		}

		fseek(stream, 0, SEEK_SET);
		int readLen = fread(buffer, fileLen, 1, stream);
		length = fileLen;
		fclose(stream);
		return 0;
	}
}
