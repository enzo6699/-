#ifndef _CODE_STATUS_H_
#define _CODE_STATUS_H_

#include "HttpMsg.h"

enum ErrorCode
{
    Success = 0,
    DeleteTask = 2,
    StartFaceSDKError = 10,
    NoSuchUser,
    UserIsLocked,
    WrongPassword,
    DeviceConnectionError,
    InvalidJsonData,
    InvalidTaskParam,
    TaskIdReplicated,
    TaskReachedLimit,
    TaskIdDoesNotExist,
    UnknownError = 127
};

std::string GetErrorMessage(int errorCode);

void Reply(http_msg_t* pHttp_msg, std::string strWrite);

#endif
