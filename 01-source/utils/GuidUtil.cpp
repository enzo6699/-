#include "GuidUtil.h"
#if defined(_MSC_VER)
#include <guiddef.h>
#include <combaseapi.h>
#else
#include <stdio.h>
#include <uuid/uuid.h>
#define GUID uuid_t
#endif

#define GUID_LEN 64

namespace tce {
	std::string GuidUtil::gguid()
	{
		GUID guid;
		char buffer[GUID_LEN] = { 0 };

#if defined(_MSC_VER)
		CoCreateGuid(&guid);
#else
		uuid_generate(reinterpret_cast<unsigned char *>(&guid));
#endif

#if defined(_MSC_VER)
		_snprintf_s(buffer, sizeof(buffer),
			"%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2],
			guid.Data4[3], guid.Data4[4], guid.Data4[5],
			guid.Data4[6], guid.Data4[7]);
#else // MSVC
		snprintf(buffer, sizeof(buffer),
			"%02X%02X%02X%02X%02X%02X%02X%02x%02X%02X%02X%02X%02X%02X%02X%02X",
			guid[0], guid[1], guid[2],
			guid[3], guid[4], guid[5],
			guid[6], guid[7], guid[8],
			guid[9], guid[10], guid[11],
			guid[12], guid[13], guid[14], guid[15]);
#endif

		/*
		(buffer, sizeof(buffer),
		"%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2],
		guid.Data4[3], guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
		*/

		return std::string(buffer);
	}
}