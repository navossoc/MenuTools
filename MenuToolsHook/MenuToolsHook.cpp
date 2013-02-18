// MenuToolsHook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "MenuCommon/Defines.h"
#include "MenuCommon/Log.h"

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

			switch (sMsg->message)
			{
			case WM_CONTEXTMENU:
			case WM_INITMENU:
			case WM_INITMENUPOPUP:
			case WM_GETSYSMENU:
				{
					// Install custom menus
					//InsertMenu(sMsg->hwnd);
				}
				break;
			case WM_SYSCOMMAND:
				{
					LOGMESSAGE("S: WM_SYSCOMMAND(%08X, %08X)", sMsg->hwnd, sMsg->wParam);
					/*
					if(WndProcMenu(sMsg->hwnd, sMsg->wParam))
					{
						return 0;
					}
					*/
				}
				break;
			}
		}
		break;
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
			MSG* pMsg = (MSG*) lParam;

			switch(pMsg->message)
			{
			case WM_CONTEXTMENU:
			case WM_INITMENU:
			case WM_INITMENUPOPUP:
			case WM_GETSYSMENU:
				{
					// Install custom menus
					//InsertMenu(pMsg->hwnd);
				}
				break;
			case WM_SYSCOMMAND:
				{
					LOGMESSAGE("P: WM_SYSCOMMAND(%08X, %08X)", pMsg->hwnd, pMsg->wParam);
					if(wParam == PM_REMOVE)
					{
						/*
						if(WndProcMenu(pMsg->hwnd, pMsg->wParam))
						{
							return 0;
						}
						*/
					}
				}
				break;
			}
		}
		break;
	}

	return CallNextHookEx(NULL, code, wParam, lParam);
}