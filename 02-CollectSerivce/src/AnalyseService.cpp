#include "AnalyseService.h"
#include "CodeStatus.h"
#include "Faceall.h"
#include "utils/StringUtil.h"
#include "utils/TimeUtil.h"
#include "utils/GuidUtil.h"
#include "utils/FileUtil.h"
#include <utils/Base64Util.h>
#include <curl/curl.h>
#include <json/writer.h>
#include <json/value.h>
#include <opencv/highgui.h>

using std::vector;
using std::shared_ptr;

static std::string content;
static size_t curlWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    content.append(ptr, size * nmemb);
    return size * nmemb;
}

AnalyseService::AnalyseService() :
    m_bStop(false),
    config(PropertyConfigurator::Instance()),
    logger(log4cpp::Category::getRoot())
{
}

AnalyseService::~AnalyseService()
{
    Stop();
}

bool AnalyseService::Start(const TaskInfo &task_info)
{
    if (StartFaceSDK() != 0)
    {
        logger.error("启动算法失败");
        return false;
    }

	//启动线程分析
	m_task_info = task_info;

	//先停止服务
    Stop();

    producer = std::make_shared<tce::CProducerImpl>();
    producer->Create(config.kafkabroker, config.kafkatopic, false);

	m_bStop = false;
	m_detectFeatureThread = std::make_shared<std::thread>(&AnalyseService::DetectFeatureThread, this);

    return true;
}

void AnalyseService::Stop()
{
    //停止线程
    m_bStop = true;

    if (m_detectFeatureThread.get() != NULL)
    {
        m_detectFeatureThread->join();
        m_detectFeatureThread = NULL;
    }
}

void AnalyseService::Config(const TaskInfo &task_info)
{
	m_task_info = task_info;
}

void AnalyseService::Push(std::shared_ptr<FrameInfo> ptrFrameInfo)
{
    mFrameQueueLock.lock();
    if (mFrameQueue.size() > MAX_VIDEOFRAME_CACHE_SIZE)
    {
        logger.warn("Frame queue is full");
//        mFrameQueue.pop();
    }
    mFrameQueue.push(ptrFrameInfo);
    mFrameQueueLock.unlock();
}

void AnalyseService::DetectFeatureThread()
{
	while (true)
	{
		if (m_bStop)
			return;

        mFrameQueueLock.lock();

        if (mFrameQueue.empty() || mFaceall.get() == NULL)
        {
            mFrameQueueLock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            continue;
        }

        std::shared_ptr<FrameInfo> frame = mFrameQueue.front();
        mFrameQueue.pop();
        mFrameQueueLock.unlock();

        vector<shared_ptr<tce::FaceResult>> faces = mFaceall->Detect(frame->mat, frame->roi, true);
        shared_ptr<tce::FaceResult> maxFace = getMaxFace(faces);
        if (!maxFace)
        {
            logger.debug("No faces found");
            continue;
        }

        if (maxFace->quality < config.qualityThreshold) {
            logger.debug("Quality too low, discarding it ...");
            continue;
        }
        if (maxFace->rect.width < config.minFaceWidth || maxFace->rect.height < config.minFaceHeight) {
            logger.debug("Face size (%d x %d ) too small, min: %d x %d",
                         maxFace->rect.width, maxFace->rect.height,
                         config.minFaceWidth, config.minFaceHeight);
            continue;
        }

        int nRet = mFaceall->Feature(frame->mat, maxFace);
        if (nRet != 0)
        {
            logger.warn("获取人脸特征值错误: %d", nRet);
            continue;
        }

        std::string originalPath = saveOriginalImage(frame);
        std::string objectPath = saveObjectImage(frame, maxFace);
        if (originalPath.empty() || objectPath.empty())
        {
            continue;
        }

        shared_ptr<FaceInfo> faceInfo(new FaceInfo);
        faceInfo->detail = maxFace;
        faceInfo->timestamp = frame->timestamp;
        faceInfo->originalPath = originalPath;
        faceInfo->objectPath = objectPath;

        send(faceInfo);
	}
}

std::shared_ptr<tce::FaceResult> AnalyseService::getMaxFace(std::vector<std::shared_ptr<tce::FaceResult> > faces)
{
    shared_ptr<tce::FaceResult> maxFace;
    int maxFaceSize = 0;
    for (int index = 0; index < faces.size(); index++)
    {
        shared_ptr<tce::FaceResult> face = faces[index];
        int width = face->rect.width;
        int height = face->rect.height;
        if (width * height > maxFaceSize)
        {
            maxFaceSize = width * height;
            maxFace = face;
        }
    }
    return maxFace;
}

std::string AnalyseService::saveOriginalImage(std::shared_ptr<FrameInfo> frame)
{
    std::string date = currentDate();
    std::string directory = config.metadatapath + m_task_info.cameraId + "/" + ORIGINAL_PHOTO_SAVEDIR + date;
    checkDirectory(directory);
    std::string originalPath = directory + "/" + tce::GuidUtil::gguid() + ".jpg";

    try
    {
        cv::rectangle(frame->mat, frame->roi, cv::Scalar(0, 0, 255), 4);
        cv::imwrite(originalPath, frame->mat);
    }
    catch (cv::Exception &ex)
    {
        logger.error("Failed saving original image: %s", ex.what());
        originalPath.clear();
    }

    return originalPath;
}

std::string AnalyseService::saveObjectImage(std::shared_ptr<FrameInfo> frame, std::shared_ptr<tce::FaceResult> face)
{
    std::string date = currentDate();
    std::string directory = config.metadatapath + m_task_info.cameraId + "/" + OBJECT_PHOTO_SAVEDIR + date;
    checkDirectory(directory);
    std::string objectPath = directory + "/" + tce::GuidUtil::gguid() + ".jpg";

//    int fullWidth = frame->mat.cols;
//    int fullHeight = frame->mat.rows;
//    cv::Rect faceRect(face->m_faceRect);
//    cv::Point center(faceRect.x + faceRect.width / 2, faceRect.y + faceRect.height / 2);
//    faceRect.x = center.x - faceRect.width < 0 ? 0 : center.x - faceRect.width;
//    faceRect.y = center.y - 1.2 * faceRect.height < 0 ? 0 : center.y - 1.2 * faceRect.height;
//    faceRect.width = center.x + faceRect.width > fullWidth ? fullWidth - center.x : faceRect.width * 2;
//    faceRect.height = center.y + 1.2 * faceRect.height > fullHeight ? fullHeight - center.y : faceRect.height * 2.4;

    try
    {
//        cv::Mat objectMat = frame->mat(faceRect);
//        cv::imwrite(objectPath, objectMat);
        cv::imwrite(objectPath, frame->faceMat);
    }
    catch (cv::Exception &ex)
    {
        logger.error("Failed saving object image: %s", ex.what());
//        logger.error("Face rect: %d,%d,%d,%d", faceRect.x, faceRect.y, faceRect.width, faceRect.height);
        objectPath.clear();
    }

    return objectPath;
}

void AnalyseService::checkDirectory(const std::string &directory)
{
    struct stat st = { 0 };
    stat(directory.c_str(), &st);
    if (S_ISDIR(st.st_mode)) {
        return;
    }

    logger.info("Creating %s", directory.c_str());
    std::string command = "mkdir -p " + directory;
    system(command.c_str());
}

std::string AnalyseService::currentDate()
{
    time_t timestamp = time(NULL);
    struct tm *ltm = localtime(&timestamp);
    char date[16] = { 0 };
    strftime(date, sizeof(date), "%Y/%m/%d/%H", ltm);

    return date;
}

int AnalyseService::StartFaceSDK()
{
    //启动算法
    if (mFaceall == NULL)
    {
        mFaceall = std::make_shared<tce::FaceEngineWrapper>();
        logger.info("Init faceall sdk");
        if (mFaceall->Init(tce::FaceDetect | tce::FaceFeature | tce::FaceAttribute | tce::FaceQuality) != 0)
        {
            return ErrorCode::StartFaceSDKError;
        }
    }
    return 0;
}

void AnalyseService::send(std::shared_ptr<FaceInfo> faceInfo)
{
    faceall_feature_t &faceFeature = faceInfo->detail->feature;

    std::string originalUrl = upload(faceInfo->originalPath);
    if (originalUrl.empty()) {
        logger.error("上传抓拍图片(%s)失败", faceInfo->originalPath.c_str());
        return;
    }

    std::string objectUrl = upload(faceInfo->objectPath);
    if (objectUrl.empty()) {
        logger.error("上传人脸图片(%s)失败", faceInfo->objectPath.c_str());
        return;
    }

    const cv::Rect &rect = faceInfo->detail->rect;
    char objectRect[64] = { 0 };
    sprintf(objectRect, "%d,%d,%d,%d", rect.x, rect.y, rect.width, rect.height);

    Json::StyledWriter writer;
    Json::Value wroot;
    wroot["collectFaceId"] = tce::GuidUtil::gguid();
    wroot["cameraId"] = m_task_info.cameraId;
    wroot["feature"] = tce::Base64Util::base64_encode((char *)faceFeature.feature, faceFeature.length * sizeof(float));
    wroot["originalUrl"] = originalUrl;
    wroot["objectUrl"] = objectUrl;
    wroot["objectRect"] = objectRect;
    wroot["objectQuality"] = faceInfo->detail->quality;
    wroot["timestamp"] = (int)(faceInfo->timestamp / 1000);
    Json::Value attribute;
    attribute["age"] = faceInfo->detail->attribute.age;
    attribute["race"] = faceInfo->detail->attribute.color;
    attribute["gender"] = faceInfo->detail->attribute.gender == 1 ? 0 : 1;
    wroot["attribute"] = attribute;

    producer->send(writer.write(wroot));
}

std::string AnalyseService::upload(const std::string &filepath)
{
    CURL *curl = curl_easy_init();
    if (!curl) {
        logger.error("Failed init curl");
        return "";
    }

    FILE *fp = fopen(filepath.c_str(), "rb");
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    std::string date = currentDate();
    std::string filename = std::string(filepath, filepath.rfind("/") + 1);
    if (filepath.find(ORIGINAL_PHOTO_SAVEDIR) != filepath.npos) {
        filename.insert(0, "original/");
    } else {
        filename.insert(0, "object/");
    }
    std::string url = config.nginxAddress + m_task_info.cameraId + "/" + date + "/" + filename;

    logger.info("Uploading %s", filepath.c_str());
    logger.info("Remote path: %s", url.c_str());

    curl_easy_setopt(curl, CURLOPT_READDATA, fp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000L);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fileSize);

    CURLcode res = curl_easy_perform(curl);
    fclose(fp);

    if (res != CURLE_OK) {
        logger.error("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return "";
    }

    curl_easy_cleanup(curl);
    return url;
}
