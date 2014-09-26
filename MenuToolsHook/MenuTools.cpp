#include "stdafx.h"
#include "MenuTools.h"

#include "MenuCommon/TrayIcon.h"

// Window information
LONG wndOldWidth = -1;
LONG wndOldHeight = -1;

BOOL MenuTools::Install(HWND hWnd)
{
	// Visible window
	if (!IsWindowVisible(hWnd))
	{
		return FALSE;
	}

	HMENU hMenuSystem = GetSystemMenu(hWnd, FALSE);

	if (!IsMenuItem(hMenuSystem, MT_MENU_PRIORITY))
	{
		HMENU hMenuPriority = CreateMenu();
		AppendMenu(hMenuPriority, MF_BYCOMMAND | MF_ENABLED | MF_STRING, MT_MENU_PRIORITY_REALTIME, _T("&Realtime"));
		AppendMenu(hMenuPriority, MF_BYCOMMAND | MF_ENABLED | MF_STRING, MT_MENU_PRIORITY_HIGH, _T("&High"));
		AppendMenu(hMenuPriority, MF_BYCOMMAND | MF_ENABLED | MF_STRING, MT_MENU_PRIORITY_ABOVE_NORMAL, _T("&Above normal"));
		AppendMenu(hMenuPriority, MF_BYCOMMAND | MF_ENABLED | MF_STRING, MT_MENU_PRIORITY_NORMAL, _T("&Normal"));
		AppendMenu(hMenuPriority, MF_BYCOMMAND | MF_ENABLED | MF_STRING, MT_MENU_PRIORITY_BELOW_NORMAL, _T("&Below normal"));
		AppendMenu(hMenuPriority, MF_BYCOMMAND | MF_ENABLED | MF_STRING, MT_MENU_PRIORITY_LOW, _T("&Low"));
		InsertSubMenu(hMenuSystem, hMenuPriority, SC_CLOSE, MF_BYCOMMAND | MF_POPUP, MT_MENU_PRIORITY, _T("&Priority"));
	}

	if (!IsMenuItem(hMenuSystem, MT_MENU_TRANSPARENCY))
	{
		HMENU hMenuTransparency = CreateMenu();
		AppendMenu(hMenuTransparency, MF_BYCOMMAND | MF_ENABLED | MF_STRING, MT_MENU_TRANSPARENCY_0, _T("&0% (Opaque)"));
		AppendMenu(hMenuTransparency, MF_BYCOMMAND | MF_ENABLED | MF_STRING, MT_MENU_TRANSPARENCY_10, _T("&10%"));
		AppendMenu(hMenuTransparency, MF_BYCOMMAND | MF_ENABLED | MF_STRING, MT_MENU_TRANSPARENCY_20, _T("&20%"));
		AppendMenu(hMenuTransparency, MF_BYCOMMAND | MF_ENABLED | MF_STRING, MT_MENU_TRANSPARENCY_30, _T("&30%"));
		AppendMenu(hMenuTransparency, MF_BYCOMMAND | MF_ENABLED | MF_STRING, MT_MENU_TRANSPARENCY_40, _T("&40%"));
		AppendMenu(hMenuTransparency, MF_BYCOMMAND | MF_ENABLED | MF_STRING, MT_MENU_TRANSPARENCY_50, _T("&50%"));
		InsertSubMenu(hMenuSystem, hMenuTransparency, SC_CLOSE, MF_BYCOMMAND | MF_POPUP, MT_MENU_TRANSPARENCY, _T("&Transparency"));
	}

	if (!IsMenuItem(hMenuSystem, MT_MENU_ALWAYS_ON_TOP))
	{
		InsertMenu(hMenuSystem, SC_CLOSE, MF_BYCOMMAND | MF_STRING, MT_MENU_ALWAYS_ON_TOP, _T("&Always on Top"));
	}

	if (!IsMenuItem(hMenuSystem, MT_MENU_MINIMIZE_TO_TRAY))
	{
		InsertMenu(hMenuSystem, SC_CLOSE, MF_BYCOMMAND | MF_STRING, MT_MENU_MINIMIZE_TO_TRAY, _T("Minimi&ze to Tray"));

	}

	if (!IsMenuItem(hMenuSystem, MT_MENU_SEPARATOR))
	{
		InsertMenu(hMenuSystem, SC_CLOSE, MF_BYCOMMAND | MF_SEPARATOR, MT_MENU_SEPARATOR, NULL);
	}

	return TRUE;
}

BOOL MenuTools::Uninstall(HWND hWnd)
{
	BOOL bSuccess = TRUE;

	HMENU hMenuSystem = GetSystemMenu(hWnd, FALSE);

	// Delete Menu Tools
	if (!DeleteMenu(hMenuSystem, MT_MENU_PRIORITY, MF_BYCOMMAND))
	{
		bSuccess = FALSE;
	}
	if (!DeleteMenu(hMenuSystem, MT_MENU_TRANSPARENCY, MF_BYCOMMAND))
	{
		bSuccess = FALSE;
	}
	if (!DeleteMenu(hMenuSystem, MT_MENU_ALWAYS_ON_TOP, MF_BYCOMMAND))
	{
		bSuccess = FALSE;
	}
	if (!DeleteMenu(hMenuSystem, MT_MENU_MINIMIZE_TO_TRAY, MF_BYCOMMAND))
	{
		bSuccess = FALSE;
	}
	if (!DeleteMenu(hMenuSystem, MT_MENU_SEPARATOR, MF_BYCOMMAND))
	{
		bSuccess = FALSE;
	}

	return bSuccess;
}

VOID MenuTools::Status(HWND hWnd)
{
	HMENU hMenuSystem = GetSystemMenu(hWnd, FALSE);

	// Priority
	HANDLE hProcess = GetCurrentProcess();

	DWORD dwPriority = GetPriorityClass(hProcess);
	switch (dwPriority)
	{
	case REALTIME_PRIORITY_CLASS:
	{
		CheckMenuRadioItem(hMenuSystem, MT_MENU_PRIORITY_REALTIME, MT_MENU_PRIORITY_LOW, MT_MENU_PRIORITY_REALTIME, MF_BYCOMMAND);
		break;
	}
	case HIGH_PRIORITY_CLASS:
	{
		CheckMenuRadioItem(hMenuSystem, MT_MENU_PRIORITY_REALTIME, MT_MENU_PRIORITY_LOW, MT_MENU_PRIORITY_HIGH, MF_BYCOMMAND);
		break;
	}
	case ABOVE_NORMAL_PRIORITY_CLASS:
	{
		CheckMenuRadioItem(hMenuSystem, MT_MENU_PRIORITY_REALTIME, MT_MENU_PRIORITY_LOW, MT_MENU_PRIORITY_ABOVE_NORMAL, MF_BYCOMMAND);
		break;
	}
	case NORMAL_PRIORITY_CLASS:
	default:
	{
		CheckMenuRadioItem(hMenuSystem, MT_MENU_PRIORITY_REALTIME, MT_MENU_PRIORITY_LOW, MT_MENU_PRIORITY_NORMAL, MF_BYCOMMAND);
		break;
	}
	case BELOW_NORMAL_PRIORITY_CLASS:
	{
		CheckMenuRadioItem(hMenuSystem, MT_MENU_PRIORITY_REALTIME, MT_MENU_PRIORITY_LOW, MT_MENU_PRIORITY_BELOW_NORMAL, MF_BYCOMMAND);
		break;
	}
	case IDLE_PRIORITY_CLASS:
	{
		CheckMenuRadioItem(hMenuSystem, MT_MENU_PRIORITY_REALTIME, MT_MENU_PRIORITY_LOW, MT_MENU_PRIORITY_LOW, MF_BYCOMMAND);
		break;
	}
	}

	// Transparency
	UINT uLevel = MT_MENU_TRANSPARENCY_0;
	COLORREF crKey;
	BYTE bAlpha;
	DWORD dwFlags;
	if (GetLayeredWindowAttributes(hWnd, &crKey, &bAlpha, &dwFlags))
	{
		if (crKey == 0 && dwFlags == LWA_ALPHA)
		{
			if (bAlpha > 229)
			{
				uLevel = MT_MENU_TRANSPARENCY_0;
			}
			else if (bAlpha > 204)
			{
				uLevel = MT_MENU_TRANSPARENCY_10;
			}
			else if (bAlpha > 178)
			{
				uLevel = MT_MENU_TRANSPARENCY_20;
			}
			else if (bAlpha > 153)
			{
				uLevel = MT_MENU_TRANSPARENCY_30;
			}
			else if (bAlpha > 127)
			{
				uLevel = MT_MENU_TRANSPARENCY_40;
			}
			else
			{
				uLevel = MT_MENU_TRANSPARENCY_50;
			}
		}
	}
	CheckMenuRadioItem(hMenuSystem, MT_MENU_TRANSPARENCY_0, MT_MENU_TRANSPARENCY_100, uLevel, MF_BYCOMMAND);

	// Always on Top
	if (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
	{
		CheckMenuItem(hMenuSystem, MT_MENU_ALWAYS_ON_TOP, MF_BYCOMMAND | MF_CHECKED);
	}
	else
	{
		CheckMenuItem(hMenuSystem, MT_MENU_ALWAYS_ON_TOP, MF_BYCOMMAND | MF_UNCHECKED);
	}

	// Minimize to Tray
	if (mTrays.count(hWnd))
	{
		CheckMenuItem(hMenuSystem, MT_MENU_MINIMIZE_TO_TRAY, MF_BYCOMMAND | MF_CHECKED);
	}
	else
	{
		CheckMenuItem(hMenuSystem, MT_MENU_MINIMIZE_TO_TRAY, MF_BYCOMMAND | MF_UNCHECKED);
	}

	return;
}

BOOL MenuTools::TrayProc(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

	// Tray Icon
	switch (lParam)
	{
		// Restore
	case WM_LBUTTONDBLCLK:
	{
		// Hide tray icon
		if (mTrays.count(hWnd))
		{
			mTrays.erase(hWnd);
		}

		ShowWindow(hWnd, SW_SHOW);
		SetForegroundWindow(hWnd);

		return TRUE;
	}
	}

	return FALSE;
}

BOOL MenuTools::WndProc(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	int wmId = wParam & 0xFFF0;

	// Handle menu messages
	switch (wmId)
	{
		// Priority
	case MT_MENU_PRIORITY_REALTIME:
	case MT_MENU_PRIORITY_HIGH:
	case MT_MENU_PRIORITY_ABOVE_NORMAL:
	case MT_MENU_PRIORITY_NORMAL:
	case MT_MENU_PRIORITY_BELOW_NORMAL:
	case MT_MENU_PRIORITY_LOW:
	{
		HANDLE hProcess = GetCurrentProcess();
		switch (wmId)
		{
		case MT_MENU_PRIORITY_REALTIME:
			SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);
			break;
		case MT_MENU_PRIORITY_HIGH:
			SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS);
			break;
		case MT_MENU_PRIORITY_ABOVE_NORMAL:
			SetPriorityClass(hProcess, ABOVE_NORMAL_PRIORITY_CLASS);
			break;
		default:
		case MT_MENU_PRIORITY_NORMAL:
			SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);
			break;
		case MT_MENU_PRIORITY_BELOW_NORMAL:
			SetPriorityClass(hProcess, BELOW_NORMAL_PRIORITY_CLASS);
			break;
		case MT_MENU_PRIORITY_LOW:
			SetPriorityClass(hProcess, IDLE_PRIORITY_CLASS);
			break;
		}

		return TRUE;
	}
		// Transparency
	case MT_MENU_TRANSPARENCY_0:
	case MT_MENU_TRANSPARENCY_10:
	case MT_MENU_TRANSPARENCY_20:
	case MT_MENU_TRANSPARENCY_30:
	case MT_MENU_TRANSPARENCY_40:
	case MT_MENU_TRANSPARENCY_50:
	{
		BYTE level;
		switch (wmId)
		{
		default:
		case MT_MENU_TRANSPARENCY_0:
			level = 0;
			break;
		case MT_MENU_TRANSPARENCY_10:
			level = 10;
			break;
		case MT_MENU_TRANSPARENCY_20:
			level = 20;
			break;
		case MT_MENU_TRANSPARENCY_30:
			level = 30;
			break;
		case MT_MENU_TRANSPARENCY_40:
			level = 40;
			break;
		case MT_MENU_TRANSPARENCY_50:
			level = 50;
			break;
		}

		if (level)
		{
			// Set WS_EX_LAYERED on this window 
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, GetWindowLongPtr(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			// Make this window % alpha
			SetLayeredWindowAttributes(hWnd, NULL, (255 * (100 - level)) / 100, LWA_ALPHA);
		}
		else
		{
			// Remove WS_EX_LAYERED from this window styles
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, GetWindowLongPtr(hWnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
			// Ask the window and its children to repaint
			RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
		}

		return TRUE;
	}
		// Always On Top
	case MT_MENU_ALWAYS_ON_TOP:
	{
		if (!(GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST))
		{
			// Set
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		else
		{
			// Remove
			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		return TRUE;
	}
		// Minimize to Tray
	case MT_MENU_MINIMIZE_TO_TRAY:
	{
		// Check if tray is already add
		if (mTrays.count(hWnd))
		{
			// Destroy tray icon
			mTrays.erase(hWnd);

			// Restore window
			ShowWindow(hWnd, SW_SHOW);
			SetForegroundWindow(hWnd);
		}
		else
		{
			// Insert
			mTrays.insert(Tray_Pair(hWnd, TrayIcon(hWnd)));
			// Show tray icon
			if (mTrays.at(hWnd).Show())
			{
				// Hide window
				ShowWindow(hWnd, SW_HIDE);
			}
			else
			{
				// Destroy tray icon
				mTrays.erase(hWnd);
			}
		}

		return TRUE;
	}
	}

	return FALSE;
}

// Helpers
BOOL InsertSubMenu(HMENU hMenu, HMENU hSubMenu, UINT uPosition, UINT uFlags, UINT uIDNewItem, LPCWSTR lpNewItem)
{
	if (InsertMenu(hMenu, uPosition, uFlags, (UINT_PTR)hSubMenu, lpNewItem))
	{
		MENUITEMINFO mmi;
		ZeroMemory(&mmi, sizeof(MENUITEMINFO));
		mmi.cbSize = sizeof(MENUITEMINFO);
		mmi.fMask = MIIM_ID;
		mmi.wID = uIDNewItem;

		return SetMenuItemInfo(hMenu, (UINT)hSubMenu, FALSE, &mmi);
	}

	return FALSE;
}

BOOL IsMenuItem(HMENU hMenu, UINT item)
{
	MENUITEMINFO mmi;
	ZeroMemory(&mmi, sizeof(MENUITEMINFO));
	mmi.cbSize = sizeof(MENUITEMINFO);
	mmi.fMask = MIIM_ID;

	return GetMenuItemInfo(hMenu, item, FALSE, &mmi);
}