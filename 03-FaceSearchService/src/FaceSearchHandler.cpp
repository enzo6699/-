#include "FaceSearchHandler.h"
#include <list>
#include "evhttp.h"
#include "common/Logger.h"
#include "common/Logger.h"
#include "utils/GuidUtil.h"
#include "utils/Base64Util.h"
#include "utils/StringUtil.h"
#include "utils/FileUtil.h"
#include "utils/GuidUtil.h"
#include "utils/FileUtil.h"
#include "CodeStatus.h"
#include "json/json.h"
#include "PropertyConfigurator.h"

log4cpp::Category &logger = log4cpp::Category::getRoot();

FaceSearchHandler::FaceSearchHandler()
{
}

FaceSearchHandler::~FaceSearchHandler()
{
}


void FaceSearchHandler::dealMsg(http_msg_t* pHttp_msg)
{
	if (pHttp_msg == nullptr || pHttp_msg->req == nullptr)
	{
		log4cpp::Category::getRoot().error("内部错误：pHttp_msg 为空");
		return;
	}

	if (pHttp_msg->request_data.length() == 0)
	{
		log4cpp::Category::getRoot().error("param is null");
		createError(pHttp_msg, ErrorCode::param_error, "json param is null");
		return;
	}

	Json::Reader reader;
	Json::Value root;

	try
	{
		if (!reader.parse(pHttp_msg->request_data, root, true))
		{
			log4cpp::Category::getRoot().error("parse json failed:%s", pHttp_msg->request_data.c_str());
			createError(pHttp_msg, ErrorCode::parser_json_failed);
			return;
		}
	}
	catch (std::exception ex)
	{
		log4cpp::Category::getRoot().error("parse json failed:%s", ex.what());
		createError(pHttp_msg, ErrorCode::parser_json_failed);
		return;
	}

	face_search_req_t face_search_req;
	try
	{
		// 获取检索条件
		if (root["startTime"].isNull())
		{
			createError(pHttp_msg, ErrorCode::param_error, "startTime is null");
			return;
		}

		if (root["endTime"].isNull())
		{
			createError(pHttp_msg, ErrorCode::param_error, "endTime is null");
			return;
		}

		if (root["topN"].isNull())
		{
			createError(pHttp_msg, ErrorCode::param_error, "topN is null");
			return;
		}

		if (root["url"].isNull() && root["image"].isNull())
		{
			createError(pHttp_msg, ErrorCode::param_error, "url is null");
			return;
		}

        face_search_req.minConfidence = PropertyConfigurator::Instance().compareThreshold;
		face_search_req.startTime = root["startTime"].asInt();
		face_search_req.endTime = root["endTime"].asInt();
		face_search_req.topN = root["topN"].asInt();
		face_search_req.topN = face_search_req.topN <= 0 ? 10 : face_search_req.topN;
		face_search_req.topN = face_search_req.topN > 1000 ? 1000 : face_search_req.topN;

		if (!root["personType"].isNull())
			face_search_req.personType = root["personType"].asString();
		if (!root["url"].isNull())
			face_search_req.url = root["url"].asString();
		if (!root["image"].isNull())
			face_search_req.image = root["image"].asString();

		if (!root["cameraIds"].isNull())
		{
			for (int i = 0; i < root["cameraIds"].size(); i++)
			{
				face_search_req.vecCameraId.push_back(root["cameraIds"][i].asString());
			}
		}
	}
	catch (std::exception ex)
	{
		createError(pHttp_msg, ErrorCode::param_error);
		return;
	}

	if (face_search_req.endTime <= face_search_req.startTime)
	{
		createError(pHttp_msg, ErrorCode::param_error, "endTime < startTime");
		return;
	}

    std::string imagePart;
    if (face_search_req.image.size() > 32) {
        imagePart = std::string(face_search_req.image, 0, 16);
        imagePart += "......";
        imagePart += std::string(face_search_req.image, face_search_req.image.size() - 16);
    }
    std::string cameras;
    for (int index = 0; index < face_search_req.vecCameraId.size(); index++) {
        cameras.append(face_search_req.vecCameraId[index]);
        if (index != face_search_req.vecCameraId.size() - 1) {
            cameras.append(",");
        }
    }

    logger.info("search params ==>");
    logger.info("cameraIds: %s", cameras.c_str());
    logger.info("startTime: %d", face_search_req.startTime);
    logger.info("endTime: %d", face_search_req.endTime);
    logger.info("topN: %d", face_search_req.topN);
    logger.info("url: %s", face_search_req.url.c_str());
    logger.info("image: %s", imagePart.c_str());
    logger.info("minConfidence: %.4f", face_search_req.minConfidence);
    logger.info("<== search params");

	// 检索业务逻辑
    std::vector<std::shared_ptr<compare_result_t>> resultList;
	int nRet = FaceIdentify::Instance().Search(face_search_req, resultList);
	if (nRet != 0)
	{
		createError(pHttp_msg, nRet, "人脸搜索失败");
		return;
	}

	if (resultList.size() == 0)
	{
		createError(pHttp_msg, ErrorCode::not_found_similar_faces);
		return;
	}

	Json::Value wroot;
	wroot.clear();

	Json::Value wresults;
    for (auto result: resultList)
    {
		Json::Value wresult;
        wresult["personId"] = result->personId;
        wresult["cameraId"] = result->cameraId;
        wresult["originalUrl"] = result->originalUrl;
        wresult["objectUrl"] = result->objectUrl;
        wresult["timestamp"] = result->timestamp;
        wresult["confidence"] = result->confidence;
		wresults.append(wresult);
	}

	wroot["searchResult"] = wresults;
	wroot["result"] = 0;
	wroot["errorMessage"] = "success";
	Json::FastWriter writer;
	std::string strWrite = writer.write(wroot);

	Reply(pHttp_msg, strWrite);
}
