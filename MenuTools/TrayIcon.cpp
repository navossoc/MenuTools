#include "stdafx.h"
#include "TrayIcon.h"

TrayIcon::TrayIcon(TCHAR* szTitle, HICON hIcon)
{
	ZeroMemory(&nid, sizeof(NOTIFYICONDATA));

	nid.cbSize = sizeof(NOTIFYICONDATA);
	//nid.hWnd;
	nid.uID = TRAYICON_ID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = TRAYICON_MESSAGE;
	nid.hIcon = hIcon;
	StringCchCopyEx(nid.szTip, ARRAYSIZE(nid.szTip), szTitle, NULL, NULL, STRSAFE_IGNORE_NULLS);
	nid.uVersion = NOTIFYICON_VERSION;
}

TrayIcon::~TrayIcon()
{
	Hide();	
}

BOOL TrayIcon::SetIcon(HICON hIcon)
{
	nid.hIcon = hIcon;
	return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

BOOL TrayIcon::SetIcon(HINSTANCE hInstance, ULONG uIcon)
{
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(uIcon));
	return SetIcon(hIcon);
}

BOOL TrayIcon::SetTooltip(TCHAR* szTitle)
{
	StringCchCopyEx(nid.szTip, ARRAYSIZE(nid.szTip), szTitle, NULL, NULL, STRSAFE_IGNORE_NULLS);
	return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

BOOL TrayIcon::Show(HWND hWnd)
{
	nid.hWnd = hWnd;
	return Shell_NotifyIcon(NIM_ADD, &nid);
}

BOOL TrayIcon::Hide()
{
	return Shell_NotifyIcon(NIM_DELETE, &nid);
}