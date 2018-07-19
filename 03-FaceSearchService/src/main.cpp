// FaceSearchService.cpp : 定义控制台应用程序的入口点。
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
#include "HttpServer.h"
#include "common/ConsumerImpl.h"
#include "utils/FileUtil.h"
#include "utils/ConvertCharCode.h"
#include "FaceSearchHandler.h"
#include "FaceDataConsumerCb.h"
#include "SqliteFaceCache.h"
#include "TimerThread.h"
#include <log4cpp/PropertyConfigurator.hh>

static struct option longopts[] =
{
	{ "help", 0, NULL, 'h' },
	{ "version", 0, NULL, 'v' },
	{ "conf", 1, NULL, 'c' },
	{ "log", 1, NULL, 'l' },
	{ "port", 1, NULL, 'p' },
	{ NULL, 0, NULL, 0 }
};

const char * help = "--version <print program version and program shutdown>\n" \
"--conf <configuration file path>\n" \
"--log <log configuration file path>\n" \
"--log <port server listen port>\n";

void show_help() {
	fprintf(stderr, help);
}

HttpServer http_server;
static SqliteFaceCache *sqliteFaceCache = NULL;
Timer deleteOverdue_timer;

static void signal_handler(int sig) {
	log4cpp::Category::getRoot().info("########signal_handler##########");
	switch (sig) {
	case SIGTERM:
	case SIGINT:
		http_server.StopHttpServer();  //终止侦听event_dispatch()的事件侦听循环，执行之后的代码
		break;
	}
}

static int initSqlite()
{
    int nRet = sqliteFaceCache->init(PropertyConfigurator::Instance().dbFilename.c_str());
	if (nRet != 0)
	{
		log4cpp::Category::getRoot().error("sqlite初始化失败，错误原因nRet：%d", nRet);
		return 1;
	}

	return 0;
}

static int startHttpService()
{
	//启动程序，实现http服务
	int nRet = http_server.StartHttpServer(PropertyConfigurator::Instance().timeout);
	if (nRet != 0)
	{
		return nRet;
	}

	// 启动服务
	FaceSearchHandler fFaceSearchHandler;
	http_server.SetResource("/facesearchservice/search", &fFaceSearchHandler);

	http_server.Run();

	return nRet;
}

static void threadSqliteInsert()
{
	for (int i = 0; i < 1000; i++)
	{
		FaceIdentify::Instance().TestSqlite();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

static void threadSqliteQuery()
{
	for (int i = 0; i < 1000; i++)
	{
		FaceIdentify::Instance().TestSqliteQuery();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

static int test2()
{
	std::thread mythread1(threadSqliteInsert);
	std::thread mythread2(threadSqliteQuery);
	mythread2.join();
	mythread1.join();
}

int main(int argc, char* argv[])
{
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
	int nRet = 0;
	char szConf[255] = { 0 };
	char szLogConf[255] = { 0 };
	int opt = 0;
	int option_index = 0;
	tce::ConsumerImpl *pConsumerImpl = new tce::ConsumerImpl();

    while ((opt = getopt_long(argc, argv, "hvc:l:", longopts, &option_index)) != -1)
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
		log4cpp::Category::getRoot().error(ex.what());
		return 0;
	}

	// 加载配置文件模块
	try
	{
		if (strlen(szConf) == 0)
			PropertyConfigurator::Instance().configure("./conf/facesearchservice.property");
		else
			PropertyConfigurator::Instance().configure(szConf);
	}
	catch (tce::ConfigureFailure ex)
	{
		log4cpp::Category::getRoot().error(ex.what());
		return 0;
	}

	log4cpp::Category::getRoot().info("**********************************");
	log4cpp::Category::getRoot().info("**                              **");
    log4cpp::Category::getRoot().info("**       程序启动              **");
    log4cpp::Category::getRoot().info("**       listen port: %d  **", PropertyConfigurator::Instance().port);
	log4cpp::Category::getRoot().info("**                              **");
	log4cpp::Category::getRoot().info("**********************************");

    sqliteFaceCache = new SqliteFaceCache;

	// 启动sqlite本地数据库
	if (initSqlite() != 0)
	{
		goto end;
	}

	// 启动识别服务
    if (FaceIdentify::Instance().Init(sqliteFaceCache) != 0)
	{
		goto end;
	}

    {
        std::shared_ptr<FaceDataConsumerCb> faceDataConsumerCb(new FaceDataConsumerCb(sqliteFaceCache));
        if (pConsumerImpl->Create(PropertyConfigurator::Instance().kafkabroker, PropertyConfigurator::Instance().kafkatopic, PropertyConfigurator::Instance().kafkagroupid, faceDataConsumerCb) == 0)
        {
            pConsumerImpl->Run();
        }
        else
        {
            log4cpp::Category::getRoot().error("start service failed!");
            goto end;
        }
    }

	log4cpp::Category::getRoot().info("**********************************");
	log4cpp::Category::getRoot().info("**                              **");
	log4cpp::Category::getRoot().info("**       程序启动完成**");
	log4cpp::Category::getRoot().info("**       listen port:%d         **", PropertyConfigurator::Instance().port);
	log4cpp::Category::getRoot().info("**                              **");
	log4cpp::Category::getRoot().info("**********************************");

	// 启动http服务
	if ((nRet = startHttpService()) != 0)
		log4cpp::Category::getRoot().error("http 服务初始化失败，错误原因nRet:%d", nRet);

end:
    delete sqliteFaceCache;
	log4cpp::Category::getRoot().info("**********************************");
	log4cpp::Category::getRoot().info("**                              **");
	log4cpp::Category::getRoot().info("**          程序停止成功**");
	log4cpp::Category::getRoot().info("**                              **");
	log4cpp::Category::getRoot().info("**********************************");

	return 0;
}
