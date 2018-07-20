#include "DataCache.h"
#include "CompareConsumer.h"
#include "PropertyConfigurator.h"
#include "HttpService.h"
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <rdkafkacpp.h>

static struct option longopts[] =
{
    { "help", 0, NULL, 'h' },
    { "version", 0, NULL, 'v' },
    { "conf", 1, NULL, 'c' },
    { "log", 1, NULL, 'l' },
    { NULL, 0, NULL, 0 }
};

const char * help = "--version <print program version and program shutdown>\n"
                    "--conf <configuration file path>\n"
                    "--log <log configuration file path>\n";

void show_help()
{
    fprintf(stderr, help);
}

int main(int argc, char *argv[])
{
	char szConf[255] = { 0 };
	char szLogConf[255] = { 0 };
	int opt = 0;
	int option_index = 0;
	while ((opt = getopt_long(argc, argv, "hvc:l:", longopts, &option_index)) != -1)
	{
		switch (opt)
		{
		case'c':
			memcpy(szConf, optarg, strlen(optarg));
			break;
		case'l':
			memcpy(szLogConf, optarg, strlen(optarg));
			break;
        case'h':
            show_help();
		default:
            return 0;
		}
	}

    if (strlen(szLogConf) == 0) {
        log4cpp::PropertyConfigurator::configure("./conf/log4cpp.property");
    } else {
        log4cpp::PropertyConfigurator::configure(szLogConf);
    }

    PropertyConfigurator &config = PropertyConfigurator::Instance();
    if (strlen(szConf) == 0) {
        config.configure("./conf/compareservice.property");
    } else {
        config.configure(szConf);
    }

    log4cpp::Category::getRoot().info("########## FCompare1N启动成功 ##########");
    log4cpp::Category::getRoot().info("Rdkafka版本: %s", RdKafka::version_str().c_str());

    std::shared_ptr<DataCache> cache(new DataCache());

    std::shared_ptr<CompareConsumer> cc(new CompareConsumer(cache));
    tce::ConsumerImpl consumer;
    consumer.Create(config.broker, config.consumerTopic, config.consumerGroup, cc);
    consumer.Run();

    HttpService service(cache);
    service.start();

    log4cpp::Category::getRoot().info("FCompare1N Stop");
	return 0;
}
