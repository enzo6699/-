#include "TaskMgr.h"
#include <thread>

TaskMgr& TaskMgr::Instance()
{
	static TaskMgr pmgr;
	return pmgr;
}

TaskMgr::TaskMgr() :
    config(PropertyConfigurator::Instance()),
    logger(log4cpp::Category::getRoot())
{
}

TaskMgr::~TaskMgr()
{
	m_mapTask.clear();
}

int TaskMgr::AddTask(const std::string& taskId, const std::shared_ptr<CameraTask> task)
{
    lock.lock();
    m_mapTask[taskId] = task;
    lock.unlock();
	return 0;
}

int TaskMgr::RemoveTask(const std::string& taskId)
{
	
	std::shared_ptr<CameraTask> task = GetTask(taskId);
	if (task.get() == NULL)
	{
		return 1;
	}
    lock.lock();
	m_mapTask.erase(taskId);
    lock.unlock();
	return 0;
}

std::shared_ptr<CameraTask> TaskMgr::GetTask(const std::string& taskId)
{
    lock.lock();
	std::shared_ptr<CameraTask> task;
    if (m_mapTask.find(taskId) != m_mapTask.end())
	{
        task = m_mapTask[taskId];
	}
    lock.unlock();
    return task;
}

std::shared_ptr<CameraTask> TaskMgr::GetTaskByCameraIP(const std::string &cameraIp)
{
    std::shared_ptr<CameraTask> target;
    lock.lock();
    logger.info("Finding task of ip: %s", cameraIp.c_str());
    for (auto &kv : m_mapTask) {
        std::shared_ptr<CameraTask> task = kv.second;
        if (task->GetTaskInfo().ip == cameraIp) {
            target = task;
            break;
        }
    }
    lock.unlock();
    return target;
}

int TaskMgr::GetTaskNumber()
{
	int taskNumber = 0;
    lock.lock();
	taskNumber = m_mapTask.size();
    lock.unlock();

	return taskNumber;
}
