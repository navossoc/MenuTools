// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

HINSTANCE hInst;                                // current instance

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	//UNREFERENCED_PARAMETER(hModule);
	hInst = hModule;
	UNREFERENCED_PARAMETER(ul_reason_for_call);
	UNREFERENCED_PARAMETER(lpReserved);

	return TRUE;
}

