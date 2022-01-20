#pragma once
#include <string>
#include <format>
#include <sstream>
using std::wstring;

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
#define MT_MENU_WIN_POS						WM_USER + 0x2040

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



#ifdef _WIN64
#define TODWORD(l)	((DWORD)( (((UINT_PTR)(l)) & 0xffffffff) ))
//#define TODWORD(l)	((DWORD)( (((UINT_PTR)(l)) & 0xffffffff) == 0xffffffff ) \
//							? (((UINT_PTR)(l)) & 0xffffffff) \
//							: ((DWORD)((((UINT_PTR)(l)) >> 32) & 0xffffffff)) ) 

#else
#define TODWORD(l)           l
#endif

inline wstring wm_to_wstring(UINT msg)
{
	switch (msg)
	{
	case WM_CTLCOLORMSGBOX: return std::format(L"WM_CTLCOLORMSGBOX {:#08x}", WM_CTLCOLORMSGBOX);
	case WM_CTLCOLOREDIT: return std::format(L"WM_CTLCOLOREDIT {:#08x}", WM_CTLCOLOREDIT);
	case WM_CTLCOLORLISTBOX: return std::format(L"WM_CTLCOLORLISTBOX {:#08x}", WM_CTLCOLORLISTBOX);
	case WM_CTLCOLORBTN: return std::format(L"WM_CTLCOLORBTN {:#08x}", WM_CTLCOLORBTN);
	case WM_CTLCOLORDLG: return std::format(L"WM_CTLCOLORDLG {:#08x}", WM_CTLCOLORDLG);
	case WM_CTLCOLORSCROLLBAR: return std::format(L"WM_CTLCOLORSCROLLBAR {:#08x}", WM_CTLCOLORSCROLLBAR);
	case WM_CTLCOLORSTATIC: return std::format(L"WM_CTLCOLORSTATIC {:#08x}", WM_CTLCOLORSTATIC);
	case MN_GETHMENU: return std::format(L"MN_GETHMENU {:#08x}", MN_GETHMENU);
	case WM_MOUSEMOVE: return std::format(L"WM_MOUSEMOVE {:#08x}", WM_MOUSEMOVE);
	case WM_LBUTTONDOWN: return std::format(L"WM_LBUTTONDOWN {:#08x}", WM_LBUTTONDOWN);
	case WM_LBUTTONUP: return std::format(L"WM_LBUTTONUP {:#08x}", WM_LBUTTONUP);
	case WM_LBUTTONDBLCLK: return std::format(L"WM_LBUTTONDBLCLK {:#08x}", WM_LBUTTONDBLCLK);
	case WM_RBUTTONDOWN: return std::format(L"WM_RBUTTONDOWN {:#08x}", WM_RBUTTONDOWN);
	case WM_RBUTTONUP: return std::format(L"WM_RBUTTONUP {:#08x}", WM_RBUTTONUP);
	case WM_RBUTTONDBLCLK: return std::format(L"WM_RBUTTONDBLCLK {:#08x}", WM_RBUTTONDBLCLK);
	case WM_MBUTTONDOWN: return std::format(L"WM_MBUTTONDOWN {:#08x}", WM_MBUTTONDOWN);
	case WM_MBUTTONUP: return std::format(L"WM_MBUTTONUP {:#08x}", WM_MBUTTONUP);
	case WM_MBUTTONDBLCLK: return std::format(L"WM_MBUTTONDBLCLK {:#08x}", WM_MBUTTONDBLCLK);
	case WM_MOUSEWHEEL: return std::format(L"WM_MOUSEWHEEL {:#08x}", WM_MOUSEWHEEL);
	case WM_XBUTTONDOWN: return std::format(L"WM_XBUTTONDOWN {:#08x}", WM_XBUTTONDOWN);
	case WM_XBUTTONUP: return std::format(L"WM_XBUTTONUP {:#08x}", WM_XBUTTONUP);
	case WM_XBUTTONDBLCLK: return std::format(L"WM_XBUTTONDBLCLK {:#08x}", WM_XBUTTONDBLCLK);
	case WM_PARENTNOTIFY: return std::format(L"WM_PARENTNOTIFY {:#08x}", WM_PARENTNOTIFY);
	case WM_ENTERMENULOOP: return std::format(L"WM_ENTERMENULOOP {:#08x}", WM_ENTERMENULOOP);
	case WM_EXITMENULOOP: return std::format(L"WM_EXITMENULOOP {:#08x}", WM_EXITMENULOOP);
	case WM_NEXTMENU: return std::format(L"WM_NEXTMENU {:#08x}", WM_NEXTMENU);
	case WM_SIZING: return std::format(L"WM_SIZING {:#08x}", WM_SIZING);
	case WM_CAPTURECHANGED: return std::format(L"WM_CAPTURECHANGED {:#08x}", WM_CAPTURECHANGED);
	case WM_MOVING: return std::format(L"WM_MOVING {:#08x}", WM_MOVING);
	case WM_POWERBROADCAST: return std::format(L"WM_POWERBROADCAST {:#08x}", WM_POWERBROADCAST);
	default:
		return wstring();
		//	return std::format(L"{:#08x}",msg);
	}
}

template<typename T, typename Tx>
bool is_one_of(T t, Tx&& p1)
{
	return t == p1;
}

template<typename T, typename T1, typename... Tx>
bool is_one_of(T t, T1&& p1, Tx&&... px) {
	return is_one_of(t, p1) || is_one_of(t, px...);
}

template<typename T, typename Tx>
bool is_each_of(T t, Tx&& p1)
{
	return t == p1;
}

template<typename T, typename T1, typename... Tx>
bool is_each_of(T t, T1&& p1, Tx&&... px) {
	return is_each_of(t, p1) && is_each_of(t, px...);
}

/*
template<typename T, typename Tx>
bool is_one_of(T t, Tx p1)
{
	return t == p1;
}
template<typename T, typename Tx, typename P2>
bool is_one_of(T t, Tx p1, P2 p2)
{
	return t == p1 || t == p2;
}
template<typename T, typename Tx, typename P2, typename P3>
bool is_one_of(T t, Tx p1, P2 p2, P3 p3)
{
	return t == p1 || t == p2 || t == p3;
}
template<typename T, typename Tx, typename P2, typename P3, typename P4>
bool is_one_of(T t, Tx p1, P2 p2, P3 p3, P4 p4)
{
	return t == p1 || t == p2 || t == p3 || t == p4;
}
template<typename T, typename Tx, typename P2, typename P3, typename P4, typename P5>
bool is_one_of(T t, Tx p1, P2 p2, P3 p3, P4 p4, P5 p5)
{
	return t == p1 || t == p2 || t == p3 || t == p4 || t == p5;
}
template<typename T, typename Tx, typename P2, typename P3, typename P4, typename P5, typename P6>
bool is_one_of(T t, Tx p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
{
	return t == p1 || t == p2 || t == p3 || t == p4 || t == p5 || t == p6;
}
template<typename T, typename Tx, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
bool is_one_of(T t, Tx p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
{
	return t == p1 || t == p2 || t == p3 || t == p4 || t == p5 || t == p6 || t == p7;
}
template<typename T, typename Tx, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
bool is_one_of(T t, Tx p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
{
	return t == p1 || t == p2 || t == p3 || t == p4 || t == p5 || t == p6 || t == p7 || t == p8;
}
template<typename T, typename Tx, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
bool is_one_of(T t, Tx p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9)
{
	return t == p1 || t == p2 || t == p3 || t == p4 || t == p5 || t == p6 || t == p7 || t == p8 || t == p9;
}
*/