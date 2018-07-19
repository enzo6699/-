#include "PropertyConfigurator.h"
#include <fstream>
#include "utils/GuidUtil.h"

PropertyConfigurator& PropertyConfigurator::Instance()
{
	static  PropertyConfigurator instance;
	return instance;
}

PropertyConfigurator::PropertyConfigurator()
{
}

PropertyConfigurator::~PropertyConfigurator()
{
}

void PropertyConfigurator::configure(const std::string& initFileName) throw (tce::ConfigureFailure)
{
	_properties.load(initFileName);

	//读取配置文件
    port = _properties.getInt("listen.port", 8100);
    address = _properties.getString("listen.address", "");
    metadatapath = _properties.getString("metadatapath", "");
    timeout = _properties.getInt("http.timeout", 10);

	//节点注册信息
	ltsServerUrl = _properties.getString("lts.server.url", "");
	appId = _properties.getString("application.name", "");

	instanceId = _properties.getString("application.instanceId", "");
	if (instanceId.length() == 0)
	{
		char szInstanceId[255] = { 0 };
		sprintf(szInstanceId, "%s:%s", appId.c_str(), tce::GuidUtil::gguid().c_str());
		instanceId = szInstanceId;

		_properties.insert(std::pair<std::string, std::string>("application.instanceId", instanceId));
	}

    qualityThreshold = _properties.getFloat("qualityThreshold", 0.5f);
    minFaceWidth = _properties.getInt("min.face.width", 80);
    minFaceHeight = _properties.getInt("min.face.height", 80);

	capacity = _properties.getInt("lts.capacity", 1);
	longPollingInterval = _properties.getInt("lts.longpolingInterval", 60);
	heartbeatInterval = _properties.getInt("lts.heartbeatInterval", 60);

	//kafka信息
    kafkabroker = _properties.getString("kafka.broker", "");
    kafkatopic = _properties.getString("kafka.topic", "");

    nginxAddress = _properties.getString("nginx.address", "");
    if (nginxAddress[nginxAddress.size() - 1] != '/') {
        nginxAddress.push_back('/');
    }
   
	if (metadatapath.length() == 0)
        throw tce::ConfigureFailure("metadatapath does not exist");

	if (appId.length() == 0)
        throw tce::ConfigureFailure("application.name does not exist");

	std::ofstream initFile(initFileName);
	_properties.save(initFile);
}
