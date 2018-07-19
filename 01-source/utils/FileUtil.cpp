#include "FileUtil.h"
#include "StringUtil.h"
#ifdef _MSC_VER
#include <Windows.h>
#endif
#include <algorithm>

namespace tce {

	int FileUtil::SaveFile(std::string filename, void *data, int dataLen)
	{

		FILE *pFile = fopen(filename.c_str(), "wb");

		if (pFile == NULL)
		{
			return  1;
		}

		fwrite(data, dataLen, 1, pFile);
		fflush(pFile);
		fclose(pFile);

		return 0;
	}

	std::string FileUtil::GetWorkDir()
	{
#if defined(_MSC_VER)
		char szExeFilePath[255] = { 0 };
		memset(szExeFilePath, 0, 255);
		::GetModuleFileNameA(NULL, szExeFilePath, 255);
		std::string strPath = std::string(szExeFilePath);
		strPath = strPath.substr(0, strPath.find_last_of("\\"));
		strPath += "\\";
		return strPath;
#else
		return "";
#endif
	}

	std::string FileUtil::GetFilename(const std::string& s)
	{
		std::string::size_type index = std::max<std::string::size_type>(s.find_last_of("\\"), s.find_last_of("/"));
		if (index == std::string::npos)
			return s;
		return s.substr(index + 1, s.size() - index);
	}

	std::string FileUtil::GetFilenameWithoutExt(const std::string& s)
	{
		std::string r = GetFilename(s);

		std::string::size_type index = r.find_last_of(".");
		if (index == std::string::npos)
			return r;
		return r.substr(0, index);
	}


	void FileUtil::FindFiles(std::vector<std::string>& vecPath, const std::string& strPath, int nFileType)
	{
#if defined(_MSC_VER)
		WIN32_FIND_DATA FindFileData;
		void* hFind;
		hFind = FindFirstFile(strPath.c_str(), &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			return;
		}
		else
		{
			do
			{
				if ((FindFileData.dwFileAttributes & nFileType) != 0 &&
					0 != strcmp(FindFileData.cFileName, ".") &&
					0 != strcmp(FindFileData.cFileName, ".."))
				{
					vecPath.push_back(FindFileData.cFileName);
				}
			} while (FindNextFile(hFind, &FindFileData));
		}
		// 查找结束 
		FindClose(hFind);
#else
		DIR *dir = opendir(strPath.c_str());
		dirent *entity = NULL;

		while (entity = readdir(dir)) {
			std::string filename(entity->d_name);
			if (filename == "." || filename == "..") {
				continue;
			}

			filename.insert(0, strPath);
			vecPath.push_back(filename);
		}

		closedir(dir);
#endif


	}


    bool FileUtil::CheckAndCreatePath(const char* s, bool isFile)
    {
        std::vector<std::string> v;
        int nRet = StringUtil::split(v, s, '/');
        if (nRet == 1)
        {
            v.clear();
            nRet = StringUtil::split(v, s, '\\');
        }
        std::string  strTemp("");
        strTemp = v[0];

        //这种情况为拷贝文件创建文件夹
        if (isFile)
        {
            for (size_t i = 1; i < v.size() - 1; i++)
            {
                strTemp += "/" + v[i];

                if (!CreatePath(strTemp.c_str()))
                    return  false;
            }
        }
        //这种只为创建文件夹
        else
        {
            for (size_t i = 1; i < v.size(); i++)
            {
                strTemp += "/" + v[i];
                if (!CreatePath(strTemp.c_str()))
                    return  false;
            }
        }

        return  true;
    }

    bool FileUtil::CreatePath(const char*  path)
    {
#if defined(_MSC_VER)
        DWORD dwAttr = GetFileAttributesA(path);

        if (dwAttr == 0xFFFFFFFF)
        {
            if (CreateDirectory(path, NULL))
            {
                return true;
            }
            else
                return  false;
        }
        else if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
        {
            return true;
        }

        return false;
#else
    char command[512] = { 0 };
    sprintf(command, "mkdir -p %s", path);
    system(command);
    return true;
#endif
    }
}
