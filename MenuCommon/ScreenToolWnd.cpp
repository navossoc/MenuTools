#include "stdafx.h"
#include "ScreenToolWnd.h"

#include <vector>
#include <ranges>
#include <algorithm>
#include <functional>
#include <map>
#include <deque>
#include <limits>
#include <numeric>
#include <windowsx.h>

using namespace std::placeholders;
using std::views::filter;

namespace
{
	const std::wstring CLASS_NAME = L"ScreenToolWnd";
	WNDCLASSW* wndClass = nullptr;

	static const FLOAT F = 0.1f; // factor
}
RECT ScaleRect(RECT& in, FLOAT f);

//struct Screen
//{
//	size_t scr; // screen number
//	int x, y, w, h; // left, top, width, height in pixels
//};


struct PositioningCfg {
	int scr; // screen 0 = all, 1 = first, 2 = second, ...
	wstring n; // name
	int x, y, w, h; // left, top, width, height in % from screen
	double f = 1;
};
using PositioningCfgs = std::vector<PositioningCfg>;

struct ScreenWnd;

struct PositioningWnd {
	PositioningWnd(PositioningCfg& pc, ScreenWnd* sw):_posCfg(pc),_scrWnd(*sw) {
		screenRect = calcScreenRect();
		//OffsetRect(&screenRect, -(sw->previewOffset()), 0);
		previewRect = calcPreviewRect();
	}

private:
	PositioningCfg _posCfg;
	ScreenWnd& _scrWnd;

	RECT calcScreenRect();
	RECT calcPreviewRect();

public:
	RECT screenRect;
	RECT previewRect;
	//PropR<RECT> screenRect = { this->calcScreenRect() };
	//PropR<RECT> previewRect = { this->calcPreviewRect() };
};


using PositioningWnds = std::vector<PositioningWnd>;

struct ScreenWnd {
	ScreenWnd(RECT& r, UINT s, UINT po):_scrRect(r), _scrNr(s), _previewOffset(po) {
		_prvRect = ScaleRect(_scrRect, F);
	}

	void Paint(PAINTSTRUCT& ps, HDC hDC);
	
	void push_back(PositioningCfg& wp) { 
		return _posititioningWnds.push_back(PositioningWnd(wp, this));
	}

private:

	UINT _scrNr; ///> Screen Nr
	UINT _previewOffset = 0; ///> Offset of preview, if multiple previews are available
	RECT _prvRect; ///> Preview
	RECT _scrRect; ///> Screen
	PositioningWnds _posititioningWnds; ///> Positioning Windows

	friend struct PositioningWnd;

public:
	bool empty() { return _posititioningWnds.empty(); }

	PropR<UINT, ScreenWnd, &ScreenWnd::_scrNr> nr = { this };
	PropR<UINT, ScreenWnd, &ScreenWnd::_previewOffset> previewOffset = { this };
	PropR<RECT> pr = { [this]() { return _prvRect; } };
	PropR<RECT> sr = { [this]() { return _scrRect; } };
	PropR<LONG> x = { [this]() { return _scrRect.left; } };
	PropR<LONG> y = { [this]() { return _scrRect.top; } };
	PropR<LONG> w = { [this]() { return _scrRect.right - _scrRect.left; } };
	PropR<LONG> h = { [this]() { return _scrRect.bottom - _scrRect.top; } };

	bool hit(POINT pt)
	{
		return PtInRect(&_prvRect, pt);
	}

	PositioningWnd* find(POINT pt)
	{
		using std::ranges::find_if;

		auto &pw = _posititioningWnds;
		if (auto it = find_if(pw.rbegin(), pw.rend(), [pt](PositioningWnd& pw) { return PtInRect(&pw.previewRect, pt); }); it != pw.rend())
		{
			return &*it;
		}

		return nullptr;
	}


};


void ScreenWnd::Paint(PAINTSTRUCT& ps, HDC hDC)
{
	FillRect(hDC, &_prvRect, GetSysColorBrush(COLOR_ACTIVEBORDER));
	FrameRect(hDC, &_prvRect, GetSysColorBrush(COLOR_HOTLIGHT));

	for (PositioningWnd& wp : _posititioningWnds) {
		RECT pr = wp.previewRect;
		FillRect(hDC, &pr, GetSysColorBrush(COLOR_INACTIVECAPTION));
		FrameRect(hDC, &pr, GetSysColorBrush(COLOR_HOTLIGHT));
	}
}

RECT PositioningWnd::calcScreenRect()
{
	PositioningCfg& pc = _posCfg;
	ScreenWnd& sw = _scrWnd;
	RECT r = {
		static_cast<LONG>(sw.x + pc.x * (sw.w / 100.0)),
		static_cast<LONG>(sw.y + pc.y * (sw.h / 100.0)),
	};
	r.right = static_cast<LONG>(r.left + pc.w * (sw.w / 100.0));
	r.bottom = static_cast<LONG>(r.top + pc.h * (sw.h / 100.0));

	return r;

}

RECT PositioningWnd::calcPreviewRect()
{
	PositioningCfg& pc = _posCfg;
	RECT r = calcScreenRect();

	int w = r.right - r.left;
	int h = r.bottom - r.top;

	int ix = static_cast<int>(((w * 1.0) - (w * pc.f)) / 2.0);
	int iy = static_cast<int>(((h * 1.0) - (h * pc.f)) / 2.0);
	InflateRect(&r, -ix, -iy);

	r = ScaleRect(r, F);	
	return r;
}

using ScreenWnds = std::vector<ScreenWnd>;

struct SelectionWnd {
private:
	RECT _rct;
};
using SelectionWnds = std::vector<SelectionWnd>;

extern HINSTANCE hInst;  // current instance

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


struct ScreenToolWnd::Impl
{
	Impl(HINSTANCE hInst, HWND hParent, UINT message, WPARAM wParam, LPARAM lParam);
	~Impl();

	HHOOK _hhkCallKeyboardMsg = 0;
	HWND _hWnd = 0;

	ScreenWnds _screenWnds;
	RECT _toolRect = { 0 };

	//SelectionWnds _selectionWnds;

	POINT _clickPoint = { 0 };
	ScreenWnd* _currentScreenWnd = nullptr;
	PositioningWnd* _currentPosWnd = nullptr;
	//RECT _currentPreviewScr = {0}, _currentPreviewPos = {0};
	//std::vector<RECT> _realScrRects, _previwScrRects, _realPosRects, _previewPosRects;
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	template<typename It, typename Ct>
	inline It NextPos(It it, Ct& posRects);
	template<typename It, typename Ct>
	inline It PreviousPos(It it, Ct& posRects);

	//RECT Calculate(WinPos& wp, Screen& s);

	static std::map<HWND, ScreenToolWnd::Impl*> hWndToImplMap;
	static LRESULT CALLBACK ToolWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

std::map<HWND, ScreenToolWnd::Impl*> ScreenToolWnd::Impl::hWndToImplMap;

ScreenToolWnd::Ptr ScreenToolWnd::pWnd;

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
		wcl.lpfnWndProc = ToolWndProc;

		wndClass = &wcl;
		RegisterClassW(wndClass);
	}
	_clickPoint = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	std::vector<RECT> mInfos;
#ifdef _WIN64
	EnumDisplayMonitors(NULL, NULL, Monitorenumproc, (LPARAM)&mInfos);
#endif
	static UINT NP = 2; // Number of previews
	
	//std::vector<RECT> scrRects;

	PositioningCfgs winPositions = {
		{ 1, L"Left TwoThirds", 3, 3, 62, 94 },
		{ 1, L"Right Third", 68, 3, 30, 94 },
		{ 1, L"Big Center", 7, 7, 90, 90, 0.8 },

		{ 2, L"Left Third", 3, 3, 30, 94 },
		{ 2, L"Right TwoThirds", 36, 3, 62, 94 },
		{ 2, L"Small Center", 20, 20, 60, 60, 0.8 },

		{ 2, L"Left Half", 3, 3, 46, 94, 0.8 },
		{ 2, L"Right Half", 52, 3, 46, 94, 0.8 },

		{ 1, L"Top Left", 3, 3, 42, 42, 0.8 },
		{ 1, L"Top Right", 56, 3, 42, 42, 0.8 },
		{ 1, L"Bottom Left", 3, 56, 42, 42, 0.8 },
		{ 1, L"Bottom Right", 56, 56, 42, 42, 0.8 },

		{ 4, L"Top Half", 3, 3, 94, 46 },
		{ 4, L"Bottom Half", 3, 52, 94, 46 },

		//{ 2, L"Top TwoThirds", 3, 3, 94, 62 },
		//{ 2, L"Bottom Third", 3, 68, 94, 30 },

		//{ 2, L"Top Third", 3, 3, 94, 30 },
		//{ 2, L"Bottom TwoThirds", 3, 36, 94, 62 },


		{ 3, L"Right 1", 5, 03, 90, 15 },
		{ 3, L"Right 2", 5, 19, 90, 15 }
		//{ 2, L"Right 3", 5, 35, 90, 15 },
		//{ 2, L"Right 4", 5, 51, 90, 15 },
		//{ 2, L"Right 5", 5, 67, 90, 15 },
		//{ 2, L"Right 6", 5, 83, 90, 15 },
	};

	LONG ox = 0, oy = 0; // offset
	for (RECT& sr : mInfos) // clac offset from different screens
	{
		ox = std::min(sr.left, ox);
		oy = std::min(sr.top, oy);
	}

	UINT bo = 0; // base offset
	UINT nr = 0;
	for (RECT r : mInfos)
	{
		UINT w = r.right - r.left;
		OffsetRect(&r, bo, 0);
		bo = w * (NP - 1);
		for (UINT i = 0; i < NP; ++i)
		{
			RECT ri = r;
			OffsetRect(&ri, w * i, 0);
			OffsetRect(&ri, -ox, -oy);

			ScreenWnd sw(ScreenWnd(ri, ++nr, w * i));

			for (PositioningCfg& wp : filter(winPositions, [sw](PositioningCfg& wp) { return wp.scr == 0 || wp.scr == sw.nr; })) {
				sw.push_back(wp);
			}

			// only if has positioning wnds
			if (!sw.empty())
				_screenWnds.push_back(sw);
		}
	}

	if (_screenWnds.empty())
		return; // if no positioning wnds,we have nothing to do

	// only if has positioning wnds
	for (ScreenWnd& sw : _screenWnds)
	{
		RECT sr = sw.sr;
		_toolRect.left = std::min( sr.left, _toolRect.left );
		_toolRect.top = std::min( sr.top, _toolRect.top );
		_toolRect.right = std::max( sr.right, _toolRect.right );
		_toolRect.bottom = std::max( sr.bottom, _toolRect.bottom );
	}

	_toolRect = ScaleRect(_toolRect, F);

	LONG w = (_toolRect.right - _toolRect.left) + GetSystemMetrics(SM_CXBORDER) * 2;
	LONG h = (_toolRect.bottom - _toolRect.top) + GetSystemMetrics(SM_CYBORDER) * 2;

	POINT pt = _clickPoint;

	OffsetRect(&_toolRect, pt.x - (w / 2), pt.y + 10);

	{
		using std::ranges::find_if;
		RECT wr;
		GetWindowRect(hParent, &wr);
		//if (auto it = find_if(_realPosRects, [&wr](RECT& r) {return EqualRect(&r, &wr); }); it != _realPosRects.end())
		//{
		//	size_t i = std::distance(_realPosRects.begin(), it);
		//	_currentPreviewPos = _previewPosRects[i];
		//}
	}



	_hWnd = CreateWindowEx(
		WS_EX_TOPMOST, // | WS_EX_TOOLWINDOW,// Optional window styles.
		CLASS_NAME.c_str(),                     // Window class
		L"Pick a rectangle",    // Window text
		WS_POPUP | WS_VISIBLE | WS_BORDER,           
		//WS_OVERLAPPEDWINDOW, // Window style

		// Size and position
		//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		_toolRect.left, _toolRect.top, w, h,

		hParent,       // Parent window    
		NULL,       // Menu
		hInst,     // Instance handle
		NULL        // Additional application data
	);
	if (_hWnd)
	{
		hWndToImplMap[_hWnd] = this;
		::ShowWindow(_hWnd, SW_SHOW);
		log_debug(L"ShowWindow, ScreenToolWnd::pWnd: {}", (void*)this);

		HMODULE hModDll = GetModuleHandle(BUILD(MT_DLL_NAME));
		if (hModDll != NULL)
		{
			HOOKPROC hkCallKeyboardMsg = (HOOKPROC)GetProcAddress(hModDll, MT_HOOK_PROC_KYB);

			DWORD dwThreadId = ::GetCurrentThreadId();
			_hhkCallKeyboardMsg = SetWindowsHookEx(WH_KEYBOARD, hkCallKeyboardMsg, NULL, dwThreadId);
		}
	}
}

ScreenToolWnd::Impl::~Impl()
{
	if (hInst) {
		UnhookWindowsHookEx(_hhkCallKeyboardMsg);

		hWndToImplMap.erase(_hWnd);
		//SendMessage(_hWnd, WM_CLOSE, 0, 0);
		if(IsWindow(_hWnd))
			DestroyWindow(_hWnd);
	}
}

LRESULT ScreenToolWnd::Impl::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	using std::ranges::find_if;

	//wstring msg = wm_to_wstring(message);
	//if(!msg.empty())
	//	log_debug(L"Message: {}\n", msg);
	LRESULT res = 0;

	POINT pt = { 0 };
	GetCursorPos(&pt);
	ScreenToClient(hWnd, &pt);

	//LONG ix = GetSystemMetrics(SM_CXBORDER) * 2;
	//LONG iy = GetSystemMetrics(SM_CXBORDER) * 2;

	//if (auto it = find_if(_previwScrRects, [&pt](RECT& r) {return PtInRect(&r, pt); }); it != _previwScrRects.end())
	//{
	//	if (!EqualRect(&*it, &_currentPreviewScr))
	//	_currentPreviewScr = *it;
	//	InvalidateRect(hWnd, &_currentPreviewScr, FALSE);
	//}
	//else
	//{
	//	for(RECT& r : _previwScrRects)
	//		InvalidateRect(hWnd, &r, FALSE);
	//	if (!_previwScrRects.empty())
	//		_currentPreviewScr = _previwScrRects.front();
	//}

	//auto posRects = std::ranges::filter_view(_previewPosRects, [pt](RECT r) { return PtInRect(&r, pt); });

	switch (message)
	{	
	case WM_MOUSEWHEEL:
	{
		//POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		//ScreenToClient(hWnd, &pt);

		//for (RECT r : posRects)
		//{
		//	log_debug(L"PosRects: Left {}, Top {}, Right {}, Bottom {}\n", r.left, r.top, r.right, r.bottom);
		//}
		//int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		//if (auto it = std::ranges::find_if(posRects, [this](RECT& r) {
		//		bool res = EqualRect(&_currentPreviewPos, &r);
		//		return res;
		//	}); it != posRects.end())
		//{
		//	if (delta < 0)
		//		it = NextPos(it, posRects);
		//	else if (delta > 0)
		//		it = PreviousPos(it, posRects);
		//	_currentPreviewPos = *it;
		//	auto r = _currentPreviewPos;
		//	log_debug(L"MouseWheel, CurPos: Left {}, Top {}, Right {}, Bottom {}", r.left, r.top, r.right, r.bottom);
		//}
		//log_debug(L"MouseWheel, Delta: {}, X: {}, Y: {}\n", delta, pt.x, pt.y);
		break;
	}

	case WM_LBUTTONDOWN:
	{
		if (_currentPosWnd)
		{
			//ScreenWnd& sw = *_currentScreenWnd;

			RECT r = _currentPosWnd->screenRect;

			HWND hParent = GetParent(hWnd);
			SetWindowPos(hParent, HWND_NOTOPMOST, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_SHOWWINDOW);

		}
		//if (auto scr_it = std::ranges::find_if(_previwScrRects, [this](RECT& r) { return EqualRect(&_currentPreviewScr, &r); }); scr_it != _previwScrRects.end())
		//{
		//	if (auto pos_it = std::ranges::find_if(_previewPosRects, [this](RECT& r) {return EqualRect(&r, &_currentPreviewPos); }); pos_it != _previewPosRects.end())
		//	{
		//		size_t i1 = std::distance(_previewPosRects.begin(), pos_it);
		//		size_t i2 = std::distance(_previwScrRects.begin(), scr_it);
		//		if (i1 < _realPosRects.size() && i2 < _realScrRects.size()) 
		//		{
		//			RECT r = _realPosRects[i1];
		//			RECT scr = _realScrRects[i2];

		//			HWND hParent = GetParent(hWnd);
		//			SetWindowPos(hParent, HWND_NOTOPMOST, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_SHOWWINDOW);
		//		}
		//	}
		//}
		ScreenToolWnd::pWnd.reset();
		return  DefWindowProc(hWnd, message, wParam, lParam);
	}

	case WM_MOUSEMOVE:
	{
		//POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		//ScreenToClient(hWnd, &pt);
		if (auto it = find_if(_screenWnds, [pt](ScreenWnd& sw) { return sw.hit(pt); }); it != _screenWnds.end())
		{
			ScreenWnd& sw = *it;
			_currentScreenWnd = &sw;
			PositioningWnd* posWnd = sw.find(pt);
			if (_currentPosWnd != posWnd)
			{
				if(_currentPosWnd)
					InvalidateRect(hWnd, &_currentPosWnd->previewRect, TRUE);
				_currentPosWnd = posWnd;
				if (_currentPosWnd)
					InvalidateRect(hWnd, &_currentPosWnd->previewRect, TRUE);
			}
		}
		else
		{
			_currentScreenWnd = nullptr;
			_currentPosWnd = nullptr;
		}

		//static std::deque<POINT> last_pts;
		//last_pts.push_back(pt);
		//while(last_pts.size() > 10)
		//	last_pts.pop_front();

		//using std::ranges::minmax;
		//using std::views::transform;
		//using std::distance;
		//using std::abs;

		//auto [x_min, x_max] = minmax(transform(last_pts, [](POINT pt) { return pt.x; }));
		//auto [y_min, y_max] = minmax(transform(last_pts, [](POINT pt) { return pt.y; }));
		//log_debug(L"Min,Max: {}, {} = {}", y_min, y_max, abs(y_max - y_min));
		//if (/*std::abs(x_max - x_min) > 10 ||*/  abs(y_max - y_min) > 10)
		//{
		//	if (auto it = std::ranges::find_if(posRects, [this](RECT& r) {
		//		bool res = EqualRect(&_currentPreviewPos, &r);
		//		return res;
		//		}); it != posRects.end())
		//	{
		//		//if (delta < 0)
		//		it = NextPos(it, posRects);
		//		//else if (delta > 0)
		//		//	it = PreviousPos(it, posRects);
		//		_currentPreviewPos = *it;
		//		auto r = _currentPreviewPos;
		//		//log_debug(L"MouseMove, CurPos: Left {}, Top {}, Right {}, Bottom {}", r.left, r.top, r.right, r.bottom);
		//	}
		//}
		//else
		//{
			//if (auto it = std::ranges::find_if(
			//	_previewPosRects.rbegin(),
			//	_previewPosRects.rend(),
			//	[pt](RECT& r) {
			//		bool res = PtInRect(&r, pt);
			//		return res;
			//	}); it != _previewPosRects.rend())
			//{
			//	if (last_pts.size() > 10 && EqualRect(&_currentPreviewPos, &*it))
			//	{

			//	}
			//	_currentPreviewPos = *it;
			//	auto cp = _currentPreviewPos;
			//	//log_debug(L"MouseMove, CurPos: Left {}, Top {}, Right {}, Bottom {}\n", cp.left, cp.top, cp.right, cp.bottom);
			//}
		//}
		break;
	}

	case WM_KEYDOWN: 
	{
		if (wParam == VK_ESCAPE)
		{
			ScreenToolWnd::pWnd.reset();
			return  DefWindowProc(hWnd, message, wParam, lParam);
		}
		//else if (is_one_of(wParam, VK_DOWN, VK_RIGHT, VK_UP, VK_LEFT))
		//{
		//	if (auto it = std::ranges::find_if(_previewPosRects, [this](RECT& r) {
		//		bool res = EqualRect(&_currentPreviewPos, &r);
		//		return res;
		//		}); it != _previewPosRects.end())
		//	{
		//		if (is_one_of(wParam, VK_DOWN, VK_RIGHT))
		//			it = NextPos(it, _previewPosRects);
		//		else //if (is_one_of(wParam, VK_UP, VK_LEFT))
		//			it = PreviousPos(it, _previewPosRects);
		//		_currentPreviewPos = *it;
		//	}
		//	else
		//	{
		//		if (!_previewPosRects.empty())
		//			_currentPreviewPos = _previewPosRects.front();
		//	}
		//}
		//else if (wParam == VK_RETURN)
		//{
		//	if (auto scr_it = std::ranges::find_if(_previwScrRects, [this](RECT& r) { return EqualRect(&_currentPreviewScr, &r); }); scr_it != _previwScrRects.end())
		//	{
		//		if (auto pos_it = std::ranges::find_if(_previewPosRects, [this](RECT& r) {return EqualRect(&r, &_currentPreviewPos); }); pos_it != _previewPosRects.end())
		//		{
		//			size_t i1 = std::distance(_previewPosRects.begin(), pos_it);
		//			size_t i2 = std::distance(_previwScrRects.begin(), scr_it);
		//			if (i1 < _realPosRects.size() && i2 < _realScrRects.size())
		//			{
		//				RECT r = _realPosRects[i1];
		//				RECT scr = _realScrRects[i2];

		//				HWND hParent = GetParent(hWnd);
		//				SetWindowPos(hParent, HWND_NOTOPMOST, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_SHOWWINDOW);
		//			}
		//		}
		//	}

		//	ScreenToolWnd::pWnd.reset();
			return  DefWindowProc(hWnd, message, wParam, lParam);
		//}
		//else
		//{
		//	wchar_t strDataToSend[32];
		//	wsprintf(strDataToSend, L"%c", wParam);
		//	MessageBox(NULL, strDataToSend, L"keyselected", MB_OK);
		//}
		break;

	} // end switch

	case WM_PAINT:
	{
		//InflateRect(&currentPart, -ix * 4, -iy * 4);

		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);

		for (ScreenWnd& sw : filter(_screenWnds, [](ScreenWnd& sw) { return !sw.empty(); }))
			sw.Paint(ps, hDC);

		//for (RECT sr : _previwScrRects) // screen rect		
		//{
		//	InflateRect(&sr, -ix, -iy);
		//	FillRect(hDC, &sr, GetSysColorBrush(COLOR_ACTIVEBORDER));
		//	FrameRect(hDC, &sr, GetSysColorBrush(COLOR_HOTLIGHT));			
		//}

		//for (RECT pr : _previewPosRects) // part rect	
		//{
		//	//InflateRect(&pr, -ix * 4, -iy * 4);
		//	FillRect(hDC, &pr, GetSysColorBrush(COLOR_INACTIVECAPTION));
		//	FrameRect(hDC, &pr, GetSysColorBrush(COLOR_HOTLIGHT));
		//	//std::wostringstream os;
		//	//os << L"x: " << pr.left << L" y: " << pr.top << L" w: " << pr.right - pr.left << L" h: " << pr.bottom - pr.top;
		//	//DrawText(hDC, os.str().c_str(), -1, &pr, DT_LEFT | DT_TOP | DT_WORDBREAK);
		//}

		if (_currentPosWnd)
		{
			FillRect(hDC, &_currentPosWnd->previewRect, GetSysColorBrush(COLOR_ACTIVECAPTION));
			FrameRect(hDC, &_currentPosWnd->previewRect, GetSysColorBrush(COLOR_HOTLIGHT));
		}


		EndPaint(hWnd, &ps);
		break;
	}

	} // end switch

	return  DefWindowProc(hWnd, message, wParam, lParam);
}



LRESULT ScreenToolWnd::Impl::ToolWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ScreenToolWnd::Impl* pImpl = hWndToImplMap[hWnd];
	if (pImpl)
		return pImpl->WndProc(hWnd, message, wParam, lParam);

	return DefWindowProc(hWnd, message, wParam, lParam);
}

template<typename It, typename Ct>
inline It ScreenToolWnd::Impl::NextPos(It it, Ct& posRects)
{
	if (it == posRects.end())
		it = posRects.begin();
	else if (++it == posRects.end())
		it = posRects.begin();
	log_debug(L"NextPos {}", std::distance(posRects.begin(), it));
	return it;
}

template<typename It, typename Ct>
inline It ScreenToolWnd::Impl::PreviousPos(It it, Ct& posRects)
{
	if (it == posRects.begin())
		it = (--posRects.end());
	else if (--it == posRects.begin())
		it = (--posRects.end());
	log_debug(L"PreviousPos {}", std::distance(posRects.begin(), it));
	return it;
}


//RECT ScreenToolWnd::Impl::Calculate(PositioningCfg& wp, Screen& s)
//{
//	RECT r = { 
//		static_cast<LONG>(s.x + wp.x * (s.w / 100.0)),
//		static_cast<LONG>(s.y + wp.y * (s.h / 100.0)),
//	};
//	r.right = static_cast<LONG>(r.left + wp.w * (s.w / 100.0));
//	r.bottom = static_cast<LONG>(r.top + wp.h * (s.h / 100.0));
//	return r;
//}

