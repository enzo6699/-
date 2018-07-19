#ifndef _TASK_INFO_H_
#define _TASK_INFO_H_

#include <string>
#include <sstream>

typedef struct _TaskInfo
{
	std::string toString()
	{
		std::stringstream ss;
                ss << "task_info_t [taskId=" << taskId << ", cameraId=" << cameraId << ", ip=" << ip
			<< ", port=" << port << ", userName=" << userName << ", password=" << password
			<< ", brand=" << brand << ", roi=" << roi << ", url=" << url
			<< ", faceAngle=" << faceAngle << ", interval=" << interval << ", filterConfidenceThreshold=" << filterConfidenceThreshold
			<< ", cacheTime=" << cacheTime << ", minWidth=" << minWidth << ", minHeight=" << minHeight << ", maxFaceNum=" << maxFaceNum 
			<< ", socre=" << score << "]";
		return ss.str();
	}

	std::string taskId;
	std::string cameraId;
	std::string ip;
	int port;
	std::string userName;
	std::string password;
	std::string brand;
	std::string roi;
	std::string url;

        std::string faceAngle;
	int interval;
	float filterConfidenceThreshold;
	int cacheTime;
	int minWidth;
	int minHeight;
	int maxFaceNum;

	float score;
        float pitch;
        float roll;
        float yaw;

} TaskInfo;

#endif
