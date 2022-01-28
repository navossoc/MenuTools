// MenuToolsHook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <windowsx.h>
#include <sstream>

#include "MenuTools.h"
#include "MenuCommon/Logger.h"
#include "MenuCommon/TrayIcon.h"
#include "MenuCommon/ScreenToolWnd.h"

#define WM_GETSYSMENU						0x313

extern HINSTANCE hInst;
namespace {
	POINT lastButtonDown = { 0 };
	WPARAM lastHitTest = 0;
	//ScreenToolWnd::Ptr pWnd;
}

// Process messages
LRESULT CALLBACK HookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
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
	case WM_NCLBUTTONDOWN: {
		if (wParam == HTCAPTION) 
		{
			lastButtonDown = {
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam)
			};
			std::wostringstream os;
			os << L"Button Down: " << lastButtonDown.x << L", " << lastButtonDown.y << std::endl;
			OutputDebugString(os.str().c_str());
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
		//lastButtonDown = { 0, 0 };
		ScreenToolWnd::pWnd.reset();
		POINT bu = {
			GET_X_LPARAM(lParam),
			GET_Y_LPARAM(lParam)
		};
		//ClientToScreen(hWnd, &bu);
		std::wostringstream os;
		os << L"Button DblClk: " << bu.x << L", " << bu.y << std::endl;
		OutputDebugString(os.str().c_str());
		break;
	}

	case WM_LBUTTONUP: 
	{
		POINT& lbd = lastButtonDown;
		POINT bu = {
			GET_X_LPARAM(lParam),
			GET_Y_LPARAM(lParam)
		};
		ClientToScreen(hWnd, &bu);
		std::wostringstream os;
		os << L"Button Up: " << bu.x << L", " << bu.y << std::endl;
		OutputDebugString(os.str().c_str());
		if (std::abs(lbd.x - bu.x) > 2 || std::abs(lbd.y - bu.y) > 2)
		{
			//ScreenToolWnd::pWnd.reset();
			return FALSE;
		}

		if (is_one_of((signed)wParam, MK_CONTROL, MK_SHIFT))
		{
			LONG diff = wParam == MK_CONTROL ? 10 : -10;

			POINT pt = { 0 };
			GetCursorPos(&pt);
			//ScreenToClient(hWnd, &pt);
			SetCursorPos(pt.x, pt.y - diff);

			//HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
			RECT r = { 0 };
			GetWindowRect(hWnd, &r);
			InflateRect(&r, diff, diff);
			SetWindowPos(hWnd, HWND_NOTOPMOST, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_SHOWWINDOW);
		}
		else
		{
			wParam = SendMessage(hWnd, WM_NCHITTEST, wParam, lParam);

			// Title bar
			if(!ScreenToolWnd::IsScreenToolWnd(hWnd))
			{
				log_debug(L"ScreenToolWnd::pWnd: {}", (void*)ScreenToolWnd::pWnd.get());
				if (ScreenToolWnd::pWnd)
				{
					ScreenToolWnd::pWnd.reset();
				}
				else
				{
					ScreenToolWnd::pWnd = ScreenToolWnd::ShowWindow(hInst, hWnd, message, wParam, MAKELPARAM(bu.x, bu.y));
				}
			}
		}

		return FALSE;
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
			return TRUE;
		}
	}
	}

	return FALSE;
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
			BOOL upFlag = (HIWORD(lParam) & KF_UP) == KF_UP;  // transition-state flag, 1 on keyup
			ScreenToolWnd::pWnd->WndProc(ScreenToolWnd::pWnd->GetHwnd(), upFlag ? WM_KEYUP:WM_KEYDOWN, wParam, lParam);
		}
	}
	}

	return CallNextHookEx(NULL, code, wParam, lParam);
}

HOOKPROC hkCallWndProc = CallWndProc;
HOOKPROC hkGetMsgProc = GetMsgProc;
HOOKPROC hkCallKeyboardMsg = CallKeyboardMsg;
