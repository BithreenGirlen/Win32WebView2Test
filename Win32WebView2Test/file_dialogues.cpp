
#include <atlbase.h>
#include <shobjidl.h>

#include "file_dialogues.h"

struct ComInit
{
	HRESULT hr;
	ComInit() : hr(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)) {}
	~ComInit() { if (SUCCEEDED(hr)) ::CoUninitialize(); }
};

/*HTMLファイル選択*/
wchar_t* SelectHtmlFile(HWND hParentWnd)
{
	ComInit sComInit;
	CComPtr<IFileOpenDialog> pFileDialog;
	HRESULT hr = pFileDialog.CoCreateInstance(CLSID_FileOpenDialog);

	if (SUCCEEDED(hr)) {
		COMDLG_FILTERSPEC filter[1]{};
		filter[0].pszName = L"HTML files";
		filter[0].pszSpec = L"*.htm;*.html;";
		hr = pFileDialog->SetFileTypes(1, filter);
		if (SUCCEEDED(hr))
		{
			FILEOPENDIALOGOPTIONS opt{};
			pFileDialog->GetOptions(&opt);
			pFileDialog->SetOptions(opt | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM);

			if (SUCCEEDED(pFileDialog->Show(hParentWnd)))
			{
				CComPtr<IShellItem> pSelectedItem;
				pFileDialog->GetResult(&pSelectedItem);

				wchar_t* pPath;
				pSelectedItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);

				return pPath;
			}
		}
	}

	return nullptr;
}
/*フォルダ選択ダイアログ*/
wchar_t* SelectWorkFolder(HWND hParentWnd)
{
	ComInit sComInit;
	CComPtr<IFileOpenDialog> pFolderDlg;
	HRESULT hr = pFolderDlg.CoCreateInstance(CLSID_FileOpenDialog);

	if (SUCCEEDED(hr)) {
		FILEOPENDIALOGOPTIONS opt{};
		pFolderDlg->GetOptions(&opt);
		pFolderDlg->SetOptions(opt | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM);

		if (SUCCEEDED(pFolderDlg->Show(hParentWnd)))
		{
			CComPtr<IShellItem> pSelectedItem;
			pFolderDlg->GetResult(&pSelectedItem);

			wchar_t* pPath;
			pSelectedItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);

			return pPath;
		}
	}

	return nullptr;
}
