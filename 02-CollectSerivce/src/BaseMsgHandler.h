#ifndef _BASE_MSG_HANDLER_H_
#define _BASE_MSG_HANDLER_H_

#include "HttpMsg.h"
#include "TaskMgr.h"
#include "PropertyConfigurator.h"
#include <log4cpp/Category.hh>

class BaseMsgHandler
{
public:
	BaseMsgHandler();
	~BaseMsgHandler();

public:
	virtual void dealMsg(http_msg_t* pHttp_msg) = 0;

protected:
    log4cpp::Category &logger;
    TaskMgr &taskMgr;
    PropertyConfigurator &config;
};



#endif
