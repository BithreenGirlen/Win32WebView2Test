#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>
#include <wrl.h>
#include <wil/com.h>

#include <string>

#include "WebView2.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();
	bool Create(HINSTANCE hInstance);
	int MessageLoop();
	HWND GetHwnd()const { return m_hWnd;}
private:
	std::wstring m_class_name = L"Win32 WebView2 Test";
	std::wstring m_window_name = L"Win32 WebView2";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnCommand(WPARAM wParam);

	enum Menu
	{
		kFileOpen = 1, kSelectFolder,
	};
	enum MenuBar{kFile, };

	HMENU m_hMenuBar = nullptr;

	void InitialiseMenuBar();

	void MenuOnFileOpen();
	void MenuOnSelectFolder();

	HRESULT m_hrComInit = E_FAIL;
	wil::com_ptr<ICoreWebView2> m_pWebView;
	wil::com_ptr<ICoreWebView2Controller> m_pWebViewController;
	wil::com_ptr<ICoreWebView2Environment> m_pWebView2Environment;

	void InitialiseWebViewEnvironment();
	HRESULT OnCreateEnvironmentCompleted(HRESULT hResult, ICoreWebView2Environment* pWebViewEnvironment);
	HRESULT OnCreateWebViewControllerCompleted(HRESULT hResult, ICoreWebView2Controller* pWebViewController);

	void ResizeWebViewBound();
	std::wstring ExecuteDecryptFunctionOnWebPage(const std::wstring &wstrText);
};

#endif //MAIN_WINDOW_H_