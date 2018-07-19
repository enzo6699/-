#ifndef _DL_OPEN_H_
#define _DL_OPEN_H_

#if defined(_MSC_VER)
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace tce {
	class Dlopen
	{
	public:
		static void* tce_dlopen(const char* path);
		static void* tce_dlsym(void* handle, const char* symbol);
		static bool tce_dlclose(void* handle);
	};
}

#endif
