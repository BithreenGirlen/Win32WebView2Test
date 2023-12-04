
#include <Windows.h>
#include <shlwapi.h>

#include "file_system_utility.h"
#include "text_utility.h"

/*内部関数*/

char* LoadExistingFile(const wchar_t* pwzFilePath, unsigned long* ulSize);
std::wstring GetCurrentProcessPath();

/*指定階層のファイル・フォルダ一覧生成*/
bool CreateFilePathList(const wchar_t* pwzFolderPath, const wchar_t* pwzFileExtension, std::vector<std::wstring>& paths)
{
	if (pwzFolderPath == nullptr)return false;

	std::wstring wstrParent = pwzFolderPath;
	wstrParent += L"\\/";
	std::wstring wstrPath = wstrParent + L'*';
	if (pwzFileExtension != nullptr)
	{
		wstrPath += pwzFileExtension;
	}

	WIN32_FIND_DATA sFindData;
	std::vector<std::wstring> wstrNames;

	HANDLE hFind = ::FindFirstFile(wstrPath.c_str(), &sFindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		if (pwzFileExtension != nullptr)
		{
			do
			{
				/*ファイル一覧*/
				if (!(sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					wstrNames.push_back(sFindData.cFileName);
				}
			} while (::FindNextFile(hFind, &sFindData));
		}
		else
		{
			do
			{
				/*フォルダ一覧*/
				if ((sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					if (wcscmp(sFindData.cFileName, L".") != 0 && wcscmp(sFindData.cFileName, L"..") != 0)
					{
						wstrNames.push_back(sFindData.cFileName);
					}
				}
			} while (::FindNextFile(hFind, &sFindData));
		}

		::FindClose(hFind);
	}

	/*名前順に整頓*/
	for (size_t i = 0; i < wstrNames.size(); ++i)
	{
		size_t nIndex = i;
		for (size_t j = i + 1; j < wstrNames.size() - 1; ++j)
		{
			if (::StrCmpLogicalW(wstrNames.at(nIndex).c_str(), wstrNames.at(j).c_str()) > 0)
			{
				nIndex = j;
			}
		}
		std::swap(wstrNames.at(i), wstrNames.at(nIndex));
	}

	for (const std::wstring& wstr : wstrNames)
	{
		paths.push_back(wstrParent + wstr);
	}

	return paths.size() > 0;
}
/*文字列としてファイル読み込み*/
std::wstring LoadFileAsString(const wchar_t* pwzFilePath)
{
	DWORD dwSize = 0;
	char* pBuffer = LoadExistingFile(pwzFilePath, &dwSize);
	if (pBuffer != nullptr)
	{
		std::string str;
		str.reserve(dwSize);
		/*終端文字包含*/
		for (size_t i = 0; i < dwSize; ++i)
		{
			str.push_back(*(pBuffer + i));
		}

		free(pBuffer);
		std::wstring wstr = WidenUtf8(str);
		return wstr;
	}

	return std::wstring();
}

/*ファイルのメモリ展開*/
char* LoadExistingFile(const wchar_t* pwzFilePath, unsigned long *ulSize)
{
	HANDLE hFile = ::CreateFile(pwzFilePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = ::GetFileSize(hFile, nullptr);
		if (dwSize != INVALID_FILE_SIZE)
		{
			char* pBuffer = static_cast<char*>(malloc(static_cast<size_t>(dwSize + 1ULL)));
			if (pBuffer != nullptr)
			{
				DWORD dwRead = 0;
				BOOL iRet = ::ReadFile(hFile, pBuffer, dwSize, &dwRead, nullptr);
				if (iRet)
				{
					::CloseHandle(hFile);
					*(pBuffer + dwRead) = '\0';
					*ulSize = dwRead;

					return pBuffer;
				}
				else
				{
					free(pBuffer);
				}
			}
		}
		::CloseHandle(hFile);
	}

	return nullptr;
}
/*実行プロセスの階層取得*/
std::wstring GetCurrentProcessPath()
{
	wchar_t pwzPath[MAX_PATH]{};
	::GetModuleFileName(nullptr, pwzPath, MAX_PATH);
	std::wstring::size_type nPos = std::wstring(pwzPath).find_last_of(L"\\/");
	return std::wstring(pwzPath).substr(0, nPos) + L"\\/";
}
/*作業フォルダ作成*/
std::wstring CreateWorkFolder(const wchar_t* pwzFolderName)
{
	std::wstring wstrFolder = GetCurrentProcessPath();
	if (pwzFolderName != nullptr)
	{
		wstrFolder += pwzFolderName;
		wstrFolder += L"\\/";
		::CreateDirectory(wstrFolder.c_str(), nullptr);
	}
	return wstrFolder;
}
/*経路からファイル名取得*/
std::wstring GetFileNameFromFilePath(const wchar_t* pwzFilePath)
{
	if (pwzFilePath != nullptr)
	{
		std::wstring wstrFilePath = pwzFilePath;
		size_t nPos = wstrFilePath.find_last_of(L"\\/");
		if (nPos != std::wstring::npos)
		{
			return wstrFilePath.substr(nPos + 1);
		}
	}

	return std::wstring();
}
/*文字列のファイル書き込み*/
bool WriteStringToFile(const std::wstring& wstrData, const wchar_t* pwzFilePath)
{
	BOOL iRet = 0;

	if (pwzFilePath != nullptr)
	{
		std::string str = NarrowUtf8(wstrData);
		HANDLE hFile = ::CreateFile(pwzFilePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::SetFilePointer(hFile, NULL, nullptr, FILE_END);

			DWORD bytesWrite = 0;
			iRet = ::WriteFile(hFile, str.data(), static_cast<DWORD>(str.size()), &bytesWrite, nullptr);
			::CloseHandle(hFile);
		}
	}
	return iRet > 0;
}