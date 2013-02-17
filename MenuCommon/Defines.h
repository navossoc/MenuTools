#pragma once

// 64 bits
#ifdef _WIN64
#define BUILD(x)		x ## 64

// 32 bits
#elif _WIN32
#define BUILD(x)		x
#endif

// Strings
#define MT_DLL_NAME		_T("MenuToolsHook.dll")
#define MT_DLL_NAME64	_T("MenuToolsHook64.dll")
#define MT_EXE_NAME		_T("MenuTools.exe")
#define MT_EXE_NAME64	_T("MenuTools64.exe")
#define MT_JOB_NAME		_T("MenuToolsJob")