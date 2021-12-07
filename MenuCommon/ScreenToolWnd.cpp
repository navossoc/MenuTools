#include "stdafx.h"
#include "ScreenToolWnd.h"

#include <string>


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
		wcl.lpfnWndProc = DefWindowProc;

		wndClass = &wcl;
		RegisterClassW(wndClass);
	}

	_hWnd = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_TOOLWINDOW,// Optional window styles.
		CLASS_NAME.c_str(),                     // Window class
		L"Learn to Program Windows",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		100, 100, 500, 250,

		NULL,       // Parent window    
		NULL,       // Menu
		hInst,     // Instance handle
		NULL        // Additional application data
	);
	if (_hWnd)
	{
		::ShowWindow(_hWnd, SW_SHOW | SW_NORMAL);
	}
}

ScreenToolWnd::Impl::~Impl()
{
	//SendMessage(_hWnd, WM_CLOSE, 0, 0);
	DestroyWindow(_hWnd);
}
