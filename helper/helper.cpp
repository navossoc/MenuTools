// helper.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	// Infers the current directory from fully qualified path, since the
	// working directory was not defined by the task created by schtasks.
	LPWSTR *szArglist;
	int nArgs;
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (szArglist == NULL)
	{
		return 1;
	}

	WCHAR drive[_MAX_DRIVE];
	WCHAR dir[_MAX_DIR];
	if (_wsplitpath_s(szArglist[0], drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0) != 0)
	{
		LocalFree(szArglist);
		return 1;
	}

	LocalFree(szArglist);

	WCHAR lpDirectory[MAX_PATH];
	_wmakepath_s(lpDirectory, drive, dir, NULL, NULL);

	// Execute process
	SHELLEXECUTEINFOW ShExecInfo;

	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOASYNC;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = L"open";
	ShExecInfo.lpFile = L"MenuTools.exe";
	ShExecInfo.lpParameters = lpCmdLine;
	ShExecInfo.lpDirectory = lpDirectory;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;

	if (ShellExecuteExW(&ShExecInfo) == FALSE)
	{
		return GetLastError();
	}

	return 0;
}
