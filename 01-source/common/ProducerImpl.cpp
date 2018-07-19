#include "ProducerImpl.h"
#include "Logger.h"

namespace tce {

	CProducerImpl::CProducerImpl()
	{
	}

	CProducerImpl::~CProducerImpl()
	{
	}

	int CProducerImpl::Create(std::string strBroker, std::string strTopic, bool bManualFlush)
	{
		std::shared_ptr<RdKafka::Conf> conf_;
		std::shared_ptr<RdKafka::Conf> tconf_;
		std::string errstr;

		m_strBroker = strBroker;
		m_strTopic = strTopic;
		m_bManualFlush = bManualFlush;

        log4cpp::Category::getRoot().info("broker:%s; topic:%s", m_strBroker.c_str(), m_strTopic.c_str());

		conf_ = std::shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
		tconf_ = std::shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

		if (conf_.get() == NULL || tconf_.get() == NULL)
		{
			return 1;
		}

		conf_->set("metadata.broker.list", strBroker, errstr);
		conf_->set("producer.type", "async", errstr);
		conf_->set("batch.num.messages", "1", errstr);
		conf_->set("linger.ms", "5", errstr);
		conf_->set("request.required.acks", "0", errstr);

		m_producer = std::shared_ptr<RdKafka::Producer>(RdKafka::Producer::create(conf_.get(), errstr));
		if (m_producer.get() == NULL)
		{
			log4cpp::Category::getRoot().error("Failed to create producer : %s", errstr.c_str());
			return 2;
		}

		//指定topic，topic即为一个消息队列名/唯一标识，topic在服务端通过脚本创建。
		m_tpk = std::shared_ptr<RdKafka::Topic>(RdKafka::Topic::create(m_producer.get(), strTopic, tconf_.get(), errstr));
		if (!m_tpk)
		{
			log4cpp::Category::getRoot().error("Failed to create : %s", errstr.c_str());
			return 3;
		}

		return 0;
	}

	int CProducerImpl::send(const std::string& data)
	{
		std::string key = "testkey";//kafka使用该key串的hash值做负载均衡，实际使用时请传NULL或随机值（例如图片名，图片ID等）。
		RdKafka::ErrorCode resp = m_producer->produce(m_tpk.get(), RdKafka::Topic::PARTITION_UA,
			RdKafka::Producer::RK_MSG_COPY, (void*)data.c_str(), data.size(),
			NULL, NULL);

		if (resp != 0)
			log4cpp::Category::getRoot().error("send data failed %s", RdKafka::err2str(resp).c_str());

		if (m_bManualFlush)
			resp = m_producer->flush(200);//强制立即将数据flush到消息队列，防止数据丢失，如果此时网络断开或者服务器故障，flush会一直卡死知道恢复正常
		//timeout_参数经过测试似乎不起作用，无论设置为多少都会一直卡死。

		if (resp != 0)
			log4cpp::Category::getRoot().error("send data failed %s", RdKafka::err2str(resp).c_str());

		return resp;
	}
}
