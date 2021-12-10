#include "stdafx.h"
#include "ScreenToolWnd.h"

#include <string>
#include <sstream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <functional>
#include <map>

using namespace std::placeholders;

struct ScreenToolWnd::Impl
{
	Impl(HINSTANCE hInst, HWND hParent, UINT message, WPARAM wParam, LPARAM lParam);
	~Impl();

	HWND _hWnd;
	std::vector<RECT> _bigScrRects, _smallScrRects, _bigPartRects, _smallPartRects;
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static std::map<HWND, ScreenToolWnd::Impl*> hWndToImplMap;
	static LRESULT CALLBACK ToolWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

std::map<HWND, ScreenToolWnd::Impl*> ScreenToolWnd::Impl::hWndToImplMap;

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

	static const FLOAT F = 0.1f; // factor
}

BOOL ScreenToolWnd::IsScreenToolWnd(HWND hWnd)
{
	static wchar_t className[100] = { L'\0' };
	GetClassName(hWnd, className, 99);
	return CLASS_NAME.compare(className) == 0;
} 

LRESULT ScreenToolWnd::Impl::ToolWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ScreenToolWnd::Impl* pImpl = hWndToImplMap[hWnd];
	if (pImpl)
		return pImpl->WndProc(hWnd, message, wParam, lParam);

	return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL Monitorenumproc(HMONITOR hMon, HDC hDC, LPRECT pRECT, LPARAM lParam)
{
	std::vector<RECT>& mInfos = *(std::vector<RECT>*)lParam;
	//MONITORINFO mi = { 0 };
	//GetMonitorInfo(hMon, &mi);
	mInfos.push_back(*pRECT);
	return TRUE;
}

RECT ScaleRect(RECT& in, FLOAT f)
{
	RECT r = { 0 };

	r.right = static_cast<LONG>(((FLOAT)((in.right - in.left) + in.left)) * f);
	r.bottom = static_cast<LONG>(((FLOAT)((in.bottom - in.top) + in.top)) * f);
	r.left = static_cast<LONG>(((FLOAT)in.left) * f);
	r.top = static_cast<LONG>(((FLOAT)in.top) * f);

	return r;
}

LRESULT ScreenToolWnd::Impl::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = 0;

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hWnd, &pt);

	LONG ix = GetSystemMetrics(SM_CXBORDER) * 2;
	LONG iy = GetSystemMetrics(SM_CXBORDER) * 2;

	RECT currentPart = { 0 };
	auto cp_it = std::ranges::find_if(
		_smallPartRects.rbegin(), _smallPartRects.rend(),
		[&pt, ix, iy](RECT r) {
			InflateRect(&r, -ix * 4, -iy * 4);
			return PtInRect(&r, pt); 
		});

	if (cp_it != _smallPartRects.rend())
		currentPart = *cp_it;

	auto scr_it = std::ranges::find_if(_smallScrRects, [&pt](RECT& r) {return PtInRect(&r, pt); });

	switch (message)
	{
	case WM_LBUTTONDOWN:
	{
		if (scr_it != _smallScrRects.end())
		{
			auto part_it = std::ranges::find_if(_smallPartRects,
				[&currentPart](RECT& r) {return EqualRect(&r, &currentPart); });
			size_t i1 = std::distance(_smallPartRects.begin(), part_it);
			size_t i2 = std::distance(_smallScrRects.begin(), scr_it);
			RECT r = _bigPartRects[i1], scr = _bigScrRects[i2];

			if (r.left == scr.left || r.top == scr.top || r.right == scr.right || r.bottom == scr.bottom)
				InflateRect(&r, (scr.right - scr.left) / 100 * 15, (scr.bottom - scr.top) / 100 * 15);

			HWND hParent = GetParent(hWnd);
			SetWindowPos(hParent, HWND_NOTOPMOST, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_SHOWWINDOW);
		}

		DestroyWindow(hWnd);
		break;
	}

	case WM_MOUSEMOVE:
	{
		if(scr_it != _smallScrRects.end())
			InvalidateRect(hWnd, &*scr_it, FALSE);
		break;
	}

	case WM_PAINT:
	{
		InflateRect(&currentPart, -ix * 4, -iy * 4);

		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);

		for (RECT sr : _smallScrRects) // screen rect		
		{
			InflateRect(&sr, -ix, -iy);
			FillRect(hDC, &sr, GetSysColorBrush(COLOR_ACTIVEBORDER));
			FrameRect(hDC, &sr, GetSysColorBrush(COLOR_HOTLIGHT));			
		}

		for (RECT pr : _smallPartRects) // part rect	
		{
			InflateRect(&pr, -ix * 4, -iy * 4);
			FillRect(hDC, &pr, GetSysColorBrush(EqualRect(&pr, &currentPart) ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION));
			FrameRect(hDC, &pr, GetSysColorBrush(COLOR_HOTLIGHT));
			//std::wostringstream os;
			//os << L"x: " << pr.left << L" y: " << pr.top << L" w: " << pr.right - pr.left << L" h: " << pr.bottom - pr.top;
			//DrawText(hDC, os.str().c_str(), -1, &pr, DT_LEFT | DT_TOP | DT_WORDBREAK);
		}

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
	_bigScrRects = mInfos;	

	LONG ox = 0, oy = 0; // offset

	for (RECT& sr : _bigScrRects) // screen rect
	{
		LONG F = 1;
		sr.right = ((sr.right - sr.left) + sr.left) / F;
		sr.bottom = ((sr.bottom - sr.top) + sr.top) / F;
		sr.left = sr.left / F;
		sr.top = sr.top / F;
		ox = min(sr.left, ox);
		oy = min(sr.top, oy);
	}


	RECT wr = { 0 }; // window rect
	for (RECT& sr : _bigScrRects)
	{
		OffsetRect(&sr, -ox, -oy);

		wr.left = min( sr.left, wr.left );
		wr.top = min( sr.top, wr.top );
		wr.right = max( sr.right, wr.right );
		wr.bottom = max( sr.bottom, wr.bottom );
	}

	for (RECT& sr : _bigScrRects)
	{
		LONG sw = (sr.right - sr.left);
		LONG sh = (sr.bottom - sr.top);

		if (sw > sh)
		{
			// left
			RECT left = { sr.left, sr.top, sr.left + sw / 2, sr.bottom };
			InflateRect(&left, 2, 2);
			_bigPartRects.push_back(left);

			// right
			RECT right = { sr.left + sw / 2, sr.top, sr.left + sr.left + sw, sr.bottom };
			_bigPartRects.push_back(right);
		}
		else
		{
			// top
			RECT top = { sr.left, sr.top, sr.left + sw, sr.bottom - sh / 2 };
			InflateRect(&top, 2, 2);
			_bigPartRects.push_back(top);

			// bottom
			RECT bottom = { sr.left, sr.top + sh / 2, sr.left + sw, sr.bottom };
			_bigPartRects.push_back(bottom);
		}

		// top left
		RECT pr = { sr.left + 2, sr.top, sr.left + sw / 2, sr.top + sh / 2 };
		_bigPartRects.push_back(pr);
		// top right
		OffsetRect(&pr, sw / 2 - 2, 0);
		_bigPartRects.push_back(pr);
		// bottom right
		OffsetRect(&pr, 0, sh / 2);
		_bigPartRects.push_back(pr);
		// bottom left
		OffsetRect(&pr, -(sw / 2 ), 0);
		_bigPartRects.push_back(pr);

		// small center
		RECT sc = sr;
		InflateRect(&sc, -(sw / 8), -(sh / 8));
		_bigPartRects.push_back(sc);

		// big center
		RECT bc = sr;
		InflateRect(&bc, -(sw / 4), -(sh / 4));
		_bigPartRects.push_back(bc);
	}

	_smallScrRects.clear();
	std::ranges::transform(_bigScrRects, std::back_inserter(_smallScrRects),
		[](RECT& r) {return ScaleRect(r, F); });

	_smallPartRects.clear();
	std::ranges::transform(_bigPartRects, std::back_inserter(_smallPartRects),
		[](RECT& r) {return ScaleRect(r, F); });

	wr = ScaleRect(wr, F);

	LONG w = (wr.right - wr.left) + GetSystemMetrics(SM_CXBORDER) * 2;
	LONG h = (wr.bottom - wr.top) + GetSystemMetrics(SM_CYBORDER) * 2;

	POINT pt;
	GetCursorPos(&pt);
	OffsetRect(&wr, pt.x - (w / 2), pt.y);

	_hWnd = CreateWindowEx(
		WS_EX_TOPMOST ,//| WS_EX_TOOLWINDOW Optional window styles.
		CLASS_NAME.c_str(),                     // Window class
		L"Learn to Program Windows",    // Window text
		WS_POPUP | WS_VISIBLE | WS_BORDER,            // WS_OVERLAPPEDWINDOW Window style

		// Size and position
		//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		wr.left, wr.top, w, h,

		hParent,       // Parent window    
		NULL,       // Menu
		hInst,     // Instance handle
		NULL        // Additional application data
	);
	if (_hWnd)
	{
		hWndToImplMap[_hWnd] = this;
		//::ShowWindow(_hWnd, SW_SHOW);
		//SetActiveWindow(_hWnd);
	}
}

ScreenToolWnd::Impl::~Impl()
{
	hWndToImplMap.erase(_hWnd);
	//SendMessage(_hWnd, WM_CLOSE, 0, 0);
	DestroyWindow(_hWnd);
}
