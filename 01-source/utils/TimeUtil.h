#ifndef _TIME_UTIL_H_
#define _TIME_UTIL_H_

#include <stdint.h>
#include <string>

namespace tce
{
	class TimeUtil
	{
	public:

		/*
		* timestamp 单位s
		*/
        static int64_t StringToTimestamp(const char *date, const char *format);

		/*
		* timestamp 单位ms
		*/
        static int64_t GetTimestamp();

		/*
		* timestamp 单位s
		*/
		static int64_t GetTimestamp_s();

		/*
		* %04d%02d%02d%02d%02d%02d%03d
		*/
		static std::string GetTimeStr();

		/*
		* %4d/%02d/%02d/
		*/
		static std::string GetDateStr();

		/*
		* timestamp 单位ms
		*/
		static std::string TimestampToStr(long long timestamp);
	};
}



#endif
