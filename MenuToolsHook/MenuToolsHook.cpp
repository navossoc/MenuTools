// MenuToolsHook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <windowsx.h>
#include <sstream>
#include <chrono>
#include <future>

#include "MenuTools.h"
#include "MenuCommon/Logger.h"
#include "MenuCommon/TrayIcon.h"
#include "MenuCommon/ScreenToolWnd.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#define WM_GETSYSMENU						0x313

extern HINSTANCE hInst;

void InflateWnd(const LONG& diff, const HWND& hWnd);

// Process messages
LRESULT CALLBACK HookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	using namespace std::chrono;

	static high_resolution_clock::time_point last_lbutton_down;
	static POINT lastButtonDown = { 0 };
	static WPARAM lastHitTest = 0;
	static bool dblClick = false;

	//static const UINT_PTR TIMER_ID = 13578;

	//static UINT_PTR timer = NULL;
	//if(!timer)
	//	timer = SetTimer(hWnd, TIMER_ID, 300, (TIMERPROC)NULL);


	switch (message)
	{
	//case WM_TIMER:
	//{
	//	if (wParam == TIMER_ID) 
	//	{
	//		if (HIBYTE(GetAsyncKeyState(VK_LSHIFT)) && HIBYTE(GetAsyncKeyState(VK_LCONTROL)) && HIBYTE(GetAsyncKeyState(0x41)))
	//		{
	//			log_debug(L"Thats i!: {}", message);
	//			RECT wr;
	//			GetWindowRect(hWnd, &wr);
	//			int caption = GetSystemMetrics(SM_CYCAPTION);
	//			POINT pt = {
	//				wr.left + ((wr.right - wr.left) / 2),
	//				wr.top + (caption / 2)
	//			};
	//			PostMessage(hWnd, WM_SHOW_WIN_POS, wParam, MAKELPARAM(pt.x, pt.y));
	//		}
	//		return TRUE;
	//	}
	//	break;
	//}
	//case WM_KEYDOWN:
	//{
	//	log_debug(L"Hello");
	//}

	case WM_CONTEXTMENU:
	case WM_INITMENU:
	case WM_INITMENUPOPUP:
	case WM_GETSYSMENU:
	{
		// Install custom menus
		if (MenuTools::Install(hWnd))
		{
			// Update menu
			MenuTools::Status(hWnd);
			return TRUE;
		}
		break;
	}
	case MT_HOOK_MSG_TRAY:
	{
		// Process tray messages
		MenuTools::TrayProc(hWnd, wParam, lParam);
		return TRUE;
		break;
	}
	case WM_COMMAND:
	case WM_SYSCOMMAND:
	{
		// Process menu messages
		MenuTools::WndProc(hWnd, wParam, lParam);
		return TRUE;
		break;
	}
	//case WM_LBUTTONDOWN: 
	case WM_NCLBUTTONDOWN: 
	{
		dblClick = false;
		last_lbutton_down = std::chrono::high_resolution_clock::now();

		POINT	bd = {										// button down
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam)
			};
		ClientToScreen(hWnd, &bd);
		auto ch = GetSystemMetrics(SM_CYCAPTION); // caption height
		RECT cr;												// caption rect
		GetWindowRect(hWnd, &cr);
		cr.bottom = cr.top + ch;
		//if (!PtInRect(&cr, bd))
		if (wParam == HTCAPTION)
		{
			lastButtonDown = {
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam)
			};
			log_debug(L"LButtonDown: {}, {}", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			if (ScreenToolWnd::pWnd)
			{
				ScreenToolWnd::pWnd.reset();
				return TRUE;						
			}

			//BYTE ks = HIBYTE(GetAsyncKeyState(VK_CONTROL) + GetAsyncKeyState(VK_SHIFT));
			//if(!ks)
			//if (!is_one_of((signed)wParam, MK_CONTROL, MK_SHIFT))
			//{
			POINT lbd = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			std::thread t([hWnd, wParam, lbd]()
				{
					auto toWait = GetDoubleClickTime() / 2;
					Sleep(toWait);


					if (!HIBYTE(GetAsyncKeyState(VK_LBUTTON))) {
						POINT pt;
						GetCursorPos(&pt);
						const LONG XTOL = GetSystemMetrics(SM_CXDOUBLECLK) / 2;
						const LONG YTOL = GetSystemMetrics(SM_CYDOUBLECLK) / 2;
						RECT tolerance = { lbd.x - XTOL, lbd.y - YTOL, lbd.x + XTOL, lbd.y + YTOL };
						log_debug(L"LButton is up: {}, {}", pt.x, pt.y);

						if (PtInRect(&tolerance, pt))
						{
							if (HIBYTE(GetAsyncKeyState(VK_CONTROL)) || HIBYTE(GetAsyncKeyState(VK_SHIFT)))
							{
								LONG diff = HIBYTE(GetAsyncKeyState(VK_CONTROL)) ? 10 : -10;
								InflateWnd(diff, hWnd);
							}
							else
							{
								log_debug(L"ScreenToolWnd::pWnd: {}", (void*)ScreenToolWnd::pWnd.get());
								{
									if (!dblClick)
									{
										PostMessage(hWnd, WM_SHOW_WIN_POS, wParam, MAKELPARAM(pt.x, pt.y));
									}
								}
							}

						}

					}
				}
			);
			t.detach();

				//TCHAR buffer[MAX_PATH] = { 0 };
				//TCHAR* out;
				//DWORD bufSize = sizeof(buffer) / sizeof(*buffer);

				//// Get the fully-qualified path of the executable
				//if (GetModuleFileName(NULL, buffer, bufSize) < bufSize)
				//{
				//	// now buffer = "c:\whatever\yourexecutable.exe"

				//	// Go to the beginning of the file name
				//	out = PathFindFileName(buffer);
				//	// now out = "yourexecutable.exe"

				//	// Set the dot before the extension to 0 (terminate the string there)
				//	*(PathFindExtension(out)) = 0;
				//	// now out = "yourexecutable"

				//	std::wstring exeName = out;
				//	std::transform(exeName.begin(), exeName.end(), exeName.begin(),
				//		[](wchar_t c) { return std::tolower(c); });
				//	//if (exeName == L"code") 
				//	{
				//	}
				//}
			//}

		}
		else
		{
			lastButtonDown = { 0, 0 };
		}
		return TRUE;
		break;
	}
	case WM_LBUTTONDBLCLK:
	case WM_NCLBUTTONDBLCLK:
	{
		dblClick = true;
		//ScreenToolWnd::pWnd.reset();
		auto now = std::chrono::high_resolution_clock::now();
		POINT bu = {
			GET_X_LPARAM(lParam),
			GET_Y_LPARAM(lParam)
		};
		std::chrono::duration<double, std::milli> millis = now - last_lbutton_down;
		//double dbl_click = GetDoubleClickTime();

		log_debug(L"LButtonDblClick: {}, {}, duration: {}", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), millis.count());
		break;
	}

	case WM_LBUTTONUP: 
	{
		auto now = std::chrono::high_resolution_clock::now();


	//	//POINT& lbd = lastButtonDown;
	//	POINT bu = {										// button up
	//		GET_X_LPARAM(lParam),
	//		GET_Y_LPARAM(lParam)
	//	};
	//	ClientToScreen(hWnd, &bu);

	//	std::chrono::duration<double, std::milli> millis = now - last_lbutton_down;
	//	//double dbl_click = GetDoubleClickTime();

	//	log_debug(L"LButtonUp: {}, {}, duration: {}", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), millis.count());

	//	//if (millis.count() <= dbl_click)
	//		//return FALSE;

	//	//static const LONG TOL = 1;
	//	//RECT tolerance = { lbd.x - TOL, lbd.y - TOL, lbd.x + TOL, lbd.y + TOL };
	//	//if (!PtInRect(&tolerance, bu))
	//	//{
	//	//	return FALSE;
	//	//}
	//	auto ch = GetSystemMetrics(SM_CYCAPTION); // caption height
	//	RECT cr;												// caption rect
	//	GetWindowRect(hWnd, &cr);
	//	cr.bottom = cr.top + ch;
	//	if(!PtInRect(&cr, bu))
	//	//if (SendMessage(hWnd, WM_NCHITTEST, wParam, MAKELPARAM(bu.x, bu.y)) != HTCAPTION)
	//		return FALSE;

	//	if (is_one_of((signed)wParam, MK_CONTROL, MK_SHIFT))
	//	{
	//		LONG diff = wParam == MK_CONTROL ? 10 : -10;
	//		InflateWnd(diff, hWnd);
	//	}
	//	else
	//	{
	//		//wParam = SendMessage(hWnd, WM_NCHITTEST, wParam, lParam);

	//		//// Title bar
	//		//if(!ScreenToolWnd::IsScreenToolWnd(hWnd))
	//		//{
	//		//	log_debug(L"ScreenToolWnd::pWnd: {}", (void*)ScreenToolWnd::pWnd.get());
	//		//	if (ScreenToolWnd::pWnd)
	//		//	{
	//		//		ScreenToolWnd::pWnd.reset();
	//		//	}
	//		//	else
	//		//	{
	//		//		/*PostMessage(hWnd, WM_SHOW_WIN_POS, wParam, MAKELPARAM(bu.x, bu.y));*/
	//		//		//std::async(std::launch::async, [hWnd, wParam, bu]()
	//		//		std::thread t([hWnd, wParam, bu]()
	//		//			{
	//		//				Sleep(100);
	//		//				if (!dblClick) 
	//		//				{
	//		//					PostMessage(hWnd, WM_SHOW_WIN_POS, wParam, MAKELPARAM(bu.x, bu.y));
	//		//					//ScreenToolWnd::pWnd = ScreenToolWnd::ShowWindow(hInst, hWnd, WM_LBUTTONUP, wParam, MAKELPARAM(bu.x, bu.y));
	//		//				}
	//		//			}
	//		//		);
	//		//		t.detach();
	//		//		//if (SendMessage(hWnd, WM_NCHITTEST, wParam, MAKELPARAM(bu.x, bu.y)) == HTCAPTION)
	//		//		//if (!dblClick)
	//		//		//	ScreenToolWnd::pWnd = ScreenToolWnd::ShowWindow(hInst, hWnd, WM_LBUTTONUP, wParam, MAKELPARAM(bu.x, bu.y));
	//		//	}
	//		//}
	//	}

		return FALSE;
	}

	case WM_SHOW_WIN_POS:
	{
		log_debug(L"ShowWonPos: {}, {}", wParam, lParam);
		ScreenToolWnd::pWnd = ScreenToolWnd::ShowWindow(hInst, hWnd, WM_LBUTTONUP, wParam, lParam);
		break;
	}

	// Roll-up/Unroll (hard coded for now)
	case WM_NCRBUTTONUP:{
		// Title bar
		if (wParam == HTCAPTION)
		{
			// Original window position
			RECT rect = { 0 };
			if (GetWindowRect(hWnd, &rect))
			{
				long width = rect.right - rect.left;
				long height = rect.bottom - rect.top;

				// The minimum tracking height of a window
				int minTrack = GetSystemMetrics(SM_CYMINTRACK);

				// Roll-up
				if (height != minTrack)
				{
					// Save old values
					wndOldWidth = width;
					wndOldHeight = height;
					height = minTrack;
				}
				// Unroll
				else
				{
					height = wndOldHeight;
				}

				// Resize window
				SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING);
			}
			else
			{
				// On error, try to unroll with old values
				if (wndOldWidth != -1 && wndOldHeight != -1)
				{
					SetWindowPos(hWnd, NULL, 0, 0, wndOldWidth, wndOldHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING);
				}
			}
		}
		return TRUE;
	}
	default:
	{
		// Quit message
		if (message == MT_HOOK_MSG_QUIT)
		{
			// Restore previous windows
			for (Tray_It it = mTrays.begin(); it != mTrays.end(); it++)
			{
				ShowWindow(it->first, SW_SHOW);
			}
			// Destroy all tray icons
			mTrays.clear();

			// Uninstall menus
			MenuTools::Uninstall(hWnd);
			//FreeLibraryAndExitThread(hInst, 0);
			return TRUE;
		}
	}
	}

	return FALSE;
}

void InflateWnd(const LONG& diff, const HWND& hWnd)
{
	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	GetWindowPlacement(hWnd, &wp);
	if (wp.showCmd == SW_MAXIMIZE && diff > 0)
		return;

	RECT wr = { 0 };
	GetWindowRect(hWnd, &wr);
	InflateRect(&wr, diff, diff);

	//auto width = r.right - r.left;
	auto height = wr.bottom - wr.top;

	auto captionHeight = GetSystemMetrics(SM_CYCAPTION);
	if (height <= captionHeight)
		return;

	HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = { sizeof(MONITORINFO) };
	GetMonitorInfo(hMon, &mi);
	auto mr = mi.rcWork;
	if (height >= (mr.bottom - mr.top))
	{
		wp.showCmd = SW_MAXIMIZE;
		SetWindowPlacement(hWnd, &wp);
		return;
	}

	POINT pt = { 0 };
	GetCursorPos(&pt);
	//ScreenToClient(hWnd, &pt);
	SetCursorPos(pt.x, pt.y - diff);


	wp.rcNormalPosition = wr;
	wp.showCmd = SW_NORMAL;
	SetWindowPlacement(hWnd, &wp);
	//if(wp.showCmd == SW_MAXIMIZE)
	//ShowWindow(hWnd, SW_NORMAL);
	//int l = 
	//SetWindowPos(hWnd, HWND_NOTOPMOST, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_SHOWWINDOW);
}

// Sent messages
LRESULT CALLBACK CallWndProc(
	_In_  int nCode,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
	)
{
	switch (nCode)
	{
	case HC_ACTION:
	{
		CWPSTRUCT* sMsg = (CWPSTRUCT*)lParam;
		HookProc(sMsg->hwnd, sMsg->message, sMsg->wParam, sMsg->lParam);
	}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Post messages
LRESULT CALLBACK GetMsgProc(
	_In_  int code,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
	)
{
	switch (code)
	{
	case HC_ACTION:
	{
		if (wParam == PM_REMOVE)
		{
			MSG* pMsg = (MSG*)lParam;
			HookProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
		}
	}
	}

	return CallNextHookEx(NULL, code, wParam, lParam);
}

// Keyboard messages
LRESULT CALLBACK CallKeyboardMsg(
	_In_  int code,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
	)
{
	switch (code)
	{
	case HC_ACTION:
	{
		if (ScreenToolWnd::pWnd && wParam == VK_ESCAPE)
		{
			ScreenToolWnd::pWnd.reset();
		}
		if(ScreenToolWnd::pWnd)
		{
			//BOOL upFlag = (HIWORD(lParam) & KF_UP) == KF_UP;  // transition-state flag, 1 on keyup
			//ScreenToolWnd::pWnd->WndProc(ScreenToolWnd::pWnd->GetHwnd(), upFlag ? WM_KEYUP:WM_KEYDOWN, wParam, lParam);
		}
	}
	}

	return CallNextHookEx(NULL, code, wParam, lParam);
}

HOOKPROC hkCallWndProc = CallWndProc;
HOOKPROC hkGetMsgProc = GetMsgProc;
HOOKPROC hkCallKeyboardMsg = CallKeyboardMsg;
