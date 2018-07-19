#include "StringUtil.h"
#include <iterator>
#include <stdio.h>
#include <stdarg.h>

#if defined(_MSC_VER)
#define VSNPRINTF _vsnprintf
#else
#define VSNPRINTF vsnprintf
#endif

namespace tce {

	std::string StringUtil::vform(const char* format, va_list args) {
		size_t size = 1024;
		char* buffer = new char[size];

		while (1) {
			va_list args_copy;

#if defined(_MSC_VER) || defined(__BORLANDC__)
			args_copy = args;
#else
			va_copy(args_copy, args);
#endif

			int n = VSNPRINTF(buffer, size, format, args_copy);

			va_end(args_copy);

			// If that worked, return a string.
			if ((n > -1) && (static_cast<size_t>(n) < size)) {
				std::string s(buffer);
				delete[] buffer;
				return s;
			}

			// Else try again with more space.
			size = (n > -1) ?
				n + 1 :   // ISO/IEC 9899:1999
				size * 2; // twice the old size

			delete[] buffer;
			buffer = new char[size];
		}
	}

	std::string StringUtil::trim(const std::string& s) {
		static const char* whiteSpace = " \t\r\n";

		// test for null string
		if (s.empty())
			return s;

		// find first non-space character
		std::string::size_type b = s.find_first_not_of(whiteSpace);
		if (b == std::string::npos) // No non-spaces
			return "";

		// find last non-space character
		std::string::size_type e = s.find_last_not_of(whiteSpace);

		// return the remaining characters
		return std::string(s, b, e - b + 1);
	}

	unsigned int StringUtil::split(std::vector<std::string>& v,
		const std::string& s,
		char delimiter, unsigned int maxSegments) {
		v.clear();
		std::back_insert_iterator<std::vector<std::string> > it(v);
		return split(it, s, delimiter, maxSegments);
	}

	float StringUtil::fscanf(float in, const char* format)
	{
		float out = 0.0f;;
		char buf[10] = {0};
		sprintf(buf, format, in);
		sscanf(buf, format, &out);
		return out;
	}
}