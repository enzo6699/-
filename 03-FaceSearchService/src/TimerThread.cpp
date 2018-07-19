#include "TimerThread.h"
#include "utils/TimeUtil.h"


TimerThread* TimerThread::m_instance;

TimerThread::TimerThread()
: m_bstop(true)
, m_bstart(false)
{
}

TimerThread::~TimerThread()
{
	m_bstop = true;
	if (m_thread.get() != NULL)
		m_thread->join();
	m_thread = NULL;
}

void TimerThread::Start()
{
	m_bstop = false;
	if (!m_bstart)
	{
		m_bstart = true;
		m_thread = std::shared_ptr<std::thread>(new std::thread(&TimerThread::run, this));
	}
}

void TimerThread::run(void)
{
	int lastElapsedTime = 1;
	while (true)
	{
		int64_t start_clock = tce::TimeUtil::GetTimestamp();
		this->m_timer_list.sort();
		std::list<Timer>::iterator iter;
		for (iter = this->m_timer_list.begin();
			iter != this->m_timer_list.end();
			iter++)
		{
			iter->leftsecs -= lastElapsedTime;
			if (iter->leftsecs <= 0)
			{
				iter->_callback(iter->m_args);
				iter->leftsecs = iter->m_interval;
			}
		}

		int64_t end_clock = tce::TimeUtil::GetTimestamp();

		std::this_thread::sleep_for(std::chrono::milliseconds(start_clock - end_clock + 200));

		end_clock = tce::TimeUtil::GetTimestamp();
		lastElapsedTime = end_clock - start_clock;
	}
	return;
}

void TimerThread::Register(Timer &timer)
{
	this->m_timer_list.push_back(timer);
}

void TimerThread::unRegister(Timer &timer)
{
	this->m_timer_list.remove(timer);
}

TimerThread* TimerThread::get_instance()
{
	if (m_instance == NULL)
	{
		m_instance = new TimerThread();
	}
	return m_instance;
}