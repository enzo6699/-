#ifndef _property_configurator_h_
#define _property_configurator_h_

#include <Properties.h>
#include <log4cpp/Category.hh>

class PropertyConfigurator
{
public:
	static PropertyConfigurator& Instance();
public:
	PropertyConfigurator();
	~PropertyConfigurator();

public:
	void configure(const std::string& initFileName) throw (tce::ConfigureFailure);
public:
    std::string localDirectory;
    std::string nginxAddress;
    std::string registerUrl;

    std::string listenAddress;
    int listenPort = 0;

    int maxTaskNum = 10;

    float compareThreshold = 0.5f;
    float qualityThreshold = 0.5f;
    float alarmQualityThreshold = 0.5f;

    std::string broker;
    std::string consumerTopic;
    std::string consumerGroup;
    std::string producerTopic;

private:
	tce::Properties _properties;
    log4cpp::Category &logger;
};

#endif
