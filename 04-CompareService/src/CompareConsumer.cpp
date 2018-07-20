#include "CompareConsumer.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <GuidUtil.h>
#include <Base64Util.h>
#include <ConsumerImpl.h>
#include <rdkafkacpp.h>
#include <StringUtil.h>

using namespace tce;
using namespace std;

CompareConsumer::CompareConsumer(std::shared_ptr<DataCache> cache) :
    cache(cache),
    logger(log4cpp::Category::getRoot()),
    config(PropertyConfigurator::Instance())
{
    producer.Create(config.broker, config.producerTopic, false);
}

void CompareConsumer::consume_cb(RdKafka::Message &message, void *opaque)
{
    if (message.err() == RdKafka::ERR_NO_ERROR) {
        string mess((char *)message.payload(), message.len());
        handleMessage(mess);
    }
}

void CompareConsumer::handleMessage(const string &message)
{
    logger.info("Received collect result: %s", message.c_str());

    wroot.clear();
    rroot.clear();

    if (!reader.parse(message, rroot)) {
        logger.error("Invalid json data: %s", message.c_str());
        wroot["result"] = 6001;
        wroot["errorMessage"] = "JSON数据格式错误";
        return;
    }

    string collectFaceId = rroot["collectFaceId"].asString();
    string cameraId = rroot["cameraId"].asString();
    string feature = rroot["feature"].asString();
    string originalUrl = rroot["originalUrl"].asString();
    string objectUrl = rroot["objectUrl"].asString();
    string objectRect = rroot["objectRect"].asString();
    float objectQuality = rroot["objectQuality"].asFloat();
    int timestamp = rroot["timestamp"].asInt();
//    Json::Value attribute = rroot["attribute"];

    if (collectFaceId.empty() ||
        cameraId.empty() ||
        feature.empty() ||
        originalUrl.empty() ||
        objectUrl.empty()) {
        wroot["result"] = 6001;
        wroot["errorMessage"] = "JSON参数错误";
        return;
    }

    if (objectQuality < config.alarmQualityThreshold) {
        logger.debug("抓拍人脸质量(%.4f)不达标，图片地址：%s", objectQuality, originalUrl.c_str());
        return;
    }

    for (const std::string &type: cache->getTypes()) {
        std::string decoded = tce::Base64Util::base64_decode(feature);
        float *start = (float*)decoded.c_str();
        std::vector<float> rawFeature(start, start + decoded.size() / sizeof(float));
        std::vector<std::shared_ptr<SearchResult>> results = cache->findFace(rawFeature, type, 1);
        if (results.size() == 0) {
            logger.debug("类型为%s的人员库为空", type.c_str());
            continue;
        }

        std::shared_ptr<SearchResult> target = results[0];
        if (target->confidence < config.compareThreshold) {
            logger.debug("类型为%s的人员库没有满足阈值(%.2f)的人脸", type.c_str(), config.compareThreshold);
            continue;
        }

        wroot["collectFaceId"] = collectFaceId;
        wroot["cameraId"] = cameraId;
        wroot["feature"] = feature;
        wroot["originalUrl"] = originalUrl;
        wroot["objectUrl"] = objectUrl;
        wroot["personId"] = target->personId;
        wroot["personType"] = target->personType;
        wroot["personName"] = target->personName;
        wroot["faceId"] = target->faceId;
        wroot["faceObjectUrl"] = target->objectUrl;
        wroot["confidence"] = target->confidence;
        wroot["timestamp"] = timestamp;

        producer.send(writer.write(wroot));
    }
}
