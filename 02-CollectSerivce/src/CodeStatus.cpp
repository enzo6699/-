#include "CodeStatus.h"
#include "json/json.h"
#include "utils/ConvertCharCode.h"
#include "common/Logger.h"

using std::string;

static std::map<ErrorCode, std::string> errors;

void Reply(http_msg_t* pHttp_msg, std::string strWrite)
{
    pHttp_msg->respone_data = strWrite;
//	tce::ConvertCharCode::AnsiToUTF8(strWrite, pHttp_msg->respone_data);
    log4cpp::Category::getRoot().info(pHttp_msg->respone_data.c_str());
}

std::string GetErrorMessage(int errorCode)
{
    if (errors.empty()) {
        errors[ErrorCode::Success] = "成功";
        errors[ErrorCode::DeleteTask] = "删除任务";
        errors[ErrorCode::StartFaceSDKError] = "算法初始化失败";
        errors[ErrorCode::NoSuchUser] = "用户不存在";
        errors[ErrorCode::UserIsLocked] = "用户被锁定";
        errors[ErrorCode::WrongPassword] = "密码错误";
        errors[ErrorCode::DeviceConnectionError] = "设备无法连接";
        errors[ErrorCode::InvalidJsonData] = "json数据无效";
        errors[ErrorCode::InvalidTaskParam] = "任务参数错误";
        errors[ErrorCode::TaskIdReplicated] = "任务重复";
        errors[ErrorCode::TaskReachedLimit] = "处理能力已满";
        errors[ErrorCode::TaskIdDoesNotExist] = "任务不存在";
        errors[ErrorCode::UnknownError] = "未知的设备错误";
    }

    return errors[(ErrorCode)errorCode];
}
