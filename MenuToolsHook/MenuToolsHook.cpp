// MenuToolsHook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

LRESULT CALLBACK CallWndProc(
	_In_  int nCode,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
	)
{
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(
	_In_  int code,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
	)
{
	return CallNextHookEx(NULL, code, wParam, lParam);
}