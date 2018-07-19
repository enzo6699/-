#include "PropertyConfigurator.h"
#include <fstream>
#include <log4cpp/Category.hh>

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
	port = _properties.getInt("facesearchservice.appender.listen.port", 8100);
	timeout = _properties.getInt("facesearchservice.appender.http.timeout", 5);

	//kafka信息
	kafkabroker = _properties.getString("facesearchservice.kafka.broker", "");
	kafkatopic = _properties.getString("facesearchservice.kafka.topic", "");
	kafkagroupid = _properties.getString("facesearchservice.kafka.group.id", "");

	dbFilename = _properties.getString("facesearchservice.sqlite.dbfilename", "./FACEINFO.db");

    sqliteCachetime = _properties.getInt("facesearchservice.sqlite.cache.days", 30);
    qualityThreshold = _properties.getFloat("facesearchservice.qualityThreshold", 0.5f);
    compareThreshold = _properties.getFloat("facesearchservice.compareThreshold", 0.5f);
	
	log4cpp::Category::getRoot().info(toString().c_str());
}

std::string PropertyConfigurator::toString()
{
	std::stringstream ss;
	ss << "property [port=" << port << ", timeout=" << timeout << ", kafkabroker=" << kafkabroker
        << ", kafkatopic=" << kafkatopic << ", kafkagroupid=" << kafkagroupid << ", sqliteDbFilename=" << dbFilename << "]";
	return ss.str();
}
