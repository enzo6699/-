#ifndef _PRODUCER_MGR_H_
#define _PRODUCER_MGR_H_

#include <memory>
#include <map>
#include "ProducerImpl.h"

namespace tce {

	typedef std::map<std::string, std::shared_ptr<CProducerImpl>>::iterator ProducerIter;

	class CProducerMgr
	{
	public:
		static CProducerMgr& Instance();
	public:
		CProducerMgr();
		~CProducerMgr();

	public:
		int AddProducer(const std::string& strTopic, const std::shared_ptr<CProducerImpl> producer);
		int RemoveProducer(const std::string& strTopic);
		std::shared_ptr<CProducerImpl> GetProducer(const std::string& strTopic);

	private:
		std::map<std::string, std::shared_ptr<CProducerImpl>> m_producerMap;
	};
}

#endif