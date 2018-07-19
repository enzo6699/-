#include "ProducerMgr.h"

namespace tce {
	CProducerMgr& CProducerMgr::Instance()
	{
		static CProducerMgr pmgr;
		return pmgr;
	}

	CProducerMgr::CProducerMgr()
	{
	}

	CProducerMgr::~CProducerMgr()
	{
	}

	int CProducerMgr::AddProducer(const std::string& strTopic, std::shared_ptr<CProducerImpl> producer)
	{
		m_producerMap.insert(std::pair<std::string, std::shared_ptr<CProducerImpl>>(strTopic, producer));
		return 0;
	}

	int CProducerMgr::RemoveProducer(const std::string& strTopic)
	{
		m_producerMap.erase(strTopic);
		return 0;
	}

	std::shared_ptr<CProducerImpl> CProducerMgr::GetProducer(const std::string& strTopic)
	{
		std::shared_ptr<CProducerImpl> producerImpl;
		ProducerIter iter = m_producerMap.find(strTopic);
		if (iter != m_producerMap.end())
			return iter->second;

		return producerImpl;
	}
}