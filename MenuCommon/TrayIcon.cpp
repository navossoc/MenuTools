#include "stdafx.h"
#include "TrayIcon.h"

Tray_Map mTrays;
UINT TrayIcon::uId = 0;

TrayIcon::TrayIcon(HWND hWnd)
{
	ZeroMemory(&nid, sizeof(NOTIFYICONDATA));

	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = uId ? ++uId : uId = 1;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = TRAYICON_MESSAGE;
	nid.hIcon = GetWindowIcon(hWnd);
	GetWindowText(hWnd, nid.szTip, 128);
	nid.uVersion = NOTIFYICON_VERSION;
}

TrayIcon::~TrayIcon()
{
	Hide();
}

void TrayIcon::SetCallBackMessage(UINT uMessage)
{
	nid.uCallbackMessage = uMessage;
}

UINT TrayIcon::Show()
{
	if (Shell_NotifyIcon(NIM_ADD, &nid))
	{
		return uId;
	}
	return NULL;
}

BOOL TrayIcon::Hide()
{
	return Shell_NotifyIcon(NIM_DELETE, &nid);
}

// Private
HICON TrayIcon::GetWindowIcon(HWND hWnd)
{
	HICON hIcon;
	hIcon = (HICON)SendMessage(hWnd, WM_GETICON, ICON_SMALL, NULL);
	if (hIcon)
	{
		return hIcon;
	}

	hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICONSM);
	if (hIcon)
	{
		return hIcon;
	}

	hIcon = (HICON)SendMessage(hWnd, WM_GETICON, ICON_BIG, NULL);
	if (hIcon)
	{
		return hIcon;
	}

	hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICON);
	if (hIcon)
	{
		return hIcon;
	}

	return hIcon;
}