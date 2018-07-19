#ifndef _TASK_MGR_H_
#define _TASK_MGR_H_

#include "PropertyConfigurator.h"
#include "CameraTask.h"
#include <map>
#include <memory>
#include <log4cpp/Category.hh>

class TaskMgr
{
public:
	static TaskMgr& Instance();
public:
	TaskMgr();
	~TaskMgr();

public:
	int AddTask(const std::string& taskId, const std::shared_ptr<CameraTask> task);
	int RemoveTask(const std::string& taskId);
	std::shared_ptr<CameraTask> GetTask(const std::string& taskId);
    std::shared_ptr<CameraTask> GetTaskByCameraIP(const std::string& cameraIp);
	int GetTaskNumber();

public:
	std::map<std::string, std::shared_ptr<CameraTask>> m_mapTask;
private:
    std::mutex lock;
    PropertyConfigurator &config;
    log4cpp::Category &logger;
};

#endif
