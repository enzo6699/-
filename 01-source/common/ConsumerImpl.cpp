#include "ConsumerImpl.h"
#include <iostream>
#include <string>

#ifndef _MSC_VER
#include <sys/time.h>
#endif

#include"Logger.h"

namespace tce {

	ConsumerImpl::ConsumerImpl()
        : m_stop(false)
		, m_thread(NULL)
		, m_consumer(NULL)
		, m_consumerCb(NULL)
	{
	}

	ConsumerImpl::~ConsumerImpl()
	{
		Destory();
	}

	int ConsumerImpl::Create(const std::string& strBroker, const std::string& strTopic, const std::string& strGroupId, std::shared_ptr<RdKafka::ConsumeCb> consumerCb)
	{
		m_strBroker = strBroker;
		m_strTopic = strTopic;
		m_strGroupId = strGroupId;
		m_consumerCb = consumerCb;
		std::shared_ptr<RdKafka::Conf> conf_;
		std::shared_ptr<RdKafka::Conf> tconf_;
		std::string errstr;

		log4cpp::Category::getRoot().info("broker:%s; topic: %s; group.id:%s", m_strBroker.c_str(), m_strTopic.c_str(), m_strGroupId.c_str());

		conf_ = std::shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
		tconf_ = std::shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

		if (conf_.get() == NULL || tconf_.get() == NULL)
		{
			return 1;
		}

		//kafka 服务器ip 端口，如果是集群模式，为"ip1:port1;ip2:port2;ip3:port3",
		//仅需要输入至少2个服务器地址，系统会自己找出最匹配客户端的服务器并动态切换。
		//topic 即消息队列名。topic在服务端通过脚本创建。
		//consumer客户端支持 同时监听多个组，此属性可以用于处理流程的分支-合并。
		conf_->set("metadata.broker.list", m_strBroker, errstr);
		conf_->set("group.id", m_strGroupId, errstr);
		conf_->set("key.deserializer", "org.apache.kafka.common.serialization.StringDeserializer", errstr);
		conf_->set("value.deserializer", "org.apache.kafka.common.serialization.StringDeserializer", errstr);
		conf_->set("enable.auto.commit", "true", errstr);
		conf_->set("auto.commit.interval.ms", "1000", errstr);
		conf_->set("session.timeout.ms", "30000", errstr);
		conf_->set("max.poll.records", "1", errstr);//每次取到的最大消息数，设置为1可以取得最大的消息处理负载均衡，同时可以防止数据丢失
		conf_->set("default_topic_conf", tconf_.get(), errstr);

		m_consumer = std::shared_ptr<RdKafka::KafkaConsumer>(RdKafka::KafkaConsumer::create(conf_.get(), errstr));
		if (m_consumer.get() == NULL) {
			log4cpp::Category::getRoot().error("Failed to create consumer: %s", errstr.c_str());
			return 2;
		}

		std::vector<std::string> vec_topics;
		vec_topics.push_back(m_strTopic);
		RdKafka::ErrorCode err = m_consumer->subscribe(vec_topics);
		if (err != 0)
		{
			log4cpp::Category::getRoot().error("Failed to subscribe consumer: %s", RdKafka::err2str(err).c_str());
			return 3;
		}

		return 0;
	}

	void ConsumerImpl::Destory()
	{
		m_stop = true;
		if (m_thread.get() != NULL)
		{
			m_thread->join();
			m_thread = NULL;
		}

		if (m_consumer.get() != NULL)
		{
			m_consumer->close();
			m_consumer = NULL;
		}
		RdKafka::wait_destroyed(3000);
	}

	int ConsumerImpl::Run()
	{
		m_stop = false;
		m_thread = std::shared_ptr<std::thread>(new std::thread(&ConsumerImpl::recv_msg, this));
		return 0;
	}

	void ConsumerImpl::recv_msg()
	{
		while (!m_stop) {
			RdKafka::Message *msg = m_consumer->consume(100);
			if (msg != NULL)
			{
				m_consumerCb->consume_cb(*msg, NULL);
				delete msg;
			}
		}
	}
}
