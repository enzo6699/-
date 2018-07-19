//
//Log.h Log接口定义
//
#ifndef _VideoDataStructuring_Include_Common_Log_
#define _VideoDataStructuring_Include_Common_Log_

struct Log
{
	enum LogLevel
	{
		LOG_LEVEL_DEBUG = 0,
		LOG_LEVEL_INFO,
		LOG_LEVEL_WARN,
		LOG_LEVEL_ERROR,
		LOG_LEVEL_TOLOGSYS,
	};

	virtual void OnLog(const char *log, LogLevel level) = 0;
};
#endif