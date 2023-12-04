
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <stdlib.h>

#include "json_serialisation.h"
#include "text_utility.h"

/*ì‡ïîä÷êî*/

char* SerialiseJson(const char* src);
char* DeserialiseJson(const char* src);


std::wstring SerialiseJsonString(const std::wstring &src)
{
	std::string str = NarrowUtf8(src);
	char* pBuffer = SerialiseJson(str.c_str());
	if (pBuffer != nullptr)
	{
		std::wstring wstr = WidenUtf8(pBuffer);
		free(pBuffer);
		return wstr;
	}

	return std::wstring();
}
std::wstring DeserialiseJsonString(const std::wstring &src)
{
	std::string str = NarrowUtf8(src);
	char* pBuffer = DeserialiseJson(str.c_str());
	if (pBuffer != nullptr)
	{
		std::wstring wstr = WidenUtf8(pBuffer);
		free(pBuffer);
		return wstr;
	}

	return std::wstring();
}

/*JSONòAë±âª*/
char* SerialiseJson(const char* src)
{
	if (src == nullptr)return nullptr;

	const char HexDigits[] = "0123456789ABCDEF";

	size_t nSrcLen = strlen(src);
	char* pResult = static_cast<char*>(malloc(nSrcLen * 6 + 3LL));
	if (pResult == nullptr)return nullptr;

	char* pPos = pResult;
	*pPos++ = '"';

	for (size_t i = 0; i < nSrcLen; ++i)
	{
		char p = *(src + i);

		switch (p)
		{
		case '\"':
			*pPos++ = '\\';
			*pPos++ = '\"';
			break;
		case '\\':
			*pPos++ = '\\';
			*pPos++ = '\\';
			break;
		case '\b':
			*pPos++ = '\\';
			*pPos++ = 'b';
			break;
		case '\f':
			*pPos++ = '\\';
			*pPos++ = 'f';
			break;
		case '\r':
			*pPos++ = '\\';
			*pPos++ = 'r';
			break;
		case '\n':
			*pPos++ = '\\';
			*pPos++ = 'n';
			break;
		case '\t':
			*pPos++ = '\\';
			*pPos++ = 't';
			break;
		default:
			if ((p >= 0 && p <= 0x1f) || p == 0x2b)
			{
				*pPos++ = '\\';
				*pPos++ = 'u';
				*pPos++ = '0';
				*pPos++ = '0';
				*pPos++ = HexDigits[(p & 0xf0) >> 4];
				*pPos++ = HexDigits[p & 0x0f];
			}
			else
			{
				*pPos++ = p;
			}
		}

	}

	size_t nDstLen = pPos - pResult;
	char* pTemp = static_cast<char*>(realloc(pResult, nDstLen + 2LL));
	if (pTemp != nullptr)
	{
		pResult = pTemp;
	}
	*(pResult + nDstLen) = '"';
	*(pResult + nDstLen + 1LL) = '\0';

	return pResult;
}
/*JSONó£éUâª*/
char* DeserialiseJson(const char* src)
{
	if (src == nullptr)return nullptr;

	size_t nSrcLen = strlen(src);
	if (src[0] != '"' || src[nSrcLen - 1] != '"')return nullptr;

	char* pResult = static_cast<char*>(malloc(nSrcLen + 1LL));
	if (pResult == nullptr)return nullptr;

	size_t nPos = 0;
	size_t nLen = 0;
	char* pp = const_cast<char*>(src + 1);

	for (;;)
	{
		char* p = strchr(pp, '\\');
		if (p == nullptr)
		{
			nLen = nSrcLen - (pp - src);
			memcpy(pResult + nPos, pp, nLen);
			nPos += nLen;
			break;
		}

		nLen = p - pp;
		memcpy(pResult + nPos, pp, nLen);
		nPos += nLen;
		pp = p + 1;

		switch (*pp)
		{
		case 'b':
			*(pResult + nPos) = '\b';
			++nPos;
			++pp;
			break;
		case 'f':
			*(pResult + nPos) = '\f';
			++nPos;
			++pp;
			break;
		case 'r':
			*(pResult + nPos) = '\r';
			++nPos;
			++pp;
			break;
		case 'n':
			*(pResult + nPos) = '\n';
			++nPos;
			++pp;
			break;
		case 't':
			*(pResult + nPos) = '\t';
			++nPos;
			++pp;
			break;
		case 'u':
			++pp;
			char pzBuffer[3]{};
			memcpy(pzBuffer, pp, 2);
			if (isxdigit(static_cast<unsigned char>(pzBuffer[0])) && isxdigit(static_cast<unsigned char>(pzBuffer[1])))
			{
				if (pzBuffer[0] != '0' && pzBuffer[1] != '0')
				{
					*(pResult + nPos) = static_cast<char>(strtol(pzBuffer, nullptr, 16));
					++nPos;
				}
				pp += 2;
			}
			memcpy(pzBuffer, pp, 2);
			if (isxdigit(static_cast<unsigned char>(pzBuffer[0])) && isxdigit(static_cast<unsigned char>(pzBuffer[1])))
			{
				*(pResult + nPos) = static_cast<char>(strtol(pzBuffer, nullptr, 16));
				++nPos;
				pp += 2;
			}
			break;
		}

		if (pp == nullptr)break;
	}

	char* pTemp = static_cast<char*>(realloc(pResult, nPos));
	if (pTemp != nullptr)
	{
		pResult = pTemp;
	}
	*(pResult + nPos - 1LL) = '\0';

	return pResult;
}
