// MenuToolsHook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MenuTools.h"

#include "MenuCommon/TrayIcon.h"

#define WM_GETSYSMENU						0x313

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
			if(MenuTools::Install(hWnd))
			{
				// Update menu
				MenuTools::Status(hWnd);
				return TRUE;
			}
		}
		break;
	case MT_HOOK_MSG_TRAY:
		{
			// Process tray messages
			MenuTools::TrayProc(hWnd, wParam, lParam);
			return TRUE;
		}
		break;
	case WM_COMMAND:
	case WM_SYSCOMMAND:
		{
			// Process menu messages
			MenuTools::WndProc(hWnd, wParam, lParam);
			return TRUE;
		}
		break;
	default:
		{
			// Quit message
			if(message == MT_HOOK_MSG_QUIT)
			{
				// Restore previous windows
				for(Tray_It it = mTrays.begin(); it != mTrays.end(); it++)
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
	switch(nCode)
	{
	case HC_ACTION:
		{
			CWPSTRUCT* sMsg = (CWPSTRUCT*) lParam;
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
			if(wParam == PM_REMOVE)
			{
				MSG* pMsg = (MSG*) lParam;
				HookProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
			}
		}
	}

	return CallNextHookEx(NULL, code, wParam, lParam);
}