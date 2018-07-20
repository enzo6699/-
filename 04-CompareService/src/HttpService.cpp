#include "HttpService.h"
#include <event.h>
#include <string.h>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <limits.h>
#include <json/reader.h>
#include <json/writer.h>
#include <curl/curl.h>
#include <Base64Util.h>
#include <GuidUtil.h>
#include <StringUtil.h>
#include <opencv2/highgui/highgui.hpp>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ADD_PERSON "/compareservice/person/add"
#define REMOVE_PERSON "/compareservice/person/remove"
#define UPDATE_PERSON "/compareservice/person/update"
#define FEATURE "/compareservice/feature"
#define HEALTH "/compareservice/health"
#define SEARCH "/compareservice/search"

using std::map;
using std::shared_ptr;
using std::pair;
using std::vector;

static std::string content;
static size_t curlWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    content.append(ptr, size * nmemb);
    return size * nmemb;
}

HttpService::HttpService(std::shared_ptr<DataCache> cache) :
    cache(cache),
    logger(log4cpp::Category::getRoot()),
    config(PropertyConfigurator::Instance())
{
    faceall = std::make_shared<tce::FaceEngine>();
}

HttpService::~HttpService()
{
    if (http) {
        logger.debug("Release libevent http object");
        evhttp_free(http);
    }

    if (base) {
        logger.debug("Release libevent base object");
        event_base_free(base);
    }
}

void HttpService::start()
{
    base = event_base_new();
    if (!base) {
        throw std::runtime_error("Failed init libevent base");
    }

    http = evhttp_new(base);
    if (!http) {
        throw std::runtime_error("Failed init libevent http");
    }

    evhttp_bound_socket *handle = evhttp_bind_socket_with_handle(http, config.listenAddress.c_str(), config.listenPort);
    if (handle) {
        logger.info("Succeeded binding to %s:%d", config.listenAddress.c_str(), config.listenPort);
    } else {
        logger.error("Failed binding to %s:%d", config.listenAddress.c_str(), config.listenPort);
        throw std::runtime_error("Failed binding");
    }

    if (evhttp_set_cb(http, SEARCH, response, this) == 0) {
        logger.info("Succeeded init " SEARCH " interface");
    } else {
        logger.error("Failed init " SEARCH " interface");
    }
    if (evhttp_set_cb(http, FEATURE, response, this) == 0) {
        logger.info("Succeeded init " FEATURE " interface");
    } else {
        logger.error("Failed init " FEATURE " interface");
    }

    if (evhttp_set_cb(http, ADD_PERSON, response, this) == 0) {
        logger.info("Succeeded init " ADD_PERSON " interface");
    } else {
        logger.error("Failed init " ADD_PERSON " interface");
    }

    if (evhttp_set_cb(http, REMOVE_PERSON, response, this) == 0) {
        logger.info("Succeeded init " REMOVE_PERSON " interface");
    } else {
        logger.error("Failed init " REMOVE_PERSON " interface");
    }

    if (evhttp_set_cb(http, UPDATE_PERSON, response, this) == 0) {
        logger.info("Succeeded init " UPDATE_PERSON " interface");
    } else {
        logger.error("Failed init " UPDATE_PERSON " interface");
    }

    if (evhttp_set_cb(http, HEALTH, response, this) == 0) {
        logger.info("Succeeded init " HEALTH " interface");
    } else {
        logger.error("Failed init " HEALTH " interface");
    }

    if (!registerService()) {
        logger.warn("注册服务失败");
    }

//    evhttp_set_timeout(http, );
    event_base_dispatch(base);
}

void HttpService::response(evhttp_request *req, void *p)
{
    HttpService *service = static_cast<HttpService *>(p);
    log4cpp::Category &logger = service->logger;

    char *peerAddress = NULL;
    uint16_t peerPort = 0;
    std::string uri(evhttp_request_get_uri(req));
    evhttp_connection_get_peer(evhttp_request_get_connection(req), &peerAddress, &peerPort);
    logger.info("Request from %s:%d, uri: %s", peerAddress, peerPort, uri.c_str());

    std::string message = service->getContent(req);

    if (uri == FEATURE) {
        service->feature(message);
    } else if (uri == SEARCH) {
        service->search(message);
    } else if (uri == ADD_PERSON) {
        service->addPerson(message);
    } else if (uri == REMOVE_PERSON) {
        service->removePerson(message);
    } else if (uri == UPDATE_PERSON) {
        service->updatePerson(message);
    } else if (uri == HEALTH) {
        service->health();
    } else {
        evhttp_send_reply(req, 400, "Bad Request", NULL);
        return;
    }

    std::string reply = service->writer.write(service->wroot);
    logger.debug("Reply with data:% s", reply.c_str());
    char length[8] = { 0 };
    sprintf(length, "%lu", reply.length());

    evbuffer *replyBuffer = evbuffer_new();
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json; charset=UTF-8");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Length", length);
    evbuffer_add(replyBuffer, reply.c_str(), reply.length());

    evhttp_send_reply(req, 200, "OK", replyBuffer);
    evbuffer_free(replyBuffer);
}

void HttpService::feature(const std::string &message)
{
    rroot.clear();
    wroot.clear();

    if (!reader.parse(message, rroot)) {
        logger.error("Failed parsing %s", message.c_str());
        wroot["result"] = 4021;
        wroot["errorMessage"] = "JSON数据格式错误";
        return;
    }

    std::string image = rroot["image"].asString();
    std::string url = rroot["url"].asString();

    if (image.empty() && url.empty()) {
        logger.error("Both image and url are empty");
        wroot["result"] = 4022;
        wroot["errorMessage"] = "参数错误（图片为空）";
        return;
    }

    std::string data;
    vector<uchar> dataVector;
    cv::Mat originalMat;
    if (image.size() > 0) {
        data = tce::Base64Util::base64_decode(image);
    }
    try {
        logger.debug("Use image data");
        dataVector = vector<uchar>(data.c_str(), data.c_str() + data.size());
        originalMat = cv::imdecode(cv::Mat(dataVector), CV_LOAD_IMAGE_COLOR | CV_LOAD_IMAGE_ANYCOLOR);
    } catch (cv::Exception &ex) {
        logger.warn("Failed decoding data from image: %s", ex.what());
        originalMat = cv::Mat();
    }
    if (originalMat.empty()) {
        logger.debug("Image data is invalid, try data from url");
        data = download(url);
        try {
            dataVector = vector<uchar>(data.c_str(), data.c_str() + data.size());
            originalMat = cv::imdecode(cv::Mat(dataVector), CV_LOAD_IMAGE_COLOR | CV_LOAD_IMAGE_ANYCOLOR);
        } catch (cv::Exception &ex) {
            logger.warn("Failed decoding data from url: %s", ex.what());
            originalMat = cv::Mat();
        }
    }
    if (originalMat.empty()) {
        logger.debug("Data from url is invalid either");
        wroot["result"] = 4022;
        wroot["errorMessage"] = "参数错误（url及base64编码均无效）";
        return;
    }

    cv::Rect faceRect;
    vector<shared_ptr<tce::FaceResult>> faces = faceall->detect(originalMat);
    if (faces.size() == 0) {
        wroot["result"] = 4024;
        wroot["errorMessage"] = "未检测到人脸";
        return;
    }
    if (faces.size() != 1) {
        wroot["result"] = 4018;
        wroot["errorMessage"] = "检测到多张人脸";
        return;
    }
    shared_ptr<tce::FaceResult> face = faces[0];
#ifndef USE_SEETA
    faceall->getQuality(originalMat, face);
    if (face->quality < config.qualityThreshold) {
        wroot["result"] = 4023;
        wroot["objectQuality"] = face->quality;
        wroot["errorMessage"] = "图片质量不达标（请使用正脸照片并确保光线适宜）";
        return;
    }
#endif

    faceall->feature(originalMat, face);

    cv::Mat objectMat;
    try {
        objectMat = originalMat(faceRect);
    } catch (cv::Exception &ex) {
        logger.error("Failed crop face: %s", ex.what());
        wroot["result"] = 4022;
        wroot["errorMessage"] = "图片无效，无法剪裁人脸";
        return;
    }

    std::string originalUrl = save(originalMat);
    if (originalUrl.empty()) {
        wroot["result"] = 4025;
        wroot["errorMessage"] = "上传原图失败";
        return;
    }
    std::string objectUrl = save(objectMat);
    if (objectUrl.empty()) {
        wroot["result"] = 4026;
        wroot["errorMessage"] = "上传人脸图片失败";
        return;
    }

    wroot["result"] = 0;
    wroot["errorMessage"] = "Success";
    wroot["feature"] = tce::Base64Util::base64_encode((char*)face->feature.data(), face->feature.size() * sizeof(float));
    wroot["originalUrl"] = originalUrl;
    wroot["objectUrl"] = objectUrl;
#ifndef USE_SEETA
    wroot["objectQuality"] = face->quality;
#endif
}

void HttpService::removePerson(const std::string &message)
{
    rroot.clear();
    wroot.clear();

    if (!reader.parse(message, rroot)) {
        logger.error("Failed parsing %s", message.c_str());
        wroot["result"] = 4021;
        wroot["errorMessage"] = "JSON数据格式错误";
        return;
    }

    std::string personId = rroot["personId"].asString();
    Json::Value faceId = rroot["faceId"];

    if (personId.empty()) {
        logger.error("personId is empty");
        wroot["result"] = 4022;
        wroot["errorMessage"] = "personId为空";
        return;
    }

    vector<std::string> faceIds;
    for (Json::ArrayIndex index = 0; index < faceId.size(); index++) {
        faceIds.push_back(faceId[index].asString());
    }
    if (cache->remove(personId, faceIds) == 0) {
        wroot["result"] = 4004;
        wroot["errorMessage"] = "删除人员失败: " + cache->getLastError();
    } else {
        wroot["result"] = 0;
        wroot["errorMessage"] = "Success";
    }
}

void HttpService::updatePerson(const std::string &message)
{
    rroot.clear();
    wroot.clear();

    if (!reader.parse(message, rroot)) {
        logger.error("Failed parsing %s", message.c_str());
        wroot["result"] = 4021;
        wroot["errorMessage"] = "JSON数据格式错误";
        return;
    }

    std::string personId = rroot["personId"].asString();
    std::string personType = rroot["personType"].asString();
    std::string personName = rroot["personName"].asString();

    if (personId.empty()) {
        logger.error("personId is empty");
        wroot["result"] = 4022;
        wroot["errorMessage"] = "personId为空";
        return;
    }
    if (personType.empty()) {
        logger.error("personType is empty");
        wroot["result"] = 4022;
        wroot["errorMessage"] = "personType为空";
        return;
    }

    if (cache->updatePerson(personId, personType, personName) < 0) {
        logger.error("Failed update person: %s", personId.c_str());
        wroot["result"] = 4044;
        wroot["errorMessage"] = "修改人员类型失败：找不到对应的personId(" + personId + ")";
        return;
    }

    wroot["result"] = 0;
    wroot["errorMessage"] = "Success";
}

void HttpService::health()
{
    wroot.clear();
    wroot["result"] = 0;
    wroot["errorMessage"] = "Success";
    wroot["capacity"] = 10;
}

void HttpService::addPerson(const std::string &message)
{
    rroot.clear();
    wroot.clear();

    if (!reader.parse(message, rroot)) {
        logger.error("Failed parsing %s", message.c_str());
        wroot["result"] = 4021;
        wroot["errorMessage"] = "JSON数据格式错误";
        return;
    }

    Json::Value persons = rroot["persons"];
    logger.info("Adding persons to data cache, count: %d", persons.size());

    for (Json::ArrayIndex index = 0; index < persons.size(); index++) {
        Json::Value person = persons[index];
        std::string personId = person["personId"].asString();
        std::string personType = person["personType"].asString();
        std::string personName = person["personName"].asString();

        logger.info("##### Person %d ####", index + 1);
        logger.info("personId: %s", personId.c_str());
        logger.info("personType: %s", personType.c_str());
        logger.info("personName: %s", personName.c_str());

        if (personId.empty() || personType.empty()) {
            logger.error("Person params are invalid");
            wroot["result"] = 4022;
            wroot["errorMessage"] = "人员参数错误";
            return;
        }

        DataCache::Person *personp = new DataCache::Person();
        personp->personId = personId;
        personp->personType = personType;
        personp->personName = personName;

        Json::Value faces = person["faces"];
        logger.info("Adding faces to person of id: %s, count: %d", personId.c_str(), faces.size());
        if (faces.size() == 0) {
            logger.error("No faces for person: %s", personId.c_str());
            wroot["result"] = 4006;
            wroot["errorMessage"] = "人脸数据为空，personId: " + personId;
            return;
        }

        for (Json::ArrayIndex index2 = 0; index2 < faces.size(); index2++) {
            Json::Value face = faces[index2];

            std::string faceId = face["faceId"].asString();
            std::string objectUrl = face["objectUrl"].asString();
            std::string feature = face["feature"].asString();
            if (faceId.empty() || personType.empty() || feature.size() < 16) {
                logger.error("Face params are invalid");
                wroot["result"] = 4022;
                wroot["errorMessage"] = "人脸参数错误";
                return;
            }

            std::string featureStart = feature.substr(0, 16);
            std::string featureEnd = feature.substr(feature.size() - 16, 16);

            logger.info("##### Face %d ####", index2 + 1);
            logger.info("faceId: %s", faceId.c_str());
            logger.info("feature: %s...%s", featureStart.c_str(), featureEnd.c_str());
            logger.info("objectUrl: %s", objectUrl.c_str());

            DataCache::Face *facep = new DataCache::Face();
            facep->faceId = faceId;
            facep->feature = feature;
            facep->objectUrl = objectUrl;

            personp->faces[faceId] = shared_ptr<DataCache::Face>(facep);
        }

        if (cache->add(shared_ptr<DataCache::Person>(personp)) == 0) {
            wroot["result"] = 4005;
            wroot["errorMessage"] = "添加人员失败";
            return;
        }
    }

    wroot["result"] = 0;
    wroot["errorMessage"] = "Success";
}

std::string HttpService::postJson(const std::string &json, const std::string &url)
{
    CURL *curl = curl_easy_init();
    if (!curl) {
        logger.error("Failed init curl");
        return "";
    }

    char error[CURL_ERROR_SIZE] = { 0 };
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);

    logger.info("Sending:\n%s==> %s", json.c_str(), url.c_str());

    content.clear();

    curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json; charset=UTF-8");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 2000L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    CURLcode code = curl_easy_perform(curl);
    if (code == CURLE_OK) {
        logger.info("发送成功");
    } else {
        logger.error("发送失败: %s", error);
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    logger.info("Received:\n%s", content.c_str());

    return content;
}

void HttpService::search(const std::string &message)
{
    rroot.clear();
    wroot.clear();

    if (!reader.parse(message, rroot)) {
        logger.error("Failed parsing %s", message.c_str());
        wroot["result"] = 6001;
        wroot["errorMessage"] = "JSON数据格式错误";
        return;
    }

    int topN = rroot["topN"].asInt();
    std::string personType = rroot["personType"].asString();
    std::string image = rroot["image"].asString();
    std::string url = rroot["url"].asString();

    std::string imagePart;
    if (image.size() > 32) {
        imagePart = std::string(image, 0, 16);
        imagePart += "......";
        imagePart += std::string(image, image.size() -16);
    }

    logger.info("search params ==>");
    logger.info("topN: %d", topN);
    logger.info("personType: %s", personType.c_str());
    logger.info("url: %s", url.c_str());
    logger.info("image: %s", imagePart.c_str());
    logger.info("<== search params");

    std::string data;
    vector<uchar> dataVector;
    cv::Mat originalMat;

    if (image.size() > 0) {
        try {
            logger.debug("Try to use image data");
            data = tce::Base64Util::base64_decode(image);
            dataVector = vector<uchar>(data.c_str(), data.c_str() + data.size());
            originalMat = cv::imdecode(cv::Mat(dataVector), CV_LOAD_IMAGE_COLOR | CV_LOAD_IMAGE_ANYCOLOR);
        } catch (cv::Exception &ex) {
            logger.warn("Failed decoding data from image: %s", ex.what());
            originalMat = cv::Mat();
        }
    }
    if (originalMat.empty()) {
        if (url.size() > 0) {
            try {
                logger.debug("Image data is invalid, try data from url");
                data = download(url);
                dataVector = vector<uchar>(data.c_str(), data.c_str() + data.size());
                originalMat = cv::imdecode(cv::Mat(dataVector), CV_LOAD_IMAGE_COLOR | CV_LOAD_IMAGE_ANYCOLOR);
            } catch (cv::Exception &ex) {
                logger.warn("Failed decoding data from url: %s", ex.what());
                originalMat = cv::Mat();
            }
        }
    }
    if (originalMat.empty()) {
        logger.error("Neither image nor url is valid");
        wroot["result"] = 6002;
        wroot["errorMessage"] = "参数错误（url及base64编码均无效）";
        return;
    }

    vector<shared_ptr<tce::FaceResult>> detectResults = faceall->detect(originalMat);
    shared_ptr<tce::FaceResult> maxFace = tce::FaceEngine::getMaxFace(detectResults);
    if (!maxFace) {
        wroot["result"] = 6003;
        wroot["errorMessage"] = "未检测到人脸";
        return;
    }

    cv::Mat objectMat;
    try {
        objectMat = originalMat(maxFace->rect);
    } catch (cv::Exception &ex) {
        logger.error("Failed crop face: %s", ex.what());
        wroot["result"] = 6006;
        wroot["errorMessage"] = "裁剪人脸图片失败";
        return;
    }

    std::string originalUrl = save(originalMat);
    if (originalUrl.empty()) {
        wroot["result"] = 6004;
        wroot["errorMessage"] = "保存原图失败";
        return;
    }
    std::string objectUrl = save(objectMat);
    if (objectUrl.empty()) {
        wroot["result"] = 6005;
        wroot["errorMessage"] = "保存人脸图片失败";
        return;
    }

    vector<shared_ptr<SearchResult>> results = cache->findFace(maxFace->feature, personType, topN);
    Json::Value faces(Json::arrayValue);
    for (shared_ptr<SearchResult> result: results) {
        if (result->confidence < config.compareThreshold) {
            logger.debug("Confidence(%.4f) too low, personId: %s, faceId: %s", result->confidence, result->personId.c_str(), result->faceId.c_str());
            continue;
        }

        Json::Value face;
        face["personId"] = result->personId;
        face["personType"] = result->personType;
        face["personName"] = result->personName;
        face["faceId"] = result->faceId;
        face["objectUrl"] = result->objectUrl;
        face["confidence"] = result->confidence;
        faces.append(face);
    }

    wroot["result"] = 0;
    wroot["errorMessage"] = "Success";
    wroot["faces"] = faces;
}

bool HttpService::isHeaderValid(struct evhttp_request *req)
{
    evkeyvalq *headers = evhttp_request_get_input_headers(req);
    //    logger.info("Input header:");
    bool valid = false;
    for (evkeyval *header = headers->tqh_first; header; header = header->next.tqe_next) {
        //        logger.info("%s: %s", header->key, header->value);
        std::string key(header->key);
        std::string value(header->value);
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        if (key == "content-type" && value == "application/json") {
            valid = true;
            break;
        }
    }

    return valid;
}

std::string HttpService::download(const std::string &url)
{
    content.clear();
    logger.info("Downloading %s", url.c_str());

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        logger.error("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        content.clear();
    } else {
        logger.info("Downloading succeeded, size: %u bytes\n", content.size());
    }

    curl_easy_cleanup(curl);
    return content;
}

std::string HttpService::currentDay()
{
    time_t timestamp = time(NULL);
    struct tm *ltm = localtime(&timestamp);
    char date[16] = { 0 };
    strftime(date, sizeof(date), "%Y/%m/%d", ltm);

    return date;
}

std::string HttpService::upload(const std::string &filepath, const std::string &filename)
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

    std::string url = config.nginxAddress + currentDay() + "/" + filename;

    logger.info("Uploading %s", filepath.c_str());
    logger.info("Remote url: %s", url.c_str());

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

    return url;
}

std::string HttpService::save(cv::Mat &mat)
{
    time_t t = time(NULL);
    tm *timeInfo = localtime(&t);
    char date[12] = { 0 };
    char hour[4] = { 0 };
    strftime(date, sizeof(date), "%Y%m%d", timeInfo);
    strftime(hour, sizeof(hour), "%H", timeInfo);

    std::string directory = config.localDirectory + "/" + date + "/" + hour;
    struct stat st = { 0 };
    stat(directory.c_str(), &st);
    if (!S_ISDIR(st.st_mode)) {
        logger.debug("Creating directory '%s'", directory.c_str());
        std::string command = "mkdir -p " + directory;
        system(command.c_str());
    }

    std::string filename = tce::GuidUtil::gguid() + ".jpg";;
    std::string localPath = directory + "/" + filename;
    cv::imwrite(localPath.c_str(), mat);

    return upload(localPath, filename);
}

std::string HttpService::getContent(evhttp_request *req)
{
    evbuffer *buf = evhttp_request_get_input_buffer(req);
    size_t length = EVBUFFER_LENGTH(buf);
    unsigned char *data= EVBUFFER_DATA(buf);
    std::string message;
    message.append(data, data + length);

    return message;
}

std::string HttpService::getLocalIP()
{
    ifaddrs *ifAddrs = NULL;
    getifaddrs(&ifAddrs);
    for (ifaddrs *ifa = ifAddrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }

        if (ifa->ifa_addr->sa_family == AF_INET) {
            sockaddr_in *sockAddr = (sockaddr_in *)ifa->ifa_addr;
            char addressBuffer[INET_ADDRSTRLEN] = { 0 };
            inet_ntop(AF_INET, &sockAddr->sin_addr, addressBuffer, INET_ADDRSTRLEN);
            logger.info("IP of %s: %s", ifa->ifa_name, addressBuffer);
            if (strcmp(addressBuffer, "127.0.0.1") == 0 || strcmp(ifa->ifa_name, "lo") == 0) {
                continue;
            }
            return addressBuffer;
        }
    }

    return "";
}

bool HttpService::registerService()
{
    char nodeId[32] = { 0 };
    sprintf(nodeId, "%s:%d", config.listenAddress.c_str(), config.listenPort);
    rroot["nodeId"] = nodeId;

    rroot["nodeType"] = "compare";
    rroot["version"] = "1.0";
    rroot["capacity"] = config.maxTaskNum;

    char url[NAME_MAX] = { 0 };
    sprintf(url, "http://%s:%d/", config.listenAddress.c_str(), config.listenPort);
    rroot["url"] = url;

    std::string json = writer.write(rroot);
    std::string reply = postJson(json, config.registerUrl);

    Json::Value rroot;
    if (!reader.parse(reply, rroot) || rroot["result"].asInt() != 0) {
        return false;
    }
    return true;
}
