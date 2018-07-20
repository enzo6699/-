#ifndef __HTTP_SERVICE_H__
#define __HTTP_SERVICE_H__

#include "PropertyConfigurator.h"
#include "DataCache.h"
#include <memory>
#include <event2/http.h>
#include <log4cpp/Category.hh>
#include <json/value.h>
#include <json/writer.h>
#include <json/reader.h>
#include <opencv2/core/core.hpp>

class HttpService
{
public:
    HttpService(std::shared_ptr<DataCache> cache);
    ~HttpService();

    void start();

private:
    static void response(struct evhttp_request *req, void *p);
    void removePerson(const std::string &message);
    void updatePerson(const std::string &message);
    void addPerson(const std::string &message);
    void feature(const std::string &message);
    void search(const std::string &message);
    void health();

    std::string postJson(const std::string &json, const std::string &url);
    bool isHeaderValid(struct evhttp_request *req);
    std::string download(const std::string &url);
    std::string upload(const std::string &filepath, const std::string &filename);
    std::string save(cv::Mat &mat);
    std::string getContent(struct evhttp_request *req);
    std::string getLocalIP();
    std::string currentDay();
    bool registerService();

    Json::Value rroot;
    Json::Value wroot;
    Json::Reader reader;
    Json::StyledWriter writer;
    event_base *base = NULL;
    std::shared_ptr<tce::FaceEngine> faceall;
    evhttp *http = NULL;
    std::shared_ptr<DataCache> cache;
    log4cpp::Category &logger;
    PropertyConfigurator &config;
};

#endif // __HTTP_SERVICE_H__
