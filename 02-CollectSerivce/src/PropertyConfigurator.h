#ifndef _property_configurator_h_
#define _property_configurator_h_

#include "common/Properties.h"

#define PERSON_PHOTO_SAVEDIR "person/photo/"
#define PERSON_FEATURE_SAVEDIR "person/feature/"
#define ORIGINAL_PHOTO_SAVEDIR  "originalphoto/"
#define OBJECT_PHOTO_SAVEDIR "objectphoto/"

#define MAX_IMAGE_SIZE 1024*1024*4

#define MAX_CACHE_NUMBER_DATA 100
#define MAX_VIDEOFRAME_CACHE_SIZE 8

#define MAX_FACE 7

#define DEFAULT_CACHETIME 5000

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
    int port = 0;
    std::string address;
	std::string metadatapath;

    // timeout in seconds for http request
    int timeout = 10;

	//任务调度服务地址
	std::string ltsServerUrl;
	std::string appId;
	std::string instanceId;
	int capacity;
    int longPollingInterval = 60;
    int heartbeatInterval = 60;

	//kafka相关信息
	std::string kafkabroker;
	std::string kafkatopic;

    std::string nginxAddress;  

    float qualityThreshold = 0.5f;
    int minFaceWidth = 80;
    int minFaceHeight = 80;

private:
	tce::Properties _properties;
};


#endif
