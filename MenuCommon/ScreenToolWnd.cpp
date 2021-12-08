#include "stdafx.h"
#include "ScreenToolWnd.h"

#include <string>
#include <vector>
#include <ranges>
#include <algorithm>

struct ScreenToolWnd::Impl
{
    Impl(HINSTANCE hInst, HWND hParent, UINT message, WPARAM wParam, LPARAM lParam);
    ~Impl();

    HWND _hWnd;
};

ScreenToolWnd::ScreenToolWnd() = default;
ScreenToolWnd::~ScreenToolWnd() = default;

ScreenToolWnd::Ptr ScreenToolWnd::ShowWindow(HINSTANCE hInst, HWND hParent, UINT message, WPARAM wParam, LPARAM lParam)
{
    Ptr ptr = std::make_unique<ScreenToolWnd>();
    ptr->_pImpl = std::make_unique<Impl>(hInst, hParent, message, wParam, lParam);
    return ptr;
}
namespace
{
    const std::wstring CLASS_NAME = L"ScreenToolWnd";
    WNDCLASSW* wndClass = nullptr;
}

BOOL ScreenToolWnd::IsScreenToolWnd(HWND hWnd)
{
	static wchar_t className[100] = { L'\0' };
	GetClassName(hWnd, className, 99);
	return CLASS_NAME.compare(className) == 0;
} 

BOOL Monitorenumproc(HMONITOR hMon, HDC hDC, LPRECT pRECT, LPARAM lParam)
{
	std::vector<RECT>& mInfos = *(std::vector<RECT>*)lParam;
	//MONITORINFO mi = { 0 };
	//GetMonitorInfo(hMon, &mi);
	mInfos.push_back(*pRECT);
	return TRUE;
}

LRESULT CALLBACK ToolWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = 0;
	switch (message)
	{
	case WM_LBUTTONDOWN:
		DestroyWindow(hWnd);
		break;

	case WM_MOUSEMOVE:
	{
		RECT r;
		GetClientRect(hWnd, &r);
		InvalidateRect(hWnd, &r, FALSE);
		break;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		RECT r;
		GetClientRect(hWnd, &r);
		InflateRect(&r, -2, -2);

		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hWnd, &pt);

		RECT quart = {r.left, r.top, (r.right - r.left) / 2 - 1, (r.bottom - r.top) / 2 - 1 };
		
		FillRect(hdc, &quart,GetSysColorBrush(PtInRect(&quart, pt) ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION));

		OffsetRect(&quart, (r.right - r.left) / 2 + 2, 0);
		FillRect(hdc, &quart,GetSysColorBrush(PtInRect(&quart, pt) ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION));

		OffsetRect(&quart, 0, (r.bottom - r.top) / 2 + 2);
		FillRect(hdc, &quart,GetSysColorBrush(PtInRect(&quart, pt) ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION));

		OffsetRect(&quart, -((r.right - r.left) / 2 + 2), 0);
		FillRect(hdc, &quart,GetSysColorBrush(PtInRect(&quart, pt) ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION));

		EndPaint(hWnd, &ps);
		break;
	}
	}

	return  DefWindowProc(hWnd, message, wParam, lParam);
}


ScreenToolWnd::Impl::Impl(HINSTANCE hInst, HWND hParent, UINT message, WPARAM wParam, LPARAM lParam)
	:_hWnd(nullptr)
{
	if (wndClass == nullptr)
	{
		static WNDCLASSW wcl = { 0 };
		wcl.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcl.hInstance = hInst;
		wcl.lpszClassName = CLASS_NAME.c_str();
		wcl.lpfnWndProc = ToolWndProc;

		wndClass = &wcl;
		RegisterClassW(wndClass);
	}

	std::vector<RECT> mInfos;
	EnumDisplayMonitors(NULL, NULL, Monitorenumproc, (LPARAM)&mInfos);
	
	

	RECT disp = { 0 };
	for (RECT& r : mInfos)
	{
		disp.left = min( r.left, disp.left );
		disp.top = min( r.top, disp.top );
		disp.right = max( r.right, disp.right );
		disp.bottom = max( r.bottom, disp.bottom );
	}

	static const LONG F = 6;
	LONG w = (disp.right - disp.left) / F;
	LONG h = (disp.bottom - disp.top) / F;

	POINT pt;
	GetCursorPos(&pt);

	disp.left = pt.x - (w / 2);
	disp.right = disp.left + w;
	disp.top = pt.y;
	disp.bottom = disp.top + h + GetSystemMetrics(SM_CYCAPTION);

	_hWnd = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_TOOLWINDOW,// Optional window styles.
		CLASS_NAME.c_str(),                     // Window class
		L"Learn to Program Windows",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		disp.left, disp.top, w, h,

		hParent,       // Parent window    
		NULL,       // Menu
		hInst,     // Instance handle
		NULL        // Additional application data
	);
	if (_hWnd)
	{
		::ShowWindow(_hWnd, SW_SHOW | SW_NORMAL);
		SetActiveWindow(_hWnd);
	}
}

ScreenToolWnd::Impl::~Impl()
{
	//SendMessage(_hWnd, WM_CLOSE, 0, 0);
	DestroyWindow(_hWnd);
}
