#ifndef _ANALYSE_SERVICE_H_
#define _ANALYSE_SERVICE_H_

#include <memory>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include "PropertyConfigurator.h"
#include "TaskInfo.h"
#include <log4cpp/Category.hh>
#include <ProducerImpl.h>
#include "Faceall.h"
#include "AnalyseStruct.h"

class AnalyseService
{
public:
	AnalyseService();
	~AnalyseService();

public:
    bool Start(const TaskInfo &task_info);
    void Stop();
    void Config(const TaskInfo &task_info);
    void Push(std::shared_ptr<FrameInfo> ptrFrameInfo);

private:
    void DetectFeatureThread();
    int StartFaceSDK();
    std::shared_ptr<tce::FaceResult> getMaxFace(std::vector<std::shared_ptr<tce::FaceResult>> faces);
    std::string saveOriginalImage(std::shared_ptr<FrameInfo> frame);
    std::string saveObjectImage(std::shared_ptr<FrameInfo> frame, std::shared_ptr<tce::FaceResult> face);
    void checkDirectory(const std::string &directory);
    std::string currentDate();
    void send(std::shared_ptr<FaceInfo> faceInfo);
    std::string upload(const std::string &filepath);

	TaskInfo m_task_info;

	std::shared_ptr<std::thread> m_detectFeatureThread;
	
    std::shared_ptr<tce::FaceEngineWrapper> mFaceall;
    std::shared_ptr<tce::CProducerImpl> producer;

    std::queue<std::shared_ptr<FrameInfo>> mFrameQueue;
    std::mutex mFrameQueueLock;

	std::atomic<bool> m_bStop;
    PropertyConfigurator &config;
    log4cpp::Category &logger;
};

#endif
