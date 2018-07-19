#include "HkCamera.h"
#include "CameraTask.h"
#include "json/json.h"
#include "common/Logger.h"
#include "opencv/highgui.h"
#include "opencv/cv.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "PropertyConfigurator.h"
#include "TaskMgr.h"
#include "CodeStatus.h"

static int gInstanceCount = 0;

static void sdkExceptionCallback(DWORD type, LONG userID, LONG handle, void *data)
{
    log4cpp::Category &logger = log4cpp::Category::getRoot();
    logger.debug("SDK exception, user: %d, description:", userID);
    switch (type) {
    case EXCEPTION_RELOGIN:
        logger.debug("用户重登陆");
        break;
    case RELOGIN_SUCCESS:
        logger.debug("用户重登陆成功");
        break;
    case EXCEPTION_ALARMRECONNECT:
        logger.debug("报警时重连");
        break;
    case EXCEPTION_EXCHANGE:
        logger.debug("用户交互时异常");
        break;
    default:
        logger.debug("未知的异常: 0x%X", type);
        break;
    }
}

void HKCamera::alarmMessageCallback(LONG command, NET_DVR_ALARMER *alarmer, char *alarmInfo, DWORD bufLen, void *user)
{
    log4cpp::Category &logger = log4cpp::Category::getRoot();

    logger.info("ALARM, command: 0x%X", command);
    switch (command) {
    case COMM_UPLOAD_FACESNAP_RESULT: {
        logger.info("Face snap");
        NET_VCA_FACESNAP_RESULT *faceSnap = (NET_VCA_FACESNAP_RESULT *)alarmInfo;
        std::shared_ptr<FrameInfo> frame(new FrameInfo);
        std::vector<uchar> data;
        if (faceSnap->pBuffer1 && faceSnap->dwFacePicLen) {
            logger.debug("Getting clipped face image");
            data = std::vector<uchar>(faceSnap->pBuffer1, faceSnap->pBuffer1 + faceSnap->dwFacePicLen);
            try {
                frame->faceMat = cv::imdecode(cv::Mat(data), CV_LOAD_IMAGE_COLOR | CV_LOAD_IMAGE_ANYCOLOR);
            } catch (cv::Exception &ex) {
                logger.error("Failed get clip image: %s", ex.what());
                break;
            }
        }
        if (faceSnap->dwBackgroundPicLen && faceSnap->pBuffer2) {
            logger.debug("Getting full image");
            data = std::vector<uchar>(faceSnap->pBuffer2, faceSnap->pBuffer2 + faceSnap->dwBackgroundPicLen);
            try {
                frame->mat = cv::imdecode(cv::Mat(data), CV_LOAD_IMAGE_COLOR | CV_LOAD_IMAGE_ANYCOLOR);
            } catch (cv::Exception &ex) {
                logger.error("Failed get full image: %s", ex.what());
                break;
            }
            const NET_VCA_RECT &rect = faceSnap->struRect;
            frame->roi.x = rect.fX * frame->mat.cols;
            frame->roi.y = rect.fY * frame->mat.rows;
            frame->roi.width = rect.fWidth * frame->mat.cols;
            frame->roi.height = rect.fHeight * frame->mat.rows;
        }
        std::shared_ptr<CameraTask> task = TaskMgr::Instance().GetTaskByCameraIP(alarmer->sDeviceIP);
        if (task) {
            task->Push(frame);
        }

        break;
    }
    default:
        break;
    }
}

HKCamera::HKCamera(CameraTask *cameraService) :
    task(cameraService),
    logger(log4cpp::Category::getRoot())
{
    if (gInstanceCount == 0)
    {
        logger.info("Try init hk sdk");
        if (!NET_DVR_Init() ||
            !NET_DVR_SetExceptionCallBack_V30(0, NULL, sdkExceptionCallback, NULL) ||
            !NET_DVR_SetDVRMessageCallBack_V50(0, alarmMessageCallback, NULL))
        {
            logger.error("Failed init SDK");
        }
    }

    gInstanceCount++;
}

HKCamera::~HKCamera()
{
    Stop();

    gInstanceCount--;
    if (gInstanceCount == 0)
    {
        logger.info("Try cleanup hk sdk");
        if (NET_DVR_Cleanup() == FALSE)
        {
            logger.error("Failed cleanup hk sdk");
        }
    }
}

int HKCamera::Start(const std::string &ip, int port, const std::string &username, const std::string &password)
{
    m_ip = ip;
    m_port = port;
    m_username = username;
    m_password = password;

    int error = login();

    if (m_userID < 0 || error != 0)
    {
        logger.error("hk login failed!");
        Stop();
        return error;
    }

    NET_DVR_SetupAlarmChan(m_userID);
    return 0;
}

void HKCamera::Stop()
{
    if (m_userID >= 0)
    {
        if (m_playHandle > 0)
        {
            NET_DVR_StopRealPlay(m_playHandle);
            m_playHandle = 0;
        }

        if (m_playPort > 0)
        {
            PlayM4_FreePort(m_playPort);
            m_playPort = 0;
        }

        NET_DVR_Logout_V30(m_userID);
        m_userID = -1;
    }
}

int HKCamera::login()
{
    if (m_userID >= 0) {
        log4cpp::Category::getRoot().debug("Already login");
        return 0;
    }

    memset(&deviceInfo, 0, sizeof(deviceInfo));
    m_userID = NET_DVR_Login_V30((char*)m_ip.c_str(), m_port, (char*)m_username.c_str(), (char*)m_password.c_str(), &deviceInfo);

    if (m_userID >= 0) {
        logger.info("Login succeeded");
        logger.info("UserId: %d", m_userID);
        logger.info("Device serial: %s", deviceInfo.sSerialNumber);
        logger.info("Start channel: %u", deviceInfo.byStartChan);
        logger.info("Start digital channel: %u", deviceInfo.byStartDChan);
        return 0;
    }

    DWORD error = NET_DVR_GetLastError();
    logger.error("Login failed, code: %d, message: %s", error, NET_DVR_GetErrorMsg());
    switch (error) {
    case NET_DVR_USER_LOCKED:
        return ErrorCode::UserIsLocked;
    case NET_DVR_PASSWORD_ERROR:
        return ErrorCode::WrongPassword;
    case NET_DVR_ORDER_ERROR: // SDK误报
    case NET_DVR_NETWORK_RECV_TIMEOUT:
    case NET_DVR_NETWORK_FAIL_CONNECT:
        return ErrorCode::DeviceConnectionError;
    case NET_DVR_USERNAME_NOT_EXIST:
        return ErrorCode::NoSuchUser;
    default:
        break;
    }
    return ErrorCode::UnknownError;
}

void HKCamera::realDataCallback(LONG playHandle, DWORD dataType, BYTE *buffer, DWORD bufferSize, void *userData)
{
    HKCamera *pCamera = static_cast<HKCamera *>(userData);

    switch (dataType) {
    case NET_DVR_SYSHEAD:
        if (pCamera->m_playPort >= 0) {
            break;
        }
        if (PlayM4_GetPort((LONG*)&pCamera->m_playPort) == FALSE) {
            log4cpp::Category::getRoot().error("Failed calling 'PlayM4_GetPort'");
            break;
        }
        if (bufferSize > 0) {
            //PlayM4_SetDecodeFrameType(pCamera->m_playPort, T_RGB32);
            PlayM4_SetStreamOpenMode(pCamera->m_playPort, STREAME_REALTIME);
            PlayM4_OpenStream(pCamera->m_playPort, buffer, bufferSize, 1024 * 1024 * 4);
#if defined(_MSC_VER)
            PlayM4_SetDisplayCallBackEx(pCamera->m_playPort, HKCamera::displayCallback, reinterpret_cast<long>(pCamera));
#else
            PlayM4_SetDisplayCallBackEx(pCamera->m_playPort, HKCamera::displayCallback, (pCamera));
#endif
            PlayM4_Play(pCamera->m_playPort, 0);
        }
        break;
    case NET_DVR_STREAMDATA:
        PlayM4_InputData(pCamera->m_playPort, buffer, bufferSize);
        break;
    default:
        break;
    }
}

void HKCamera::displayCallback(DISPLAY_INFO *displayInfo)
{
    /*
    此处HK windows有问题
    HKCamera *pCamera = static_cast<HKCamera *>((void*)displayInfo->nUser);
    if (pCamera == NULL)
    {
        pCamera = &HKCamera::Instance();
    }
    */
#if defined(_MSC_VER)
    HKCamera *pCamera = pInstance;
#else
    HKCamera *pCamera = static_cast<HKCamera *>(displayInfo->nUser);
#endif
    switch (displayInfo->nType) {
    case T_YV12:
        if (pCamera) {
            pCamera->capture(displayInfo);
        }
        else {
            log4cpp::Category::getRoot().warn("NULL camera instance");
        }
        break;
    case T_RGB32:
//		if (pCamera->m_frameDataCallBack != NULL)
//		{
//			std::shared_ptr<frame_info_t> ptrFrameInfo = std::make_shared<frame_info_t>();
//			ptrFrameInfo->nBufLen = displayInfo->nBufLen;
//			ptrFrameInfo->nHeight = displayInfo->nHeight;
//			ptrFrameInfo->nWidth = displayInfo->nWidth;
//			ptrFrameInfo->nStamp = displayInfo->nStamp;
//			ptrFrameInfo->nType = displayInfo->nType;
//			ptrFrameInfo->nUser = displayInfo->nUser;
//			ptrFrameInfo->pBuf = new char[displayInfo->nBufLen];
//			memcpy(ptrFrameInfo->pBuf, displayInfo->pBuf, displayInfo->nBufLen);
//			pCamera->m_frameDataCallBack(ptrFrameInfo, pCamera->m_pUser);
//		}
        break;
    default:
        log4cpp::Category::getRoot().warn("Not supported frame format");
        break;
    }
}

void HKCamera::capture(const DISPLAY_INFO *displayInfo)
{
    HKCamera *pCamera = static_cast<HKCamera *>(displayInfo->nUser);
    std::shared_ptr<FrameInfo> ptrFrameInfo = std::make_shared<FrameInfo>();

    cv::Mat dst(displayInfo->nHeight, displayInfo->nWidth, CV_8UC3);//8UC3表示8bit uchar 无符号类型,3通道值
    cv::Mat src(displayInfo->nHeight + displayInfo->nHeight / 2, displayInfo->nWidth, CV_8UC1, displayInfo->pBuf);
    cv::cvtColor(src, dst, CV_YUV2BGR_YV12);
    ptrFrameInfo->mat = dst;

    task->Push(ptrFrameInfo);
}

bool HKCamera::initFaceConfig()
{
    BOOL res = FALSE;
//    const char *bufferIn = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
//                           "<EventAbility version=\"2.0\">"
//                           "    <channelNO>0</channelNO>"
//                           "</EventAbility>";

//    const int bufferLength = 4096;
//    char *bufferOut = new char[bufferLength];
//    memset(bufferOut, 0, bufferLength);
//    res = NET_DVR_GetDeviceAbility(m_userID, DEVICE_ABILITY_INFO, (char*)bufferIn, strlen(bufferIn), bufferOut, bufferLength);
//    if (res) {
//        logger.info("Device ability:\n%s", bufferOut);
//        delete [] bufferOut;
//    } else {
//        logger.error("Failed get device abilities");
//        delete [] bufferOut;
//        return false;
//    }

    NET_DVR_CHANNEL_GROUP paramsIn = { 0 };
    paramsIn.dwSize = sizeof(paramsIn);
    paramsIn.dwChannel = deviceInfo.byStartChan;
    paramsIn.byID = 1;
    NET_DVR_DETECT_FACE paramsOut = { 0 };
    paramsOut.dwSize = sizeof(paramsOut);
    DWORD status = 0;

    res = NET_DVR_GetDeviceConfig(m_userID, NET_DVR_GET_FACE_DETECT, 1,
                                       &paramsIn,
                                       sizeof(paramsIn),
                                       &status,
                                       &paramsOut,
                                       sizeof(paramsOut));
    if (res && (status == 0 || status == 1)) {
        logger.info("Succeeded get face detection params");
        logger.info("Enabled: %u", paramsOut.byEnableDetectFace);
        logger.info("Sensitivity: %u", paramsOut.byDetectSensitive);
        logger.info("Handle type: %u", paramsOut.struAlarmHandleType.dwHandleType);
        return true;
    } else {
        logger.info("Failed get face detection params, code: %u, message: %s", NET_DVR_GetLastError(), NET_DVR_GetErrorMsg());
        return false;
    }
}
