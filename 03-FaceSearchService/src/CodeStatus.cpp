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

std::string createError(http_msg_t *pHttp_msg, int errorCode, std::string message)
{
    if (errors.empty()) {
        errors[ErrorCode::success] = "成功";
		errors[ErrorCode::start_facesdk_error] = "启动分析算法失败";
		errors[ErrorCode::function_is_not_supported] = "不支持的方法";
		errors[ErrorCode::param_error] = "参数错误";
		errors[ErrorCode::interal_error] = "内部错误";
		errors[ErrorCode::parser_json_failed] = "json协议解析错误";
		errors[ErrorCode::image_data_is_empty] = "图片数据为空";
		errors[ErrorCode::image_format_error] = "图片数据格式错误";
		errors[ErrorCode::no_faces_in_image] = "图片中未找到人脸";
		errors[ErrorCode::not_found_similar_faces] = "未找到相似的人脸";
		errors[ErrorCode::read_loacl_faceinfo_error] = "sqlite获取人脸信息错误";
    }

    string errorMessage = errors[(ErrorCode)errorCode];
    Json::Value wroot;
    wroot.clear();

    wroot["result"] = errorCode;
	if (message.length() > 0)
		wroot["errorMessage"] = errorMessage + ":" + message;
	else
		wroot["errorMessage"] = errorMessage;

	Json::FastWriter writer;
    pHttp_msg->respone_data = writer.write(wroot);

    return errorMessage;
}

std::string createError(int errorCode, std::string message)
{
	if (errors.empty()) {
		errors[ErrorCode::success] = "成功";
		errors[ErrorCode::start_facesdk_error] = "启动分析算法失败";
		errors[ErrorCode::function_is_not_supported] = "不支持的方法";
		errors[ErrorCode::param_error] = "参数错误";
		errors[ErrorCode::interal_error] = "内部错误";
		errors[ErrorCode::parser_json_failed] = "json协议解析错误";
		errors[ErrorCode::image_data_is_empty] = "图片数据为空";
		errors[ErrorCode::image_format_error] = "图片数据格式错误";
		errors[ErrorCode::no_faces_in_image] = "图片中未找到人脸";
		errors[ErrorCode::not_found_similar_faces] = "未找到相似的人脸";
		errors[ErrorCode::read_loacl_faceinfo_error] = "sqlite获取人脸信息错误";
	}

	string errorMessage = errors[(ErrorCode)errorCode];
	Json::Value wroot;
	wroot.clear();

	wroot["result"] = errorCode;
	if (message.length() > 0)
		wroot["errorMessage"] = errorMessage + ":" + message;
	else
		wroot["errorMessage"] = errorMessage;

	Json::FastWriter writer;

	return writer.write(wroot);
}