# Win32WebView2Test

This is Win32 equivalent for [Executing Javascript on WPF WebView2](https://github.com/BithreenGirlen/WpfWebView2Test)

## Background

WPF has comfortable development environment, but built result has many files for runtime, and sometimes seems messy.  
This is _l'essai_ to slim down the built result developping by Win32.

## Key point

### Json Serialisation/Deserialisation

Win32 lacks native library equivalent to .Net JsonSerializer.Serialize/Deserialize.  
The serialisation is to escape characters and add quotaions; deserialisation vice versa.

#### Serialisation
``` cpp
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
```
#### Deserialisation
``` cpp
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
```

## aync/await

Win32 also lacks .Net aync/await. Using event is Win32 way to wait for JavaScript execution.

### asynchronisation
``` cpp
std::wstring CMainWindow::ExecuteDecryptFunctionOnWebPage(const std::wstring &wstrText)
{
    if (m_pWebView != nullptr)
    {
        std::wstring wstrScript = L"decrypt(" + SerialiseJsonString(wstrText) + L")";

        wil::unique_handle hEvent(::CreateEvent(nullptr, FALSE, FALSE, nullptr));
        std::wstring wstrResult;
        HRESULT hr = m_pWebView->ExecuteScript(
            wstrScript.c_str(),
            Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                [&hEvent, &wstrResult](HRESULT hResult, LPCWSTR pwzResultObjectAsJson)
                ->HRESULT
                {
                    if (SUCCEEDED(hResult) && pwzResultObjectAsJson != nullptr)
                    {
                        wstrResult = DeserialiseJsonString(pwzResultObjectAsJson);
                    }
                    ::SetEvent(hEvent.get());
                    return S_OK;
                }
            ).Get()
                    );
        if (SUCCEEDED(hr))
        {
            DWORD dwIndex = 0;
            hr = ::CoWaitForMultipleHandles(
                COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS | COWAIT_INPUTAVAILABLE,
                INFINITE, 1, hEvent.addressof(), &dwIndex);
            if (SUCCEEDED(hr))
            {
                return wstrResult;
            }
        }
    }

    return std::wstring();
}
```

## Result

Compact built files; one dll and one exe.
