#include "stdafx.h"
#include "Hooks.h"

Hooks::Hooks()
{
	//
}

Hooks::~Hooks()
{
	Uninstall();
}

BOOL Hooks::Install()
{
	// Load hook DLL
	HMODULE hModDLL = LoadLibrary(BUILD(MT_DLL_NAME));
	if (!hModDLL)
	{
		return FALSE;
	}

	// CallWndProc function
	HOOKPROC hkCallWndProc = (HOOKPROC)GetProcAddress(hModDLL, MT_HOOK_PROC_CWP);
	if (!hkCallWndProc)
	{
		return FALSE;
	}

	// GetMsgProc function
	HOOKPROC hkGetMsgProc = (HOOKPROC)GetProcAddress(hModDLL, MT_HOOK_PROC_GMP);
	if (!hkGetMsgProc)
	{
		return FALSE;
	}

	// Set hook on CallWndProc
	hhkCallWndProc = SetWindowsHookEx(WH_CALLWNDPROC, hkCallWndProc, hModDLL, NULL);
	if (!hhkCallWndProc)
	{
		return FALSE;
	}

	// Set hook on GetMessage
	hhkGetMessage = SetWindowsHookEx(WH_GETMESSAGE, hkGetMsgProc, hModDLL, NULL);
	if (!hhkGetMessage)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL Hooks::Uninstall()
{
	// Send a especial message to remove menus
	SendMessage(HWND_BROADCAST, MT_HOOK_MSG_QUIT, NULL, NULL);

	BOOL bRetn = TRUE;

	// Unset the CallWndProc hook
	if (!UnhookWindowsHookEx(hhkCallWndProc))
	{
		bRetn = FALSE;
	}

	// Unset the GetMessage
	if (!UnhookWindowsHookEx(hhkGetMessage))
	{
		bRetn = FALSE;
	}

	return bRetn;
}