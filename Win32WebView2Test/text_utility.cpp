
#include <shlwapi.h>

#include "text_utility.h"


/*std::string to std::wstring*/
std::wstring WidenUtf8(const std::string &str)
{
	if (!str.empty())
	{
		int iLen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), nullptr, 0);
		if (iLen > 0)
		{
			std::wstring wstr(iLen, 0);
			::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &wstr[0], iLen);
			return wstr;
		}
	}

	return std::wstring();
}
/*std::wstring to std::string*/
std::string NarrowUtf8(const std::wstring &wstr)
{
	if (!wstr.empty())
	{
		int iLen = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), nullptr, 0, nullptr, nullptr);
		if (iLen > 0)
		{
			std::string str(iLen, 0);
			::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), &str[0], iLen, nullptr, nullptr);
			return str;
		}
	}
	return std::string();
}
