#ifndef _PRODUCER_IMPL_H_
#define _PRODUCER_IMPL_H_

#include <iostream>
#include <memory>
#include "rdkafkacpp.h"

namespace tce {


	class CProducerImpl
	{
	public:
		CProducerImpl();
		~CProducerImpl();

	public:
		int Create(std::string strBroker, std::string strTopic, bool bManualFlush = true);
		int send(const std::string& data);
	private:
		std::string m_strBroker;
		std::string m_strTopic;
		bool m_bManualFlush;

		std::shared_ptr<RdKafka::Producer> m_producer;
		std::shared_ptr<RdKafka::Topic> m_tpk;
	};
}

#endif