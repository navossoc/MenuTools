// MenuTools.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MenuTools.h"
#include "Hooks.h"
#include "Startup.h"

#include "MenuCommon/TrayIcon.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
HWND hWnd;										// current window handle
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
UINT uTrayId;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	Startup startup;
	// Command line arguments
	if (!startup.ParseFlags(GetCommandLineW()))
	{
		return FALSE;
	}

	// Single instance
	if (!startup.CreateJob())
	{
		return FALSE;
	}

	// Initialize global strings
	BOOL bIsWOW64;
	if (IsWow64Process(GetCurrentProcess(), &bIsWOW64))
	{
		LoadString(hInstance, bIsWOW64 ? IDS_APP_TITLE64 : IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	}
	else
	{
		LoadString(hInstance, BUILD(IDS_APP_TITLE), szTitle, MAX_LOADSTRING);
	}
	LoadString(hInstance, IDC_MENUTOOLS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, SW_HIDE))
	{
		return FALSE;
	}

#ifndef _WIN64
	// Create tray icon
	TrayIcon tray(hWnd);
	tray.SetCallBackMessage(MT_TRAY_MESSAGE);
	uTrayId = tray.Show();
	if (!uTrayId)
	{
		// TODO: L10n
		MessageBox(hWnd, _T("Failed to create tray icon!"), szTitle, MB_OK);
		return FALSE;
	}

	// Hide it...
	if (startup.flags & Startup::HIDE_TRAY)
	{
		tray.Hide();
	}
#endif

	// Install wide hooks
	Hooks hooks;
	if (!hooks.Install())
	{
		// TODO: L10n
		MessageBox(hWnd, _T("Failed to install hooks!"), szTitle, MB_OK);
		return FALSE;
	}

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MENUTOOLS));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_MENUTOOLS);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 0, 0, NULL, NULL, NULL, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
#ifndef _WIN64
		// Notify icon message
	case MT_TRAY_MESSAGE:
	{
		// Taskbar icon id
		if (wParam == uTrayId)
		{
			// Message
			switch (lParam)
			{
			case WM_RBUTTONDOWN:
			{
				POINT pt;
				GetCursorPos(&pt);

				SetForegroundWindow(hWnd);

				HMENU hMenu = GetSubMenu(GetMenu(hWnd), 0);
				TrackPopupMenuEx(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, hWnd, NULL);
				PostMessage(hWnd, WM_NULL, 0, 0);
			}
			}
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
#endif
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
