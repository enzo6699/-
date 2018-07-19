#include "Dlopen.h"

namespace tce {

	void* Dlopen::tce_dlopen(const char* path)
	{
#if defined(_MSC_VER)
		return  LoadLibrary(path);
#else
		return dlopen(path, RTLD_NOW | RTLD_GLOBAL);
#endif

	}

	void* Dlopen::tce_dlsym(void* handle, const char* symbol)
	{
#if defined(_MSC_VER)
		return GetProcAddress((HMODULE)handle, symbol);
#else
		return dlsym(handle, symbol);
#endif
	}

	bool Dlopen::tce_dlclose(void* handle)
	{
		if (handle != NULL)
		{
#if defined(_MSC_VER)
			FreeLibrary((HMODULE)handle);
#else
			dlclose(handle);
#endif
		}

		return true;
	}
}