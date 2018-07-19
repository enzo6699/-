#ifndef _BASE_MSG_HANDLER_H_
#define _BASE_MSG_HANDLER_H_

#include "HttpMsg.h"

class BaseMsgHandler
{
public:
	BaseMsgHandler();
	~BaseMsgHandler();

public:
	virtual void dealMsg(http_msg_t* pHttp_msg) = 0;
};



#endif