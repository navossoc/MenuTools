#include "stdafx.h"
#include "ScreenToolWnd.h"

#include <string>
#include <format>
#include <sstream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <functional>
#include <map>
#include <limits>
#include <numeric>
#include <windowsx.h>

using namespace std::placeholders;
using std::wstring;
wstring wm_to_wstring(UINT msg);

struct Screen
{
	size_t scr; // screen number
	int x, y, w, h; // left, top, width, height in pixels
};

struct WinPos {
	int scr; // screen 0 = all, 1 = first, 2 = second, ...
	wstring n; // name
	int x, y, w, h; // left, top, width, height in % from screen
};


struct ScreenToolWnd::Impl
{
	Impl(HINSTANCE hInst, HWND hParent, UINT message, WPARAM wParam, LPARAM lParam);
	~Impl();

	HWND _hWnd;
	RECT _currentPreviewScr = {0}, _currentPreviewPos = {0};
	std::vector<RECT> _realScrRects, _previwScrRects, _realPosRects, _previewPosRects;
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	template<typename It, typename Ct>
	inline It NextPos(It& it, Ct& posRects);
	template<typename It, typename Ct>
	inline It PreviousPos(It& it, Ct& posRects);

	RECT Calculate(WinPos& wp, Screen& s);

	static std::map<HWND, ScreenToolWnd::Impl*> hWndToImplMap;
	static LRESULT CALLBACK ToolWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

std::map<HWND, ScreenToolWnd::Impl*> ScreenToolWnd::Impl::hWndToImplMap;

ScreenToolWnd::ScreenToolWnd() = default;
ScreenToolWnd::~ScreenToolWnd() = default;

HWND ScreenToolWnd::GetHwnd()
{
	return this->_pImpl->_hWnd;
}

LRESULT ScreenToolWnd::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return _pImpl->WndProc(hWnd, message, wParam,lParam);
}

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
	MONITORINFOEX mi = { 0 };
	mi.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMon, &mi);
	mInfos.push_back(mi.rcWork);
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


template<typename It, typename Ct>
inline It ScreenToolWnd::Impl::NextPos(It& it, Ct& posRects)
{
	if (++it == posRects.end())
		it = posRects.begin();
	return it;
}

template<typename It, typename Ct>
inline It ScreenToolWnd::Impl::PreviousPos(It& it, Ct& posRects)
{
	if (it == posRects.begin())
		it = --posRects.end();
	if (--it == posRects.begin())
		it = --posRects.end();
	return it;
}


LRESULT ScreenToolWnd::Impl::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	OutputDebugString(std::format(L"Message: {}\n", wm_to_wstring(message)).c_str());
	LRESULT res = 0;

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hWnd, &pt);

	LONG ix = GetSystemMetrics(SM_CXBORDER) * 2;
	LONG iy = GetSystemMetrics(SM_CXBORDER) * 2;

	if (auto it = std::ranges::find_if(_previwScrRects, [&pt](RECT& r) {return PtInRect(&r, pt); }); it != _previwScrRects.end())
	{
		if (!EqualRect(&*it, &_currentPreviewScr))
			InvalidateRect(hWnd, &_currentPreviewScr, FALSE);
		_currentPreviewScr = *it;
		InvalidateRect(hWnd, &_currentPreviewScr, FALSE);
	}

	switch (message)
	{	
	case WM_MOUSEWHEEL:
	{
		//POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		//ScreenToClient(hWnd, &pt);

		auto posRects = std::ranges::filter_view(_previewPosRects, [pt](RECT r) {
				bool res = PtInRect(&r, pt);
				return res;
			});
		for (RECT r : posRects)
		{
			OutputDebugString(std::format(L"PosRects: Left {}, Top {}, Right {}, Bottom {}\n", r.left, r.top, r.right, r.bottom).c_str());
		}
		int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (auto it = std::ranges::find_if(posRects, [this](RECT& r) {
				bool res = EqualRect(&_currentPreviewPos, &r);
				return res;
			}); it != posRects.end())
		{
			if (delta < 0)
				it = NextPos(it, posRects);
			else if (delta > 0)
				it = PreviousPos(it, posRects);
			_currentPreviewPos = *it;
			auto r = _currentPreviewPos;
			OutputDebugString(std::format(L"MouseWheel, CurPos: Left {}, Top {}, Right {}, Bottom {}\n", r.left, r.top, r.right, r.bottom).c_str());
		}
		OutputDebugString(std::format(L"MouseWheel, Delta: {}, X: {}, Y: {}\n", delta, pt.x, pt.y).c_str());
		break;
	}

	case WM_LBUTTONDOWN:
	{
		if (auto scr_it = std::ranges::find_if(_previwScrRects, [this](RECT& r) { return EqualRect(&_currentPreviewScr, &r); }); scr_it != _previwScrRects.end())
		{
			if (auto pos_it = std::ranges::find_if(_previewPosRects, [this](RECT& r) {return EqualRect(&r, &_currentPreviewPos); }); pos_it != _previewPosRects.end())
			{
				size_t i1 = std::distance(_previewPosRects.begin(), pos_it);
				size_t i2 = std::distance(_previwScrRects.begin(), scr_it);
				if (i1 < _realPosRects.size() && i2 < _realScrRects.size()) 
				{
					RECT r = _realPosRects[i1];
					RECT scr = _realScrRects[i2];

					HWND hParent = GetParent(hWnd);
					SetWindowPos(hParent, HWND_NOTOPMOST, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_SHOWWINDOW);
				}
			}
		}

		DestroyWindow(hWnd);
		break;
	}

	case WM_MOUSEMOVE:
	{
		if (auto it = std::ranges::find_if(
			_previewPosRects.rbegin(),
			_previewPosRects.rend(),
			[pt](RECT& r) {
				bool res = PtInRect(&r, pt);
				return res;
			}); it != _previewPosRects.rend())
		{
			_currentPreviewPos = *it;
			auto cp = _currentPreviewPos;
			OutputDebugString(std::format(L"MouseMove, CurPos: Left {}, Top {}, Right {}, Bottom {}\n", cp.left, cp.top, cp.right, cp.bottom).c_str());
		}
		break;
	}

	case WM_PAINT:
	{
		//InflateRect(&currentPart, -ix * 4, -iy * 4);

		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);

		for (RECT sr : _previwScrRects) // screen rect		
		{
			InflateRect(&sr, -ix, -iy);
			FillRect(hDC, &sr, GetSysColorBrush(COLOR_ACTIVEBORDER));
			FrameRect(hDC, &sr, GetSysColorBrush(COLOR_HOTLIGHT));			
		}

		for (RECT pr : _previewPosRects) // part rect	
		{
			//InflateRect(&pr, -ix * 4, -iy * 4);
			FillRect(hDC, &pr, GetSysColorBrush(COLOR_INACTIVECAPTION));
			FrameRect(hDC, &pr, GetSysColorBrush(COLOR_HOTLIGHT));
			//std::wostringstream os;
			//os << L"x: " << pr.left << L" y: " << pr.top << L" w: " << pr.right - pr.left << L" h: " << pr.bottom - pr.top;
			//DrawText(hDC, os.str().c_str(), -1, &pr, DT_LEFT | DT_TOP | DT_WORDBREAK);
		}

		FillRect(hDC, &_currentPreviewPos, GetSysColorBrush(COLOR_ACTIVECAPTION));
		FrameRect(hDC, &_currentPreviewPos, GetSysColorBrush(COLOR_HOTLIGHT));

		EndPaint(hWnd, &ps);
		break;
	}

	case WM_KEYDOWN: 
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(hWnd);
			break;
		}
		else if (wParam == VK_DOWN || wParam == VK_UP)
		{
			if (auto it = std::ranges::find_if(_previewPosRects, [this](RECT& r) {
				bool res = EqualRect(&_currentPreviewPos, &r);
				return res;
				}); it != _previewPosRects.end())
			{
				if (wParam == VK_DOWN)
					it = NextPos(it, _previewPosRects);
				else
					it = PreviousPos(it, _previewPosRects);
				_currentPreviewPos = *it;
			}
		}
		else if (wParam == VK_RETURN)
		{
			if (auto scr_it = std::ranges::find_if(_previwScrRects, [this](RECT& r) { return EqualRect(&_currentPreviewScr, &r); }); scr_it != _previwScrRects.end())
			{
				if (auto pos_it = std::ranges::find_if(_previewPosRects, [this](RECT& r) {return EqualRect(&r, &_currentPreviewPos); }); pos_it != _previewPosRects.end())
				{
					size_t i1 = std::distance(_previewPosRects.begin(), pos_it);
					size_t i2 = std::distance(_previwScrRects.begin(), scr_it);
					if (i1 < _realPosRects.size() && i2 < _realScrRects.size())
					{
						RECT r = _realPosRects[i1];
						RECT scr = _realScrRects[i2];

						HWND hParent = GetParent(hWnd);
						SetWindowPos(hParent, HWND_NOTOPMOST, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_SHOWWINDOW);
					}
				}
			}

			DestroyWindow(hWnd);
		}
		else
		{
			wchar_t strDataToSend[32];
			wsprintf(strDataToSend, L"%c", wParam);
			MessageBox(NULL, strDataToSend, L"keyselected", MB_OK);
		}
		break;
	}

	return  DefWindowProc(hWnd, message, wParam, lParam);
}



RECT ScreenToolWnd::Impl::Calculate(WinPos& wp, Screen& s)
{
	RECT r = { 
		static_cast<LONG>(s.x + wp.x * (s.w / 100.0)),
		static_cast<LONG>(s.y + wp.y * (s.h / 100.0)),
	};
	r.right = static_cast<LONG>(r.left + wp.w * (s.w / 100.0));
	r.bottom = static_cast<LONG>(r.top + wp.h * (s.h / 100.0));
	return r;
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
	//for (RECT r : mInfos)
	//{
	//	_realScrRects.push_back(r);
	//	OffsetRect(&r, r.right - r.left, 0);
	//	_realScrRects.push_back(r);
	//}
	_realScrRects = mInfos;	

	//RECT wa = { 0 };
	//SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0);

	LONG ox = 0, oy = 0; // offset

	for (RECT& sr : _realScrRects) // screen rect
	{
		//LONG F = 1;
		//sr.right = ((sr.right - sr.left) + sr.left) / F;
		//sr.bottom = ((sr.bottom - sr.top) + sr.top) / F;
		//sr.left = sr.left / F;
		//sr.top = sr.top / F;
		ox = std::min(sr.left, ox);
		oy = std::min(sr.top, oy);
	}


	RECT wr = { 0 }; // window rect
	for (RECT& sr : _realScrRects)
	{
		OffsetRect(&sr, -ox, -oy);

		wr.left = std::min( sr.left, wr.left );
		wr.top = std::min( sr.top, wr.top );
		wr.right = std::max( sr.right, wr.right );
		wr.bottom = std::max( sr.bottom, wr.bottom );
	}

	WinPos winPositions[] = {
		{ 0, L"Big Center", 7, 7, 90, 90 },
		{ 0, L"Small Center", 20, 20, 60, 60 },

		{ 1, L"Left Half", 3, 3, 46, 94 },
		{ 1, L"Right Half", 52, 3, 46, 94 },
		{ 2, L"Top Half", 3, 3, 94, 46 },
		{ 2, L"Bottom Half", 52, 3, 94, 46 },

		{ 0, L"Top Left", 3, 3, 42, 42 },
		{ 0, L"Bottom Left", 3, 56, 42, 42 },
		{ 0, L"Top Right", 56, 3, 42, 42 },
		{ 0, L"Bottom Right", 56, 56, 42, 42 }
	};

	for (size_t i = 0; i < _realScrRects.size(); ++i)
	{
		RECT& sr = _realScrRects[i];
		Screen s = {
			i + 1,
			sr.left,
			sr.top,
			sr.right - sr.left,
			sr.bottom - sr.top
		};

		for (WinPos& wp : winPositions) {
			if (wp.scr > 0 && wp.scr != s.scr)
				continue;

			_realPosRects.push_back(Calculate(wp, s));
		}
	}

	_previwScrRects.clear();
	_previewPosRects.clear();
	std::ranges::transform(_realScrRects, std::back_inserter(_previwScrRects),
		[](RECT& r) {return ScaleRect(r, F); });

	std::ranges::transform(_realPosRects, std::back_inserter(_previewPosRects),
		[](RECT& r) {return ScaleRect(r, F); });

	// after all calculations, reset the offset in case of more than one screen with different resolution 
	for (RECT& bpr : _realPosRects)
	{
		OffsetRect(&bpr, ox, oy);
	}

	wr = ScaleRect(wr, F);
	//wr.bottom += wr.bottom - wr.top;

	//InflateRect(&wr, 0, wr.bottom - wr.top);

	LONG w = (wr.right - wr.left) + GetSystemMetrics(SM_CXBORDER) * 2;
	LONG h = (wr.bottom - wr.top) + GetSystemMetrics(SM_CYBORDER) * 2;

	POINT pt;
	GetCursorPos(&pt);
	OffsetRect(&wr, pt.x - (w / 2), pt.y + 10);

	_hWnd = CreateWindowEx(
		WS_EX_TOPMOST, // | WS_EX_TOOLWINDOW,// Optional window styles.
		CLASS_NAME.c_str(),                     // Window class
		L"Pick a rectangle",    // Window text
		WS_POPUP | WS_VISIBLE | WS_BORDER,           
		//WS_OVERLAPPEDWINDOW, // Window style

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
		::ShowWindow(_hWnd, SW_SHOW);
		//SetActiveWindow(_hWnd);
	}
}

ScreenToolWnd::Impl::~Impl()
{
	hWndToImplMap.erase(_hWnd);
	//SendMessage(_hWnd, WM_CLOSE, 0, 0);
	DestroyWindow(_hWnd);
}


std::wstring wm_to_wstring(UINT msg)
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
		return std::format(L"{:#08x}",msg);
	}
}
