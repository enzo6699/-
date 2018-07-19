#ifndef _COVERT_CHAR_CODE_H_
#define _COVERT_CHAR_CODE_H_

#include <string>

namespace tce{

	class ConvertCharCode
	{
	public:
		static int UnicodeToGB2312(const wchar_t *src, char **dest);

		static int Gb2312ToUnicode(const char *src, wchar_t **dest);

        static std::string & Utf8ToAnsi(std::string &src, std::string &des);
        static std::string & Utf8ToAnsi(const char* src, int length, std::string &des);

		//Ansi编码转换为Unicode(Utf16)编码
        static std::wstring & AnsiToUtf16(std::string &scr, std::wstring &des);

		static void GbkToUtf8(std::string &src, std::string &des);

		static void AnsiToUTF8(std::string &src, std::string& des);
	};


}
#endif
