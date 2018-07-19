#include "FaceDataConsumerCb.h"
#include "json/json.h"
#include "utils/Base64Util.h"
#include "utils/StringUtil.h"
#include <string.h>


FaceDataConsumerCb::FaceDataConsumerCb(SqliteFaceCache *pSqliteFaceCache)
    : m_pSqliteFaceCache(pSqliteFaceCache)
    , logger(log4cpp::Category::getRoot())
    , config(PropertyConfigurator::Instance())
{
}

FaceDataConsumerCb::~FaceDataConsumerCb()
{
    //TIMERMANAGE->unRegister(m_timer);
}

void FaceDataConsumerCb::consume_cb(RdKafka::Message &msg, void *opaque)
{
    switch (msg.err()) {
    case RdKafka::ERR_NO_ERROR: {
        RdKafka::MessageTimestamp ts;
        ts = msg.timestamp();
        if (ts.type != RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE) {
            std::string tsname = "?";
            if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME)
                tsname = "create time";
            else if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_LOG_APPEND_TIME)
                tsname = "log append time";
            log4cpp::Category::getRoot().debug("Timestamp:%s %d", tsname.c_str(), ts.timestamp);
        }

        dealMsg(static_cast<const char *>(msg.payload()), msg.len());
    }
    default:
        break;
    }
}

void FaceDataConsumerCb::dealMsg(const char *data, size_t nLen)
{
    Json::Reader reader;
    Json::Value root;

    try
    {
        if (!reader.parse(data, data + nLen, root, true))
        {
            log4cpp::Category::getRoot().error("kafka message parse json failed:%s", data);
            return;
        }
    }
    catch (std::exception ex)
    {
        log4cpp::Category::getRoot().error("kafka message parse json failed:%s", ex.what());
        return;
    }

    if (root["collectFaceId"].isNull() || root["cameraId"].isNull() || root["feature"].isNull()
            || root["originalUrl"].isNull() || root["objectUrl"].isNull() || root["timestamp"].isNull())
    {
        log4cpp::Category::getRoot().error("kafka message param error:%s", data);
        return;
    }

    std::string collectFaceId = root["collectFaceId"].asString();

    std::string cameraId = root["cameraId"].asString();
    std::string feature = root["feature"].asString();
    std::string originalUrl = root["originalUrl"].asString();
    std::string objectUrl = root["objectUrl"].asString();
    std::string objectRect = root["objectRect"].asString();
    float objectQuality = root["objectQuality"].asFloat();
    int timestamp = root["timestamp"].asInt();

    std::string featurePart = std::string(feature, 0, 16);
    featurePart += "......";
    featurePart += std::string(feature, feature.size() - 16);
    logger.info("==> collect message");
    logger.info("collectFaceId: %s", collectFaceId.c_str());
    logger.info("cameraId: %s", cameraId.c_str());
    logger.info("feature: %s", featurePart.c_str());
    logger.info("originalUrl: %s", originalUrl.c_str());
    logger.info("objectUrl: %s", objectUrl.c_str());
    logger.info("objectRect: %s", objectRect.c_str());
    logger.info("objectQuality: %.4f", objectQuality);
    logger.info("timestamp: %d", timestamp);
    logger.info("<== collect message");

    std::string featureDecode = tce::Base64Util::base64_decode(feature);

    int nRet = m_pSqliteFaceCache->Insert(collectFaceId.c_str(), cameraId.c_str(), originalUrl.c_str(), objectUrl.c_str(),
                                          featureDecode, objectRect, objectQuality, timestamp);

    if (nRet != 0)
    {
        log4cpp::Category::getRoot().error("sqlite insert error");
    }
}
