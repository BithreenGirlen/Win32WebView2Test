﻿// Win32WebView2Test.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "Win32WebView2Test.h"
#include "main_window.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。

    int iRet = 0;
    CMainWindow* pMainWindow = new CMainWindow();
    if (pMainWindow != nullptr)
    {
        bool bRet = pMainWindow->Create(hInstance);
        if (bRet)
        {
            ::ShowWindow(pMainWindow->GetHwnd(), nCmdShow);
            iRet = pMainWindow->MessageLoop();
        }

        delete pMainWindow;
    }

    return iRet;
}
