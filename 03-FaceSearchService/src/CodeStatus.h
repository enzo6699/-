#ifndef _CODE_STATUS_H_
#define _CODE_STATUS_H_

#include "HttpMsg.h"

enum ErrorCode
{
    success = 0,
    start_facesdk_error = 20001,
	function_is_not_supported,
	param_error,
	interal_error,
	parser_json_failed,
	image_data_is_empty,
    image_format_error,
	no_faces_in_image,
	not_found_similar_faces,
	read_loacl_faceinfo_error
};

std::string createError(http_msg_t* pHttp_msg, int errorCode, std::string message = "");
std::string createError(int errorCode, std::string message);

void Reply(http_msg_t* pHttp_msg, std::string strWrite);

#endif
