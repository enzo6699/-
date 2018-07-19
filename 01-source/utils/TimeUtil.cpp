#include "TimeUtil.h"
#include <stdio.h>
#if defined(_MSC_VER)
#include <time.h>
#include <windows.h>
#else
#include <sys/time.h> 
#endif

namespace tce
{
#if defined(_MSC_VER)
	static int gettimeofday(struct timeval *tp, void *tzp)
	{
		time_t clock;
		struct tm tm;
		SYSTEMTIME wtm;
		GetLocalTime(&wtm);
		tm.tm_year = wtm.wYear - 1900;
		tm.tm_mon = wtm.wMonth - 1;
		tm.tm_mday = wtm.wDay;
		tm.tm_hour = wtm.wHour;
		tm.tm_min = wtm.wMinute;
		tm.tm_sec = wtm.wSecond;
		tm.tm_isdst = -1;
		clock = mktime(&tm);
		tp->tv_sec = (long long)clock;
		tp->tv_usec = (long long)wtm.wMilliseconds * 1000;
		return (0);
	}
#endif
    int64_t TimeUtil::StringToTimestamp(const char *date, const char *format)
	{
		tm tm_;
		int year, month, day, hour, minute, second;
		int nRet = sscanf(date, format, &year, &month, &day, &hour, &minute, &second);
		if (nRet == 0)
			return 0;
		tm_.tm_year = year - 1900;
		tm_.tm_mon = month - 1;
		tm_.tm_mday = day;
		tm_.tm_hour = hour;
		tm_.tm_min = minute;
		tm_.tm_sec = second;
		tm_.tm_isdst = 0;

		return mktime(&tm_);
	}

    int64_t TimeUtil::GetTimestamp()
	{
		static timeval tv = { 0 };
		gettimeofday(&tv, NULL);
		return (long long)tv.tv_sec * 1000 + (long long)tv.tv_usec / 1000;
	}

	int64_t TimeUtil::GetTimestamp_s()
	{
		static timeval tv = { 0 };
		gettimeofday(&tv, NULL);
		return (long long)tv.tv_sec;
	}

	std::string TimeUtil::GetTimeStr()
	{
#ifdef _MSC_VER
		SYSTEMTIME wtm;
		GetLocalTime(&wtm);
		char szTime[22] = { 0 };
		sprintf(szTime, "%04d%02d%02d%02d%02d%02d%03d", wtm.wYear, wtm.wMonth, wtm.wDay, wtm.wHour, wtm.wMinute, wtm.wSecond, wtm.wMilliseconds);
		return szTime;
#else
        time_t timestamp = time(NULL);
        struct tm *lt = localtime(&timestamp);
        char szTime[22] = { 0 };
        strftime(szTime, sizeof(szTime), "%Y%m%d%H%M%S", lt);

        struct timeval tv = { 0 };
        gettimeofday(&tv, NULL);
        sprintf(szTime, "%s%03d", szTime, tv.tv_usec / 1000);

        return szTime;
#endif
	}

	std::string TimeUtil::GetDateStr()
	{
		time_t now = time(0);
		tm *ltm = localtime(&now);
		char szTime[50] = { 0 };
		sprintf(szTime, "%4d/%02d/%02d/", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
		return std::string(szTime);
	}

	std::string TimeUtil::TimestampToStr(long long timestamp)
	{
#ifdef _MSC_VER
		time_t tick = (time_t)timestamp / 1000;
		struct tm tm;
		tm = *localtime(&tick);
		SYSTEMTIME wtm = { 1900 + tm.tm_year,
			1 + tm.tm_mon,
			tm.tm_wday,
			tm.tm_mday,
			tm.tm_hour,
			tm.tm_min,
			tm.tm_sec,
			timestamp%1000 };
		char szTime[22] = { 0 };
		sprintf(szTime, "%04d%02d%02d%02d%02d%02d%03d", wtm.wYear, wtm.wMonth, wtm.wDay, wtm.wHour, wtm.wMinute, wtm.wSecond, wtm.wMilliseconds);
		return szTime;
#else
        time_t tick = (time_t)(timestamp / 1000);
        struct tm *tm = localtime(&tick);
        char szTime[22] = { 0 };
        strftime(szTime, sizeof(szTime), "%Y%m%d%H%M%S", tm);

        struct timeval tv = { 0 };
        gettimeofday(&tv, NULL);
        sprintf(szTime, "%s%03d", szTime, tv.tv_usec / 1000);

        return szTime;
#endif
	}
}
