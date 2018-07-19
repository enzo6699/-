// fcamera.cpp : 定义控制台应用程序的入口点。
//

#ifdef _MSC_VER
#include "common/GetoptWin.h"
#else
#include <getopt.h>
#include <unistd.h>
#endif
#include <signal.h>
#include "config.h"
#include "PropertyConfigurator.h"
#include "CameraTask.h"
#include "common/Logger.h"
#include "TaskMgr.h"
#include "LtsJobClient.h"

static struct option longopts[] =
{
	{ "help", 0, NULL, 'h' },
	{ "version", 0, NULL, 'v' },
	{ "conf", 1, NULL, 'c' },
	{ "log", 1, NULL, 'l' },
	{ NULL, 0, NULL, 0 }
};

const char * help = "--version <print program version and program shutdown>\n" \
"--conf <configuration file path>\n" \
"--log <log configuration file path>\n";

void show_help() {
	fprintf(stderr, help);
}

void signal_handler(int sig) {
	log4cpp::Category::getRoot().info("########signal_handler##########");
	switch (sig) {
	case SIGTERM:
	case SIGINT:
		break;
	}
}

int main(int argc, char* argv[])
{
	char szConf[255] = { 0 };
	char szLogConf[255] = { 0 };
	int opt = 0;
	int option_index = 0;
	while ((opt = getopt_long(argc, argv, "hvc:l:p:", longopts, &option_index)) != -1)
	{
		switch (opt)
		{
		case'h':
			show_help();
			exit(0);
		case'v':
			printf("Version %s_%d.%d.%d.%d\n",
				Demo_VERSION_APPNAME,
				Demo_VERSION_MAJOR,
				Demo_VERSION_MINOR,
				Demo_VERSION_MICRO,
				Demo_VERSION_REPOS);
			exit(0);
		case'c':
			memcpy(szConf, optarg, strlen(optarg));
			break;
		case'l':
			memcpy(szLogConf, optarg, strlen(optarg));
			break;
		default:
			break;
		}
	}

	// 加载log4日志模块
	try
	{
		if (strlen(szLogConf) == 0)
			log4cpp::PropertyConfigurator::configure("./conf/log4cpp.property");
		else
			log4cpp::PropertyConfigurator::configure(szLogConf);
	}
	catch (log4cpp::ConfigureFailure &ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}

	log4cpp::Category &logger = log4cpp::Category::getRoot();

	// 加载配置文件模块
	try
	{
		if (strlen(szConf) == 0)
			PropertyConfigurator::Instance().configure("./conf/collectservice.property");
		else
			PropertyConfigurator::Instance().configure(szConf);
	}
	catch (tce::ConfigureFailure ex)
	{
		logger.error(ex.what());
		return 2;
	}

	logger.info("**********************************");
	logger.info("**                              **");
	logger.info("**       程序启动完成           **");
	logger.info("**       listen port:%d         **", PropertyConfigurator::Instance().port);
	logger.info("**                              **");
	logger.info("**********************************");

    TaskListener listener;
    listener.start();
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

end:
	logger.info("**********************************");
	logger.info("**                              **");
	logger.info("**          程序停止成功        **");
	logger.info("**                              **");
	logger.info("**********************************");

	return 0;
}
