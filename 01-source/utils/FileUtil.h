#ifndef _FILE_UTIL_H_
#define _FILE_UTIL_H_

#include <iostream>
#include <vector>
#if defined(__GNUC__)
#include <dirent.h>
#endif

// find dir
#ifndef FILE_ATTRIBUTE_DIRECTORY  
#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  
#endif

// find file
#ifndef FILE_ATTRIBUTE_ARCHIVE  
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020  
#endif

namespace tce {
	class FileUtil
	{
	public:
		static int SaveFile(std::string filename, void *data, int dataLen);

		static std::string GetWorkDir();
		static std::string GetFilename(const std::string& s);
		static std::string GetFilenameWithoutExt(const std::string& s);

		// linux only support find file
		static void FindFiles(std::vector<std::string>& vecPath, const std::string& strPath, int nFileType);
        static bool CreatePath(const char*  path);
        static bool CheckAndCreatePath(const char* s, bool isFile);
	};
}
#endif
