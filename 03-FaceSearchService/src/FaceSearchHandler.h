#ifndef _FACE_SEARCH_HANDLER_H_
#define _FACE_SEARCH_HANDLER_H_

#include "BaseMsgHandler.h"
#include "FaceIdentify.h"

class FaceSearchHandler : public BaseMsgHandler
{
public:
	FaceSearchHandler();
	~FaceSearchHandler();

private:
	virtual void dealMsg(http_msg_t* pHttp_msg);
};


#endif