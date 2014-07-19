#pragma once

// Debug
#define MT_DEBUG_ONLY_X86					FALSE
#define MT_DEBUG_ONLY_X64					FALSE

// 64 bits
#ifdef _WIN64
#define BUILD(x)							x ## 64

// 32 bits
#elif _WIN32
#define BUILD(x)							x
#endif

// Strings
#define MT_DLL_NAME							_T("MenuToolsHook.dll")
#define MT_DLL_NAME64						_T("MenuToolsHook64.dll")
#define MT_EXE_NAME							_T("MenuTools.exe")
#define MT_EXE_NAME64						_T("MenuTools64.exe")
#define MT_JOB_NAME							_T("MenuToolsJob")

// Hook
#define MT_HOOK_PROC_CWP					"CallWndProc"
#define MT_HOOK_PROC_GMP					"GetMsgProc"

// Hook -> Messages
#define MT_HOOK_MSG_QUIT					RegisterWindowMessage(_T("MenuToolsQuit"))
#define MT_HOOK_MSG_TRAY					WM_USER + 0x210

// Menu
#define MT_MENU_PRIORITY					WM_USER + 0x2100
#define MT_MENU_TRANSPARENCY				WM_USER + 0x2200
#define MT_MENU_ALWAYS_ON_TOP				WM_USER + 0x2010
#define MT_MENU_MINIMIZE_TO_TRAY			WM_USER + 0x2020
#define MT_MENU_SEPARATOR					WM_USER + 0x2030

// Menu -> Priority
#define MT_MENU_PRIORITY_REALTIME			WM_USER + 0x2110
#define MT_MENU_PRIORITY_HIGH				WM_USER + 0x2120
#define MT_MENU_PRIORITY_ABOVE_NORMAL		WM_USER + 0x2130
#define MT_MENU_PRIORITY_NORMAL				WM_USER + 0x2140
#define MT_MENU_PRIORITY_BELOW_NORMAL		WM_USER + 0x2150
#define MT_MENU_PRIORITY_LOW				WM_USER + 0x2160

// Menu -> Transparency
#define MT_MENU_TRANSPARENCY_0				WM_USER + 0x2210
#define MT_MENU_TRANSPARENCY_10				WM_USER + 0x2220
#define MT_MENU_TRANSPARENCY_20				WM_USER + 0x2230
#define MT_MENU_TRANSPARENCY_30				WM_USER + 0x2240
#define MT_MENU_TRANSPARENCY_40				WM_USER + 0x2250
#define MT_MENU_TRANSPARENCY_50				WM_USER + 0x2260
#define MT_MENU_TRANSPARENCY_60				WM_USER + 0x2270
#define MT_MENU_TRANSPARENCY_70				WM_USER + 0x2280
#define MT_MENU_TRANSPARENCY_80				WM_USER + 0x2290
#define MT_MENU_TRANSPARENCY_90				WM_USER + 0x22A0
#define MT_MENU_TRANSPARENCY_100			WM_USER + 0x22B0

// Tray
#define MT_TRAY_MESSAGE						WM_USER + 0x200