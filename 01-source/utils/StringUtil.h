#ifndef _STRING_UTIL_H_
#define _STRING_UTIL_H_

#include <string.h>
#include <iostream>
#include <vector>


#ifdef __GNUC__
#ifndef INT_MAX
#define INT_MAX       2147483647    /* maximum (signed) int value */
#endif
#endif

namespace tce
{
	class StringUtil
	{
	public:
		static std::string vform(const char* format, va_list args);

		static std::string trim(const std::string& s);

		static float fscanf(float in, const char* format);

		static unsigned int split(std::vector<std::string>& v,
			const std::string& s, char delimiter,
			unsigned int maxSegments = INT_MAX);

		template<typename T> static unsigned int split(T& output,
			const std::string& s, char delimiter,
			unsigned int maxSegments = INT_MAX) {
			std::string::size_type left = 0;
			unsigned int i;
			for (i = 1; i < maxSegments; i++) {
				std::string::size_type right = s.find(delimiter, left);
				if (right == std::string::npos) {
					break;
				}
				*output++ = s.substr(left, right - left);
				left = right + 1;
			}

			*output++ = s.substr(left);
			return i;
		}

	};
}

#endif