#include "stdafx.h"
#include "Hooks.h"

Hooks::Hooks()
	: hhkCallWndProc(NULL)
	, hhkGetMessage(NULL)
	, hhkCallKeyboardMsg(NULL)
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


	// GetKeyboardProc function
	HOOKPROC hkCallKeyboardMsg = (HOOKPROC)GetProcAddress(hModDLL, MT_HOOK_PROC_KYB);
	if (!hkCallKeyboardMsg)
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

	// Set hook on Keyboard
	hhkCallKeyboardMsg = SetWindowsHookEx(WH_KEYBOARD, hkCallKeyboardMsg, hModDLL, NULL);
	if (!hhkCallKeyboardMsg)
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

	// SUnset hook on Keyboard
	if (!UnhookWindowsHookEx(hhkCallKeyboardMsg))
	{
		bRetn = FALSE;
	}

	return bRetn;
}