#ifndef _TIMER_THREAD_H_
#define _TIMER_THREAD_H_

#include <thread>
#include <list>
#include <memory>
#include <atomic>

struct Timer
{

	void *m_args;
	int(*_callback)(void *args);
	int m_interval;
	int leftsecs;
	void open(int interval, int(*callback)(void *args), void *args = NULL)
	{
		m_interval = interval * 1000;
		leftsecs = m_interval;
		_callback = callback;
		m_args = args;
	}

	bool operator < (Timer timer)
	{
		return timer.leftsecs < this->leftsecs;
	}

	bool operator == (Timer timer)
	{
		return timer.leftsecs == this->leftsecs;
	}

};

class TimerThread
{

public:
	TimerThread();
	virtual ~TimerThread();

public:
	static TimerThread* m_instance;
	static TimerThread* get_instance();

	void Start();

	void Register(Timer &timer);
	void unRegister(Timer &timer);
protected:
	void run(void);
private:
	std::list<Timer> m_timer_list;
	std::shared_ptr<std::thread> m_thread;
	std::atomic<bool> m_bstop;
	std::atomic<bool> m_bstart;
};

#define TIMERMANAGE TimerThread::get_instance()

#endif