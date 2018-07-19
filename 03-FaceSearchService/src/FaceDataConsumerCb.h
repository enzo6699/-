#ifndef _SEARCH_CONSUMER_CB_H_
#define _SEARCH_CONSUMER_CB_H_

#include "PropertyConfigurator.h"
#include "rdkafkacpp.h"
#include "SqliteFaceCache.h"
#include "TimerThread.h"
#include "common/Lock.h"
#include <log4cpp/Category.hh>

class FaceDataConsumerCb : public RdKafka::ConsumeCb
{
public:
	FaceDataConsumerCb(SqliteFaceCache *pSqliteFaceCache);
	~FaceDataConsumerCb();

private:
	virtual void consume_cb(RdKafka::Message &msg, void *opaque);
	void dealMsg(const char *data, size_t nLen);

private:
	SqliteFaceCache *m_pSqliteFaceCache;

	int m_maxCache; // 批量插入数据最大条数
	Timer m_timer;

	tce::Lock m_lock;

	std::vector<std::shared_ptr<face_info_t>> m_vecFaceInfo;
    log4cpp::Category &logger;
    PropertyConfigurator &config;
};

#endif
