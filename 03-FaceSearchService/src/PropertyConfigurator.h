#ifndef _property_configurator_h_
#define _property_configurator_h_

#include "common/Properties.h"

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
	//读取配置文件
    int port = 0;
	//http处理超时时间
	int timeout;

	//kafka相关信息
	std::string kafkabroker;
	std::string kafkatopic;
	std::string kafkagroupid;

	//sqlite相关配置
    std::string dbFilename;

	int sqliteCachetime;
    float compareThreshold = 0.5f;
    float qualityThreshold = 0.5f;

private:
	tce::Properties _properties;

	std::string toString();
};


#endif
