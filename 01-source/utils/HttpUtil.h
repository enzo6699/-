#ifndef _HTTP_UTIL_H_
#define _HTTP_UTIL_H_

#include <string>

//#define DISABLE_EXPECT

namespace tce
{
	class HttpUtil
	{
	public:

		static std::string HttpGet(const char* url, long &statusCode, int timeout = 3000);

		static std::string HttpPost(const char* url, const char* data, int dataLength, long &statusCode, int timeout = 3000);

		/*
		* NAME HttpGet()
		*
		* DESCRIPTION 
		*
		* param url input
		* param data input
		* param length data's length
		*
		*/
		static size_t HttpGet(const char* url, char *data, size_t length, int timeout = 3000);

		/*
		* NAME HttpGet2()
		*
		* DESCRIPTION upload file to nginx
		*
		* param url input
		* param data input
		* param length data's length
		*
		*/
		static size_t HttpUpload(const char* url, const char *data, size_t length, int timeout = 3000);
	};
}

#endif