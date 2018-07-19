#include "CameraTask.h"
#include "HkCamera.h"
#include "CodeStatus.h"
#include "utils/GuidUtil.h"
#include "utils/TimeUtil.h"
#include "utils/StringUtil.h"
#include "utils/Base64Util.h"
#include "json/json.h"
#include <curl/curl.h>
#include <algorithm>

using std::shared_ptr;
using std::string;

CameraTask::CameraTask() :
    config(PropertyConfigurator::Instance()),
    logger(log4cpp::Category::getRoot())
{
}

CameraTask::~CameraTask()
{
	Stop();
}

int CameraTask::Start(const TaskInfo &task_info)
{
	m_task_info = task_info;

	//启动分析服务
    if (!mAnalyseService.Start(m_task_info))
    {
		Stop();
        log4cpp::Category::getRoot().error("算法初始化失败");
        return ErrorCode::StartFaceSDKError;
	}

	log4cpp::Category::getRoot().info("Start Analyse succeeded");

    int result = startCamera();
    if (result != 0)
	{
		Stop();
        return result;
	}

    m_serviceStatus = 0;
	return 0;
}

int CameraTask::startCamera()
{
    std::transform(m_task_info.brand.begin(), m_task_info.brand.end(), m_task_info.brand.begin(), ::toupper);
	media = shared_ptr<Media>(new HKCamera(this));
    int result = media->Start(m_task_info.ip, m_task_info.port, m_task_info.userName, m_task_info.password);

    if (result == 0) {
        logger.info("摄像头启动成功");
        return 0;
    }

    logger.error("摄像头启动失败");
    media.reset();
    return result;
}

void CameraTask::Stop()
{
    logger.info("Stop camera ...");
    if (media)
    {
        media->Stop();
    }
    logger.info("Stop camera done");

    logger.info("Stop analyse service ...");
    mAnalyseService.Stop();
    logger.info("Stop analyse service done");

    m_serviceStatus = 9001;
}

void CameraTask::Config(const TaskInfo &task_info)
{
	m_task_info = task_info;
    mAnalyseService.Config(task_info);
}

void CameraTask::Push(std::shared_ptr<FrameInfo> frame)
{
    mAnalyseService.Push(frame);
}

