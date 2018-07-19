#ifndef _CONSUMER_IMPL_H_
#define _CONSUMER_IMPL_H_

#include <memory>
#include <atomic>
#include <thread>
#include "rdkafkacpp.h"

namespace tce {
	class ConsumerImpl
	{
	public:
		ConsumerImpl();
		~ConsumerImpl();

	public:
		int Create(const std::string& strBroker, const std::string& strTopic, const std::string& strGroupId, std::shared_ptr<RdKafka::ConsumeCb> consumerCb);
		void Destory();

		int Run();
	public:
		void recv_msg();
	private:
		std::string m_strBroker;
		std::string m_strTopic;
		std::string m_strGroupId;
		std::shared_ptr<RdKafka::ConsumeCb> m_consumerCb;
		std::shared_ptr<RdKafka::KafkaConsumer> m_consumer;

		std::shared_ptr<std::thread> m_thread;
		std::atomic<bool> m_stop;

	};
}

#endif