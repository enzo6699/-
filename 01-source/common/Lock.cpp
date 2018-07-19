#include "Lock.h"

namespace tce {
	Lock::Lock()
	{
#if defined(__GNUC__)
		pthread_rwlock_init(&m_rw_lock, NULL);
#else
		InitializeCriticalSection(&m_rw_lock);
#endif // __GNUC__
	}

	Lock::~Lock()
	{
#if defined(__GNUC__)
		pthread_rwlock_destroy(&m_rw_lock);
#else
		DeleteCriticalSection(&m_rw_lock);
#endif // linux
	}

	void Lock::lock()
	{
#if defined(__GNUC__)
		pthread_rwlock_wrlock(&m_rw_lock);
#else
		EnterCriticalSection(&m_rw_lock);
#endif
	}

	void Lock::unlock()
	{
#if defined(__GNUC__)
		pthread_rwlock_unlock(&m_rw_lock);
#else
		LeaveCriticalSection(&m_rw_lock);
#endif
	}
}