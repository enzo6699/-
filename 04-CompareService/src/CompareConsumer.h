#ifndef __COMPARECONSUMER_H__
#define __COMPARECONSUMER_H__

#include "DataCache.h"
#include <string.h>
#include <time.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <ProducerImpl.h>
#include <opencv2/core/core.hpp>
#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>
#include <ConsumerImpl.h>
#include <log4cpp/Category.hh>
#include "PropertyConfigurator.h"

class CompareConsumer : public RdKafka::ConsumeCb
{
public:
    CompareConsumer(std::shared_ptr<DataCache> cache);
    virtual void consume_cb(RdKafka::Message &message, void *opaque);

private:
    void handleMessage(const std::string &message);

    std::shared_ptr<DataCache> cache;
    log4cpp::Category &logger;
    PropertyConfigurator &config;
    tce::CProducerImpl producer;
    Json::Value wroot;
    Json::Value rroot;
    Json::Reader reader;
    Json::StyledWriter writer;
};

#endif // __COMPARECONSUMER_H__
