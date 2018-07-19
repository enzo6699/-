#ifndef _LOCK_H_
#define _LOCK_H_

#ifdef _MSC_VER
#include <Windows.h>
#endif
#if defined(__GNUC__)
#include <thread>
#endif

namespace tce {
	class Lock
	{
	public:
		Lock();
		virtual~Lock();

	public:
		void lock();
		void unlock();

	private:
#if defined(__GNUC__)
		pthread_rwlock_t m_rw_lock;
#else
		CRITICAL_SECTION m_rw_lock;
#endif
	};
}

#endif