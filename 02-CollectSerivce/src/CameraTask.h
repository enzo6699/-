#ifndef _CAMERA_TASK_H_
#define _CAMERA_TASK_H_

#include <atomic>
#include <queue>
#include <log4cpp/Category.hh>
#include "PropertyConfigurator.h"
#include "AnalyseService.h"
#include "TaskInfo.h"
#include "Media.h"

typedef struct _face_raw_t
{
	std::string toString()
	{
		std::stringstream ss;
        ss << "face_raw_t [cameraId=" << cameraId << ", originalUrl=" << originalUrl << ", objectUrl=" << objectUrl
			<< ", facepostion=" << facePostion << ", pose=" << pose << ", score=" << score << ", timestamp=" << timestamp << "]";
		return ss.str();
	}

	std::string cameraId;
    std::string originalUrl;
	std::string objectUrl;
	std::string facePostion;
	std::string pose;
	std::string feature;
	float score;
	int timestamp;
} face_raw_t;

class CameraTask
{
public:
    CameraTask();
    ~CameraTask();

    int Start(const TaskInfo &task_info);
    void Config(const TaskInfo &task_info);
	void Stop();

    inline int GetStatus() { return m_serviceStatus; }
    inline TaskInfo& GetTaskInfo() { return m_task_info; }
    void Push(std::shared_ptr<FrameInfo> frame);

private:
    int startCamera();

    int m_serviceStatus = 9001; // 服务状态，0-正常，9001-停止

    AnalyseService mAnalyseService;
    TaskInfo m_task_info;

    std::shared_ptr<Media> media;
	
	std::queue<std::shared_ptr<face_raw_t>> m_faceRowQueue;

	char m_buffer[MAX_IMAGE_SIZE];

    PropertyConfigurator &config;
    log4cpp::Category &logger;
};

#endif // _CAMERA_TASK_H_
