
#include <Windows.h>
#include <CommCtrl.h>

#include "main_window.h"
#include "file_dialogues.h"
#include "file_system_utility.h"
#include "json_serialisation.h"
#include "resource.h"

#pragma comment(lib, "Comctl32.lib")

CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::Create(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_583E5A));
    wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_ICON_583E5A);
    wcex.lpszClassName = m_class_name.c_str();
    wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_583E5A));

    if (::RegisterClassExW(&wcex))
    {
        m_hInstance = hInstance;

        m_hWnd = ::CreateWindowW(m_class_name.c_str(), m_window_name.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 450, nullptr, nullptr, hInstance, this);
        if (m_hWnd != nullptr)
        {
            return true;
        }
        else
        {
            std::wstring wstrMessage = L"CreateWindowExW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
        }
    }
    else
    {
        std::wstring wstrMessage = L"RegisterClassW failed; code: " + std::to_wstring(::GetLastError());
        ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    }

	return false;
}

int CMainWindow::MessageLoop()
{
    MSG msg;

    for (;;)
    {
        BOOL bRet = ::GetMessageW(&msg, 0, 0, 0);
        if (bRet > 0)
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
        else if (bRet == 0)
        {
            /*ループ終了*/
            return static_cast<int>(msg.wParam);
        }
        else
        {
            /*ループ異常*/
            std::wstring wstrMessage = L"GetMessageW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
            return -1;
        }
    }
    return 0;
}
/*C CALLBACK*/
LRESULT CMainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMainWindow* pThis = nullptr;
    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = reinterpret_cast<CMainWindow*>(pCreateStruct->lpCreateParams);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }

    pThis = reinterpret_cast<CMainWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (pThis != nullptr)
    {
        return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*メッセージ処理*/
LRESULT CMainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return OnCreate(hWnd);
    case WM_DESTROY:
        return OnDestroy();
    case WM_CLOSE:
        return OnClose();
    case WM_PAINT:
        return OnPaint();
    case WM_SIZE:
        return OnSize();
    case WM_COMMAND:
        return OnCommand(wParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
    m_hWnd = hWnd;

    InitialiseMenuBar();

    m_hrComInit = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    InitialiseWebViewEnvironment();

    return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
    if (SUCCEEDED(m_hrComInit))
    {
        ::CoUninitialize();
    }

    ::PostQuitMessage(0);

    return 0;
}
/*WM_CLOSE*/
LRESULT CMainWindow::OnClose()
{
    ::DestroyWindow(m_hWnd);
    ::UnregisterClassW(m_class_name.c_str(), m_hInstance);

    return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = ::BeginPaint(m_hWnd, &ps);

    ::EndPaint(m_hWnd, &ps);

    return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{
    ResizeWebViewBound();

    return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam)
{
    int wmKind = HIWORD(wParam);
    int wmId = LOWORD(wParam);
    if (wmKind == 0)
    {
        /*Menus*/
        switch (wmId)
        {
        case Menu::kFileOpen:
            MenuOnFileOpen();
            break;
        case Menu::kSelectFolder:
            MenuOnSelectFolder();
            break;
        }
    }
    if (wmKind > 1)
    {
        /*Controls*/
    }

    return 0;
}

/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
    HMENU hMenuFile = nullptr;
    HMENU hMenuBar = nullptr;
    BOOL iRet = FALSE;

    if (m_hMenuBar != nullptr)return;

    hMenuFile = ::CreateMenu();
    if (hMenuFile == nullptr)goto failed;

    iRet = ::AppendMenuA(hMenuFile, MF_STRING, Menu::kFileOpen, "Open");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuFile, MF_STRING, Menu::kSelectFolder, "Select");
    if (iRet == 0)goto failed;

    hMenuBar = ::CreateMenu();
    if (hMenuBar == nullptr) goto failed;

    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuFile), "File");
    if (iRet == 0)goto failed;

    iRet = ::SetMenu(m_hWnd, hMenuBar);
    if (iRet == 0)goto failed;

    m_hMenuBar = hMenuBar;

    /*正常終了*/
    return;

failed:
    std::wstring wstrMessage = L"Failed to create menu; code: " + std::to_wstring(::GetLastError());
    ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    /*SetMenu成功後はウィンドウ破棄時に破棄されるが、今は紐づけ前なのでここで破棄する。*/
    if (hMenuFile != nullptr)
    {
        ::DestroyMenu(hMenuFile);
    }
    if (hMenuBar != nullptr)
    {
        ::DestroyMenu(hMenuBar);
    }

}
/*HTML選択*/
void CMainWindow::MenuOnFileOpen()
{
    wchar_t* pBuffer = SelectHtmlFile(m_hWnd);
    if (pBuffer != nullptr)
    {
        if (m_pWebView != nullptr)
        {
            m_pWebView->Navigate(pBuffer);
        }

        ::CoTaskMemFree(pBuffer);
    }
}
/*フォルダ選択*/
void CMainWindow::MenuOnSelectFolder()
{
    wchar_t* pBuffer = SelectWorkFolder(m_hWnd);
    if (pBuffer != nullptr)
    {
        std::vector<std::wstring> wstrFiles;
        bool bRet = CreateFilePathList(pBuffer, L".txt", wstrFiles);
        if (bRet)
        {
            std::wstring wstrFolder = CreateWorkFolder(L"decrypted");
            for (std::wstring wstrFile : wstrFiles)
            {
                std::wstring wstrText = LoadFileAsString(wstrFile.c_str());
                if (!wstrText.empty())
                {
                    std::wstring wstrResult = ExecuteDecryptFunctionOnWebPage(wstrText);
                    if (!wstrResult.empty())
                    {
                        std::wstring wstrFilePath = wstrFolder + GetFileNameFromFilePath(wstrFile.c_str());
                        WriteStringToFile(wstrResult, wstrFilePath.c_str());
                    }
                }
            }
        }

        ::CoTaskMemFree(pBuffer);
    }
}
/*WebView2環境初期設定*/
void CMainWindow::InitialiseWebViewEnvironment()
{
    HRESULT hr = ::CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        nullptr,
        nullptr,
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(this, &CMainWindow::OnCreateEnvironmentCompleted).Get()
    );
}
/*ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler Callback*/
HRESULT CMainWindow::OnCreateEnvironmentCompleted(HRESULT hResult, ICoreWebView2Environment* pWebViewEnvironment)
{
    if (SUCCEEDED(hResult) && pWebViewEnvironment != nullptr)
    {
        m_pWebView2Environment = pWebViewEnvironment;

        HRESULT hr = m_pWebView2Environment->CreateCoreWebView2Controller(
            m_hWnd,
            Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(this, &CMainWindow::OnCreateWebViewControllerCompleted).Get()
        );
    }

    return S_OK;
}
/*ICoreWebView2CreateCoreWebView2ControllerCompletedHandler Callback*/
HRESULT CMainWindow::OnCreateWebViewControllerCompleted(HRESULT hResult, ICoreWebView2Controller* pWebViewController)
{
    if (SUCCEEDED(hResult) && pWebViewController != nullptr)
    {
        m_pWebViewController = pWebViewController;

        HRESULT hr = m_pWebViewController->get_CoreWebView2(&m_pWebView);
        if (SUCCEEDED(hr))
        {
            ResizeWebViewBound();

            m_pWebView->Navigate(CreateWorkFolder(nullptr).c_str());
        }
    }

    return S_OK;
}
/*WebView2表示位置・寸法指定*/
void CMainWindow::ResizeWebViewBound()
{
    if (m_pWebViewController != nullptr)
    {
        RECT srcRect{};
        ::GetClientRect(m_hWnd, &srcRect);
        m_pWebViewController->put_Bounds(srcRect);
    }
}

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
