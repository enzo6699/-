#include "LtsJobClient.h"
#include "json/json.h"
#include "TaskInfo.h"
#include "PropertyConfigurator.h"
#include "CameraTask.h"
#include "TaskMgr.h"
#include "CodeStatus.h"
#include <curl/curl.h>
#include <log4cpp/Category.hh>

static PropertyConfigurator &config = PropertyConfigurator::Instance();
static log4cpp::Category &logger = log4cpp::Category::getRoot();
static TaskMgr &taskMgr = TaskMgr::Instance();
static std::atomic<bool> listenerRunning(false);

static size_t curlWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    if (userdata) {
        std::string *content = (std::string *)userdata;
        content->append(ptr, size * nmemb);
    }
    return size * nmemb;
}

static std::string getJson(const std::string &url, int timeout, long *statusCode)
{
    std::string content;
    CURL *curl = curl_easy_init();
    if (!curl) {
        logger.error("Failed init curl");
        return "";
    }

    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout * 1000);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

    content.clear();
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        logger.error("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        content.clear();
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, statusCode);
    curl_easy_cleanup(curl);

    return content;
}

static std::string postJson(const std::string &url, int timeout, const std::string &data)
{
    std::string content;
    CURL *curl = curl_easy_init();
    if (!curl) {
        logger.error("Failed init curl");
        return "";
    }

    curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json; charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout * 1000);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

    content.clear();
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        logger.error("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        content.clear();
    }

    curl_easy_cleanup(curl);
    return content;
}

TaskListener::TaskListener()
{
    mHeartbeatUrl = config.ltsServerUrl + "app/lts/task/status";
    mResponseUrl = config.ltsServerUrl +"app/lts/task/response";
    mNotificationsUrl = config.ltsServerUrl + "app/lts/task/notifications";
}

TaskListener::~TaskListener()
{
    listenerRunning = false;
    if (mHeartbeatThread) {
        mHeartbeatThread->join();
    }
    if (mPullThread) {
        mPullThread->join();
    }

    logger.debug("Task listen stopped");
}

void TaskListener::start()
{
    listenerRunning = true;
    mHeartbeatThread = std::make_shared<std::thread>(&TaskListener::heartbeat, this);
    mPullThread = std::make_shared<std::thread>(&TaskListener::pullTask, this);
}

void TaskListener::heartbeat()
{
    while (listenerRunning) {
        doHeartbeat();
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}

void TaskListener::doHeartbeat()
{
    static Json::Value wroot;
    static Json::StyledWriter writer;

    wroot.clear();
    wroot["appId"] = config.appId;
    wroot["instanceId"] = config.instanceId;

    Json::Value tasks(Json::arrayValue);
    for (auto iter: taskMgr.m_mapTask) {
        std::string taskId = iter.first;
        std::shared_ptr<CameraTask> cameraTask = iter.second;

        Json::Value task;
        task["taskId"] = taskId;
        task["status"] = cameraTask->GetStatus();
        tasks.append(task);
    }
    wroot["tasks"] = tasks;
    std::string data = writer.write(wroot);

    logger.info("Sending heartbeat: %s", data.c_str());
    std::string reply = postJson(mHeartbeatUrl, config.timeout, data);
    logger.info("Received heartbeat reply: %s", reply.c_str());
}

void TaskListener::pullTask()
{
    static char url[2048] = { 0 };
    while (listenerRunning) {
        memset(url, 0, sizeof(url));
        float weight = taskMgr.GetTaskNumber() / (float)config.capacity;
        sprintf(url, "%s?appId=%s&instanceId=%s&weight=%.2f", mNotificationsUrl.c_str(), config.appId.c_str(), config.instanceId.c_str(), weight);
        long httpStatus = 0;
        logger.debug("Polling %s", url);
        std::string reply = getJson(url, config.longPollingInterval, &httpStatus);
        logger.info("Got polling reply: %s", reply.c_str());

        switch (httpStatus) {
        case 200: {
            logger.info("Got a new task");
            TaskResult taskResult = handleTask(reply);
            if (taskResult.taskId.empty()) {
                break;
            }
            taskResult.cause = GetErrorMessage(taskResult.status);
            respond(taskResult);
            // flush after each task request(add or delete)
            doHeartbeat();
            break;
        }
        case 304:
            logger.info("No new task");
            break;
        default:
            logger.info("Things went wrong during polling, wait and try again");
            std::this_thread::sleep_for(std::chrono::seconds(5));
            break;
        }
    }
}

void TaskListener::respond(const TaskResult &taskResult)
{
    if (taskResult.taskId.empty()) {
        logger.debug("No taskId to respond");
        return;
    }

    char url[2048] = { 0 };
    char *causeEncoded = curl_escape(taskResult.cause.c_str(), taskResult.cause.size());

    sprintf(url, "%s?taskId=%s&appId=%s&instanceId=%s&status=%d&cause=%s",
            mResponseUrl.c_str(),
            taskResult.taskId.c_str(),
            config.appId.c_str(),
            config.instanceId.c_str(),
            taskResult.status,
            causeEncoded);

    long httpStatus = 0;
    logger.debug("Sending task response to %s", url);
    std::string reply = getJson(url, config.timeout, &httpStatus);
    curl_free(causeEncoded);
    logger.debug("Received: %s, http code: %d", reply.c_str(), httpStatus);
}

TaskResult TaskListener::handleTask(const std::string &taskData)
{
    static Json::Value root;
    static Json::Reader reader;

    TaskResult taskResult;
    if (taskData.empty()) {
        logger.error("Empty task data");
        return taskResult;
    }

    if (!reader.parse(taskData, root)) {
        logger.error("Failed parse task data: %s", taskData.c_str());
        return taskResult;
    }

    int result = root["result"].asInt();
    if (result != 0 || root["taskId"].isNull() || root["type"].isNull()) {
        logger.error("Invalid task data: %s", taskData.c_str());
        return taskResult;
    }

    int taskType = root["type"].asInt();
    std::string taskId = root["taskId"].asString();
    std::string taskParam = root["taskParam"].isNull() ? "" : root["taskParam"].asString();

    taskResult.taskId = taskId;

    // 0表示添加任务, 2表示删除任务
    switch (taskType) {
    case 0: {
        if (!reader.parse(taskParam, root)) {
            logger.error("无效的taskParam: %s", taskParam.c_str());
            taskResult.status = ErrorCode::InvalidJsonData;
            return taskResult;
        }
        if (root["deviceId"].isNull()) {
            logger.error("deviceId不存在");
            taskResult.status = ErrorCode::InvalidTaskParam;
            break;
        }
        if (root["ipAddr"].isNull()) {
            logger.error("ipAddr不存在");
            taskResult.status = ErrorCode::InvalidTaskParam;
            break;
        }
        if (root["port"].isNull()) {
            logger.error("port不存在");
            taskResult.status = ErrorCode::InvalidTaskParam;
            break;
        }
        if (root["userId"].isNull()) {
            logger.error("userId不存在");
            taskResult.status = ErrorCode::InvalidTaskParam;
            break;
        }
        if (root["password"].isNull()) {
            logger.error("password不存在");
            taskResult.status = ErrorCode::InvalidTaskParam;
            break;
        }

        TaskInfo taskInfo;
        taskInfo.taskId = taskId;
        taskInfo.cameraId = root["deviceId"].asString();
        taskInfo.ip = root["ipAddr"].asString();
        taskInfo.port = root["port"].asInt();
        taskInfo.userName = root["userId"].asString();
        taskInfo.password = root["password"].asString();

        std::shared_ptr<CameraTask> cameraTask(new CameraTask);
        int nRet = cameraTask->Start(taskInfo);
        if (nRet != 0) {
            logger.error("任务启动失败, nRet: %d", nRet);
            taskResult.status = nRet;
            break;
        }

        taskResult.status = 0;
        taskMgr.AddTask(taskId, cameraTask);
        break;
    }
    case 2: {
        taskResult.status = 2;
        taskMgr.RemoveTask(taskId);
        break;
    }
    default:
        break;
    }

    return taskResult;
}
