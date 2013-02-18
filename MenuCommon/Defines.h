#pragma once

// Debug
#define MT_DEBUG_ONLY_X86		FALSE
#define MT_DEBUG_ONLY_X64		FALSE

// 64 bits
#ifdef _WIN64
#define BUILD(x)				x ## 64

// 32 bits
#elif _WIN32
#define BUILD(x)				x
#endif

// Strings
#define MT_DLL_NAME				_T("MenuToolsHook.dll")
#define MT_DLL_NAME64			_T("MenuToolsHook64.dll")
#define MT_EXE_NAME				_T("MenuTools.exe")
#define MT_EXE_NAME64			_T("MenuTools64.exe")
#define MT_JOB_NAME				_T("MenuToolsJob")

// Hook
#define MT_HOOK_PROC_CWP		"CallWndProc"
#define MT_HOOK_PROC_GMP		"GetMsgProc"

// Tray
#define MT_TRAY_ID				1
#define MT_TRAY_MESSAGE			WM_USER + 100