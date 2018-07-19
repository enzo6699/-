#include "HttpUtil.h"
#include "common/Logger.h" 
#include "curl/curl.h"
#include <string.h>

namespace tce
{
	struct MemoryStruct{
		MemoryStruct()
		{
			memory = NULL;
			size = 0;
			memorySize = 0;
		}
		char *memory;
		size_t size;
		size_t memorySize;
	};

	struct WriteThis {
		const char *readptr;
		size_t sizeleft;
	};

	static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp)
	{
		struct WriteThis *wt = (struct WriteThis *)userp;
		size_t buffer_size = size*nmemb;

		if (wt->sizeleft) {
			/* copy as much as possible from the source to the destination */
			size_t copy_this_much = wt->sizeleft;
			if (copy_this_much > buffer_size)
				copy_this_much = buffer_size;
			memcpy(dest, wt->readptr, copy_this_much);

			wt->readptr += copy_this_much;
			wt->sizeleft -= copy_this_much;
			return copy_this_much; /* we copied this many bytes */
		}

		return 0; /* no more data left to deliver */
	}

	static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
	{
		size_t realsize = size * nmemb;
		struct MemoryStruct *mem = (struct MemoryStruct *)userp;

		mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
		if (mem->memory == NULL) {
			/* out of memory! */
			log4cpp::Category::getRoot().info("not enough memory (realloc returned NULL)");
			return 0;
		}

		memcpy(&(mem->memory[mem->size]), contents, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;

		return realsize;
	}

	static size_t write_callback2(void *contents, size_t size, size_t nmemb, void *userp)
	{
		size_t realsize = size*nmemb;
		struct MemoryStruct *mem = (struct MemoryStruct *)userp;
		if (mem->memory == NULL)
		{
			log4cpp::Category::getRoot().info("out of memory");
			return 0;
		}
		if ((mem->size + realsize) < mem->memorySize){
			memcpy(&(mem->memory[mem->size]), contents, realsize);
			mem->size += realsize;
			return realsize;
		}
		else
		{
			return mem->size;
		}
	}

	size_t HttpUtil::HttpGet(const char* url, char *data, size_t length, int timeout)
	{
		CURL *curl_handle;
		CURLcode res;
		long retcode = 0;
		int BuffLength;
		struct MemoryStruct chunk;
		chunk.memory = data;
		chunk.memorySize = length;
		chunk.size = 0;

		log4cpp::Category::getRoot().debug("upload file %s", url);

		curl_global_init(CURL_GLOBAL_ALL);
		curl_handle = curl_easy_init();
		curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, timeout);
		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback2);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		res = curl_easy_perform(curl_handle);
		if (res != CURLE_OK) {
			log4cpp::Category::getRoot().error("curl_easy_perform() failed: %s;\n", curl_easy_strerror(res));
		}
		else {
			log4cpp::Category::getRoot().debug("%lu bytes retrieved\n", (int)chunk.size);
		}
		BuffLength = (int)chunk.size;
		res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &retcode);
		curl_easy_cleanup(curl_handle);
		curl_global_cleanup();

		return BuffLength;
	}

	std::string HttpUtil::HttpGet(const char* url, long &statusCode, int timeout)
	{
		CURL *curl_handle;
		CURLcode res;
		std::string result;
		struct MemoryStruct chunk;

		log4cpp::Category::getRoot().debug("upload file %s", url);

		curl_global_init(CURL_GLOBAL_ALL);
		curl_handle = curl_easy_init();
		curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, timeout);
		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		res = curl_easy_perform(curl_handle);
		if (res != CURLE_OK) {
			res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &statusCode);
			log4cpp::Category::getRoot().error("curl_easy_perform() failed: %s; retcode :%d\n", curl_easy_strerror(res), statusCode);
			curl_easy_cleanup(curl_handle);
			curl_global_cleanup();
			if (chunk.memory != NULL)
			{
				free(chunk.memory);
				chunk.memory = NULL;
			}
			return result;
		}
		else {
			log4cpp::Category::getRoot().debug("%lu bytes retrieved\n", (int)chunk.size);
		}
		res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &statusCode);
		curl_easy_cleanup(curl_handle);
		curl_global_cleanup();

		result = std::string(chunk.memory, chunk.size);

		if (chunk.memory != NULL)
		{
			free(chunk.memory);
			chunk.memory = NULL;
		}

		return result;
	}

	std::string HttpUtil::HttpPost(const char* url, const char* data, int dataLength, long &statusCode, int timeout)
	{
		CURL *curl_handle;
		CURLcode res;
		std::string result;
		struct MemoryStruct chunk;

		struct WriteThis wt;
		wt.readptr = data;
		wt.sizeleft = dataLength;

		log4cpp::Category::getRoot().debug("upload file %s", url);

		curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json; charset=utf-8");
		
		curl_global_init(CURL_GLOBAL_ALL);
		curl_handle = curl_easy_init();
		curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, timeout);
		curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, read_callback);
		curl_easy_setopt(curl_handle, CURLOPT_READDATA, (void *)&wt);
		//curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);

		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

#ifdef USE_CHUNKED
		{
			struct curl_slist *chunk = NULL;

			chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
			res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		}
#else
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, (long)wt.sizeleft);
#endif

#ifdef DISABLE_EXPECT
		{
			struct curl_slist *chunk = NULL;

			chunk = curl_slist_append(chunk, "Expect:");
			res = curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, chunk);
		}
#endif

		res = curl_easy_perform(curl_handle);
	
		if (res != CURLE_OK) {
			res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &statusCode);
			log4cpp::Category::getRoot().error("curl_easy_perform() failed: %s; retcode :%d\n", curl_easy_strerror(res), statusCode);
			curl_slist_free_all(headers);
			curl_easy_cleanup(curl_handle);
			curl_global_cleanup();
			if (chunk.memory != NULL)
			{
				free(chunk.memory);
				chunk.memory = NULL;
			}
			return result;
		}
		else {
			log4cpp::Category::getRoot().info("%lu bytes retrieved", (int)chunk.size);
		}
		res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &statusCode);
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl_handle);
		curl_global_cleanup();

		result = std::string(chunk.memory, chunk.size);
		if (chunk.memory != NULL)
		{
			free(chunk.memory);
			chunk.memory = NULL;
		}

		return result;
	}

	size_t HttpUtil::HttpUpload(const char* url, const char *data, size_t length, int timeout)
	{
		CURL *curl_handle;

		struct MemoryStruct chunk;
		struct WriteThis wt;
		wt.readptr = data;
		wt.sizeleft = length;

		curl_global_init(CURL_GLOBAL_ALL);
		curl_handle = curl_easy_init();


		log4cpp::Category::getRoot().info("Uploading data (%d bytes) to %s", length, url);

		curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, timeout);
		curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, read_callback);
		curl_easy_setopt(curl_handle, CURLOPT_READDATA, (void *)&wt);
		curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)length);

		CURLcode res = curl_easy_perform(curl_handle);
		if (res != CURLE_OK) {
			log4cpp::Category::getRoot().error("curl_easy_perform() failed: %s", curl_easy_strerror(res));
			curl_easy_cleanup(curl_handle);
			return 1;
		}

		return 0;
	}
}
