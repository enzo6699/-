#include "PropertyConfigurator.h"
#include "./../utils/GuidUtil.h"
#include <fstream>
#include <log4cpp/Category.hh>

PropertyConfigurator& PropertyConfigurator::Instance()
{
	static  PropertyConfigurator instance;
	return instance;
}

PropertyConfigurator::PropertyConfigurator() : logger(log4cpp::Category::getRoot())
{
}

PropertyConfigurator::~PropertyConfigurator()
{
}

void PropertyConfigurator::configure(const std::string& initFileName) throw (tce::ConfigureFailure)
{
    logger.info("正在读取配置文件 ...");

	_properties.load(initFileName);

    broker = _properties.getString("kafka.broker", "");
    consumerTopic = _properties.getString("kafka.consumer.topic", "");
    consumerGroup = _properties.getString("kafka.consumer.group", "");
    producerTopic = _properties.getString("kafka.producer.topic", "");

    localDirectory = _properties.getString("local.directory", "");

    nginxAddress = _properties.getString("nginx.address", "");
    registerUrl = _properties.getString("register.url", "");
    listenAddress = _properties.getString("listen.address", "");
    listenPort = _properties.getInt("listen.port", 0);

    compareThreshold = _properties.getFloat("compareThreshold", 0.5f);
    qualityThreshold = _properties.getFloat("qualityThreshold", 0.5f);
    alarmQualityThreshold = _properties.getFloat("alarmQualityThreshold", 0.5f);

    if (broker.length() == 0)
        throw tce::ConfigureFailure("kafka.broker does not exist");
    if (consumerTopic.length() == 0)
        throw tce::ConfigureFailure("kafka.consumer.topic does not exist");
    if (consumerGroup.length() == 0)
        throw tce::ConfigureFailure("kafka.consumer.group does not exist");
    if (producerTopic.length() == 0)
        throw tce::ConfigureFailure("kafka.producer.topic does not exist");

    logger.info("kafka地址: %s", broker.c_str());
    logger.info("kafka接收比对任务的topic: %s", consumerTopic.c_str());
    logger.info("kafka接收比对任务的group: %s", consumerGroup.c_str());
    logger.info("kafka发送比对结果的topic: %s", producerTopic.c_str());

    logger.info("抓拍图片保存路径: %s", localDirectory.c_str());
    logger.info("Nginx地址: %s", nginxAddress.c_str());

    logger.info("比对阈值: %.2f", compareThreshold);
}
