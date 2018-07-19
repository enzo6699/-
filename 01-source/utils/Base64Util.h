#ifndef _BASE64_UTIL_H_
#define _BASE64_UTIL_H_

#include <iostream>

namespace tce {

	class Base64Util
	{
	public:
		static std::string base64_encode(char const* bytes_to_encode, unsigned int len);
		static std::string base64_decode(std::string const& encoded_string);
	};
}

#endif