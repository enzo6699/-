#include "ConvertCharCode.h"

#if defined(_MSC_VER)
#include <windows.h>
#endif
namespace tce{

	int ConvertCharCode::UnicodeToGB2312(const wchar_t *src, char **dest)
	{
#if defined(_MSC_VER)
		char* buffer;
		int size = ::WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);
		// null termidated wchar''s buffer
		buffer = new char[size];
		int ret = ::WideCharToMultiByte(CP_ACP, NULL, src, -1, buffer, size + 1, NULL, NULL);

		if (*dest != 0)
			delete *dest;
		*dest = buffer;

		return ret;
#else
		return 0;
#endif
	}


	int ConvertCharCode::Gb2312ToUnicode(const char *src, wchar_t **dest)
	{
#if defined(_MSC_VER)
		int length = strlen(src);                // null terminated buffer
		wchar_t *buffer = new wchar_t[length + 1];   // WCHAR means unsinged short, 2 bytes
		// provide enough buffer size for Unicodes

		int ret = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, length, buffer, length);
		buffer[ret] = 0;

		if (*dest != 0)
			delete *dest;
		*dest = buffer;

		return ret;
#else
		return 0;
#endif
	}


	std::string & ConvertCharCode::Utf8ToAnsi(std::string &src, std::string &des)
	{
#if defined(_MSC_VER)
		size_t n = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src.c_str(), -1, NULL, 0);
		wchar_t *sl = new wchar_t[n];

		//多字节(UTF8)编码转换为宽字节编码
		MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, sl, n);

		n = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)sl, -1, NULL, 0, NULL, NULL);
		char *sm = new char[n + 1];
		memset(sm, 0, n + 1);
		//宽字节转换为多字节(Ansi)编码
		WideCharToMultiByte(CP_ACP, 0, sl, -1, sm, n, NULL, 0);

		des = sm;
		delete[]sl;
		delete[]sm;

        return des;
#else
        return "";
#endif
	}

	std::string & ConvertCharCode::Utf8ToAnsi(const char* src, int length, std::string &des)
	{
#if defined(_MSC_VER)
		size_t n = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src, length, NULL, 0);
		wchar_t *sl = new wchar_t[n];

		//多字节(UTF8)编码转换为宽字节编码
		MultiByteToWideChar(CP_UTF8, 0, (LPCCH)src, -1, sl, n);

		n = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)sl, -1, NULL, 0, NULL, NULL);
		char *sm = new char[n + 1];
		memset(sm, 0, n + 1);
		//宽字节转换为多字节(Ansi)编码
		WideCharToMultiByte(CP_ACP, 0, sl, -1, sm, n, NULL, 0);

		des = sm;
		delete[]sl;
		delete[]sm;

        return des;
#else
        return "";
#endif
	}

	//Ansi编码转换为Unicode(Utf16)编码
	std::wstring & ConvertCharCode::AnsiToUtf16(std::string &scr, std::wstring &des)
	{
#if defined(_MSC_VER)
		size_t n = scr.size();

		wchar_t *sl = new wchar_t[n + 1];

		MultiByteToWideChar(CP_ACP, 0, scr.c_str(), -1, sl, n + 1);

		des = sl;
		delete[]sl;

        return des;
#else
        return "";
#endif
	}

	void ConvertCharCode::GbkToUtf8(std::string &src, std::string &des)
	{
#if defined(_MSC_VER)
		int len = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src.c_str(), -1, NULL, 0);

		unsigned short * wszUtf8 = new unsigned short[len + 1];
		memset(wszUtf8, 0, len * 2 + 2);

		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src.c_str(), -1, (LPWSTR)wszUtf8, len);
		len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, NULL, 0, NULL, NULL);
		char *szUtf8 = new char[len + 1];
		memset(szUtf8, 0, len + 1);
		WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, szUtf8, len, NULL, NULL);
		des = szUtf8;
		delete[] szUtf8;
		delete[] wszUtf8;
#endif
	}

	void ConvertCharCode::AnsiToUTF8(std::string &src, std::string& des)
	{
#if defined(_MSC_VER)
		int len = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src.c_str(), -1, NULL, 0);

		unsigned short * wszUtf8 = new unsigned short[len + 1];
		memset(wszUtf8, 0, len * 2 + 2);

		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src.c_str(), -1, (LPWSTR)wszUtf8, len);
		len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, NULL, 0, NULL, NULL);
		char *szUtf8 = new char[len + 1];
		memset(szUtf8, 0, len + 1);
		WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, szUtf8, len, NULL, NULL);
		des = szUtf8;
		delete[] szUtf8;
		delete[] wszUtf8;
#endif
	}
}
