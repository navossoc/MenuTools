#pragma once
#include <memory>

class ScreenToolWnd
{
public:
	using Ptr = std::unique_ptr<ScreenToolWnd>;
	static Ptr ShowWindow(HINSTANCE hInst, HWND hParent, UINT message, WPARAM wParam, LPARAM lParam);
	static BOOL IsScreenToolWnd(HWND hWnd);

	ScreenToolWnd();
	~ScreenToolWnd();

private:
	struct Impl;
	std::unique_ptr<Impl> _pImpl;
	//~ScreenToolWnd();
};