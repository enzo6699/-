#ifndef _LTS_JOBCLIENT_H_
#define _LTS_JOBCLIENT_H_

#include <string>
#include <memory>
#include <thread>

struct TaskResult
{
    std::string taskId;
    int status;
    std::string cause;
};

class TaskListener
{
public:
    TaskListener();
    ~TaskListener();

    void start();

private:
    void heartbeat();
    void doHeartbeat();
    void pullTask();
    void respond(const TaskResult &taskResult);
    TaskResult handleTask(const std::string &taskData);

    std::shared_ptr<std::thread> mHeartbeatThread;
    std::shared_ptr<std::thread> mPullThread;
    std::string mHeartbeatUrl;
    std::string mResponseUrl;
    std::string mNotificationsUrl;
};

#endif
