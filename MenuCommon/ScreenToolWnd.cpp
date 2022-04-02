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

	const FLOAT F = 0.1f; // factor
	const UINT CLOSE_TIMER = 0x30;
	const UINT CLOSE_TIMEOUT = 3500;
}
RECT ScaleRect(RECT& in, FLOAT f);
DWORD WINAPI KeyThread(LPVOID);


//struct Screen
//{
//	size_t scr; // screen number
//	int x, y, w, h; // left, top, width, height in pixels
//};


struct PositioningCfg {
	int monNr, prvNr; //monitor 0 = all, 1 = first, screen 0 = all, 1 = first, 2 = second, ...
	wstring name; // name
	int x, y, w, h; // left, top, width, height in % from screen
	double prvFactor = 1; // preview factor, to scale the preview against the real size
};
using PositioningCfgs = std::vector<PositioningCfg>;

struct ScreenWnd;

struct PositioningWnd {
	PositioningWnd(PositioningCfg& pc, ScreenWnd* sw);

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
	ScreenWnd(RECT& r, UINT s, UINT p, UINT po, POINT so)
		: _scrRect(r)
		, _monNr(s)
		, _prvNr(p)
		, _prvOffset(po)
		, _scrOffset(so)
	{
		_prvRect = ScaleRect(_scrRect, F);
	}

	void Paint(PAINTSTRUCT& ps, HDC hDC);
	
	void push_back(PositioningCfg& wp) { 
		return _posititioningWnds.push_back(PositioningWnd(wp, this));
	}

private:

	UINT _monNr; ///> Screen Nr
	UINT _prvNr; ///> Preview Nr
	UINT _prvOffset = 0; ///> Offset of preview, if multiple previews are available
	POINT _scrOffset = { 0 }; ///> Offset for screens with different resolution
	RECT _prvRect; ///> Preview
	RECT _scrRect; ///> Screen
	PositioningWnds _posititioningWnds; ///> Positioning Windows

	friend struct PositioningWnd;

public:
	bool empty() { return _posititioningWnds.empty(); }

	PropR<UINT, ScreenWnd, &ScreenWnd::_monNr> monNr = { this };
	PropR<UINT, ScreenWnd, &ScreenWnd::_prvNr> prvNr = { this };
	PropR<UINT, ScreenWnd, &ScreenWnd::_prvOffset> prvOffset = { this };
	PropR<POINT, ScreenWnd, &ScreenWnd::_scrOffset> scrOffset = { this };
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

	bool hit(const RECT& wr) // Window-Rect
	{
		return EqualRect(&_scrRect, &wr);
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

	PositioningWnd* find(const RECT& wr) // Window-Rect
	{
		using std::ranges::find_if;

		auto &pw = _posititioningWnds;
		if (auto it = find_if(pw.rbegin(), pw.rend(), [wr](PositioningWnd& pw) { return EqualRect(&pw.screenRect, &wr); }); it != pw.rend())
		{
			return &*it;
		}

		return nullptr;
	}

	PositioningWnd* next(PositioningWnd* curWnd, POINT* pt = nullptr)
	{
		using std::ranges::find_if;
		using std::views::filter;

		auto pws = filter(_posititioningWnds, [pt](PositioningWnd& pw) {
			return pt == nullptr || PtInRect(&pw.previewRect, *pt);
			});

		if (curWnd == nullptr)
			return &pws.front();

		auto wr = curWnd->screenRect;
		if (auto it = find_if(pws.begin(), pws.end(), [wr](PositioningWnd& pw) { return EqualRect(&pw.screenRect, &wr); }); it != pws.end())
		{
			if (++it == pws.end())
			{
				if (pt == nullptr) 
					return nullptr;
				it = pws.begin();
			}
			return &*it;
		}
		return nullptr;
	}

	PositioningWnd* prev(PositioningWnd* curWnd, POINT* pt = nullptr)
	{
		using std::ranges::find_if;
		using std::views::filter;

		auto pws = filter(_posititioningWnds, [pt](PositioningWnd& pw) {
			return pt == nullptr || PtInRect(&pw.previewRect, *pt);
			}) | std::views::reverse;


		if (curWnd == nullptr)
			return &pws.front();

		auto wr = curWnd->screenRect;
		if (auto it = find_if(pws.begin(), pws.end(), [wr](PositioningWnd& pw) { return EqualRect(&pw.screenRect, &wr); }); it != pws.end())
		{
			if (++it == pws.end())
			{
				if (pt == nullptr) 
					return nullptr;
				it = pws.begin();
			}
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

PositioningWnd::PositioningWnd(PositioningCfg& pc, ScreenWnd* sw) :_posCfg(pc), _scrWnd(*sw) {
	screenRect = calcScreenRect();
	previewRect = calcPreviewRect();
	OffsetRect(&screenRect, (sw->prvOffset() * -1) + sw->scrOffset().x, sw->scrOffset().y);
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

	int ix = static_cast<int>(((w * 1.0) - (w * pc.prvFactor)) / 2.0);
	int iy = static_cast<int>(((h * 1.0) - (h * pc.prvFactor)) / 2.0);
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
	void OnKeyDown(UINT virtualKey);
	
	void nextPosWnd(POINT* pt = nullptr)
	{
		RECT before = _currentPosWnd ? _currentPosWnd->previewRect : RECT{};
		if (_currentScreenWnd == nullptr)
		{ 
			_currentScreenWnd = &_screenWnds.front();
			_currentPosWnd = _currentScreenWnd->next(_currentPosWnd);
		}
		else
		{
			_currentPosWnd = _currentScreenWnd->next(_currentPosWnd, pt);
			if (_currentPosWnd == nullptr)
			{
				using std::ranges::find_if;

				ScreenWnd* csw = _currentScreenWnd;
				if (auto it = find_if(_screenWnds.begin(), _screenWnds.end(), [csw](ScreenWnd& sw) { return csw == &sw; }); it != _screenWnds.end())
				{
					if (++it == _screenWnds.end())
						it = _screenWnds.begin();
					_currentScreenWnd = &*it;
					InvalidateRect(_hWnd, &before, TRUE);
					return nextPosWnd();
				}
			}
		}
		RECT after = _currentPosWnd ? _currentPosWnd->previewRect : RECT{};
		InvalidateRect(_hWnd, &before, TRUE);
		InvalidateRect(_hWnd, &after, TRUE);
	}

	void prevPosWnd(POINT* pt = nullptr)
	{
		RECT before = _currentPosWnd ? _currentPosWnd->previewRect : RECT{};
		if (_currentScreenWnd == nullptr)
		{ 
			_currentScreenWnd = &_screenWnds.back();
			_currentPosWnd = _currentScreenWnd->prev(_currentPosWnd);
		}
		else
		{
			_currentPosWnd = _currentScreenWnd->prev(_currentPosWnd, pt);
			if (_currentPosWnd == nullptr)
			{
				using std::ranges::find_if;

				ScreenWnd* csw = _currentScreenWnd;
				if (auto it = find_if(_screenWnds.rbegin(), _screenWnds.rend(), [csw](ScreenWnd& sw) { return csw == &sw; }); it != _screenWnds.rend())
				{
					if (++it == _screenWnds.rend())
						it = _screenWnds.rbegin();
					_currentScreenWnd = &*it;
					InvalidateRect(_hWnd, &before, TRUE);
					return prevPosWnd();
				}
			}
		}
		RECT after = _currentPosWnd ? _currentPosWnd->previewRect : RECT{};
		InvalidateRect(_hWnd, &before, TRUE);
		InvalidateRect(_hWnd, &after, TRUE);
	}

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

VOID ScreenToolWnd::OnKeyDown(UINT virtualKey)
{
	if(_pImpl)
		_pImpl->OnKeyDown(virtualKey);
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


BOOL CALLBACK Monitorenumproc(HMONITOR hMon, HDC hDC, LPRECT pRECT, LPARAM lParam)
{
	std::vector<RECT>& mInfos = *(std::vector<RECT>*)lParam;
	MONITORINFOEX mi = { 0 };
	mi.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMon, &mi);
	mInfos.push_back(mi.rcWork);
	return TRUE;
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
	std::vector<RECT> monInfos;
	EnumDisplayMonitors(NULL, NULL, Monitorenumproc, (LPARAM)&monInfos);

	if (monInfos.empty())
	{
		RECT wa = { 0 };
		SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0);
		monInfos.push_back(wa);
	}
	//std::vector<RECT> scrRects;
	// MonNr, PrvNr, Name, Left%, Top%, Width%, Height%
	PositioningCfgs winPositions = {
		{1, 1, L"Left TwoThirds", 3, 6, 64, 92 },
		{1, 1, L"Right Third", 67, 6, 31, 92 },

		{1, 1, L"Small Wide Center", 15, 15, 70, 70, 0.8 },
		{1, 1, L"Mini Wide Center", 25, 25, 50, 50, 0.7 },

		{1, 2, L"Left Third", 3, 6, 31, 92 },
		{1, 2, L"Right TwoThirds", 34, 6, 64, 92 },

		{1, 2, L"Big Wide Center", 3, 6, 95, 92, 0.65 },

		{1, 2, L"Left Half", 2, 2, 49, 96, 0.8 },
		{1, 2, L"Right Half", 51, 2, 49, 96, 0.8 },

		{1, 1, L"Top Left", 3, 0, 39, 50, 0.75 },
		{1, 1, L"Top Right", 58, 0, 39, 50, 0.75 },
		{1, 1, L"Bottom Left", 3, 50, 39, 50, 0.75 },
		{1, 1, L"Bottom Right", 58, 50, 39, 50, 0.75 },

		{2, 2, L"Top TwoThirds", 6, 3, 92, 60 , 1 },
		{2, 2, L"Bottom Third", 6, 63, 92, 35 , 1 },

		{2, 1, L"Top Third", 6, 3, 92, 35, 1.05},
		{2, 1, L"Bottom TwoThirds", 6, 38, 92, 60, 1.05 },

		{2, 2, L"Top Half", 6, 3, 92, 46, 0.8 },
		{2, 2, L"Bottom Half", 6, 52, 92, 46, 0.8 },


		{2, 2, L"Big Height Center", 6, 4, 92, 95, 0.65 },

		{2, 1, L"Right 1", 6, 03, 92, 10, 0.9 },
		{2, 1, L"Right 2", 6, 15, 92, 10, 0.9 },
		{2, 1, L"Right 3", 6, 27, 92, 10, 0.9 },
		{2, 1, L"Right 4", 6, 39, 92, 10, 0.9 },
		{2, 1, L"Right 5", 6, 51, 92, 10, 0.9 },
		{2, 1, L"Right 6", 6, 63, 92, 10, 0.9 },
		{2, 1, L"Right 7", 6, 75, 92, 10, 0.9 },
		{2, 1, L"Right 8", 6, 87, 92, 10, 0.9 },

		{2, 1, L"Small Height Center TwoThirds", 15, 15, 70, 70, 0.7 },
		{2, 1, L"Mini Height Center", 25, 25, 50, 50, 0.7 }
};

	using std::views::transform;
	//using std::views::filter;
	using std::ranges::min;
	using std::ranges::max;

	 // determine offset from screens with different resolutions
	LONG ox = min(monInfos | transform([](const RECT& r) {return r.left; }));
	LONG oy = min(monInfos | transform([](const RECT& r) {return r.top; }));

	// baseOffset means the offset from ScreenWnd if more than one per monitor is shown
	for (UINT mi = 0, baseOffset = 0; mi < monInfos.size(); ++mi) // monitor-index
	{
		auto wps = filter(winPositions, [mi](const PositioningCfg& pc) { return (mi + 1) == pc.monNr; });
		if(wps.empty())
			continue;

		// the number of previews for this monitor
		UINT np = max(transform(wps, [](const PositioningCfg& cfg) {return cfg.prvNr; }));

		RECT monRect = monInfos[mi];
		UINT monWidth = monRect.right - monRect.left;

		for (UINT pi = 0; pi < np; ++pi) // preview-index
		{
			RECT ri = monRect;
			OffsetRect(&ri, (signed)baseOffset, 0);
			OffsetRect(&ri, -ox, -oy);

			ScreenWnd sw(ScreenWnd(ri, mi + 1, pi + 1, baseOffset, { ox, oy }));

			// add the monitor width to the baseOffset
			baseOffset += monWidth;

			auto belongToScreenWnd = [&sw](PositioningCfg& wp) -> bool
			{
				if (wp.monNr == 0 || wp.monNr == sw.monNr)
					return wp.prvNr == 0 || wp.prvNr == sw.prvNr;
				else
					return false;
			};

			for (PositioningCfg& wp : wps | filter( belongToScreenWnd )) {
				sw.push_back(wp);
			}

			// only if has positioning wnds
			if (!sw.empty())
				_screenWnds.push_back(sw);
		}
		// remove the last one, to suppress the gap between the monitor previews
		baseOffset -= monWidth;
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
		::SetFocus(_hWnd);
		log_debug(L"ShowWindow, ScreenToolWnd::pWnd: {}", (void*)this);

		SetTimer(_hWnd, CLOSE_TIMER, CLOSE_TIMEOUT, (TIMERPROC)NULL);

		CreateThread(NULL, NULL, KeyThread, NULL, NULL, NULL);

		HMODULE hModDll = GetModuleHandle(BUILD(MT_DLL_NAME));
		if (hModDll != NULL)
		{
			HOOKPROC hkCallKeyboardMsg = (HOOKPROC)GetProcAddress(hModDll, MT_HOOK_PROC_KYB);

			DWORD dwThreadId = ::GetCurrentThreadId();
			_hhkCallKeyboardMsg = SetWindowsHookEx(WH_KEYBOARD, hkCallKeyboardMsg, NULL, dwThreadId);
		}
	}
}

DWORD WINAPI KeyThread(LPVOID)
{
	while(ScreenToolWnd::pWnd)
	{
		auto vks = { VK_RETURN, VK_LEFT, VK_RIGHT, VK_UP, VK_DIVIDE };
		for(auto vk : vks)
		{ 
			if (HIBYTE(GetAsyncKeyState(vk)))
				ScreenToolWnd::pWnd->OnKeyDown(vk);
		}
	}
	return 0;
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


	HWND hParent = GetParent(hWnd);
	RECT wr = {};
	GetWindowRect(hParent, &wr);

	PositioningWnd* fittingPosWnd = nullptr;
	for (ScreenWnd& sw : _screenWnds)
	{
		if (PositioningWnd* posWnd = sw.find(wr))
		{
			fittingPosWnd = posWnd;
			break;
		}
	}

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
	//case WM_CREATE:
	//{
	//	SetTimer(hWnd, 13, 25000, (TIMERPROC)NULL);
	//	break;
	//}
	case WM_TIMER:
	{
		if (wParam == CLOSE_TIMER)
		{
			ScreenToolWnd::pWnd.reset();
			return  DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}

	case WM_MOUSEWHEEL:
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		ScreenToClient(hWnd, &pt);

		SetTimer(_hWnd, CLOSE_TIMER, CLOSE_TIMEOUT * 4, (TIMERPROC)NULL);
		int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (delta < 0)
			nextPosWnd(&pt);
		else if (delta > 0)
			prevPosWnd(&pt);


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

	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE)
		{
			ScreenToolWnd::pWnd.reset();
			return  DefWindowProc(hWnd, message, wParam, lParam);
		}
		else if (is_one_of(wParam, VK_DOWN, VK_RIGHT, VK_UP, VK_LEFT))
		{
			SetTimer(_hWnd, CLOSE_TIMER, CLOSE_TIMEOUT * 4, (TIMERPROC)NULL);
			if (is_one_of(wParam, VK_DOWN, VK_RIGHT))
				nextPosWnd();
			else //if (is_one_of(wParam, VK_UP, VK_LEFT))
				prevPosWnd();
		}
		else if (wParam == VK_RETURN)
		{
			if (_currentPosWnd)
			{
				RECT wr = _currentPosWnd->screenRect;

				WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
				GetWindowPlacement(hParent, &wp);

				wp.rcNormalPosition = wr;
				wp.showCmd = SW_NORMAL;
				SetWindowPlacement(hParent, &wp);
				ScreenToolWnd::pWnd.reset();
			}
		}

		return  DefWindowProc(hWnd, message, wParam, lParam);
		break;

	} // end switch


	case WM_LBUTTONDOWN:
	{
		if (_currentPosWnd)
		{
			RECT wr = _currentPosWnd->screenRect;

			WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
			GetWindowPlacement(hParent, &wp);

			wp.rcNormalPosition = wr;
			wp.showCmd = SW_NORMAL;
			SetWindowPlacement(hParent, &wp);
		}
		ScreenToolWnd::pWnd.reset();
		return  DefWindowProc(hWnd, message, wParam, lParam);
	}

	case WM_MOUSEMOVE:
	{
		SetTimer(_hWnd, CLOSE_TIMER, CLOSE_TIMEOUT, (TIMERPROC)NULL);
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
		break;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);

		for (ScreenWnd& sw : filter(_screenWnds, [](ScreenWnd& sw) { return !sw.empty(); }))
			sw.Paint(ps, hDC);

		if (_currentPosWnd)
		{
			FillRect(hDC, &_currentPosWnd->previewRect, GetSysColorBrush(COLOR_ACTIVECAPTION));
			FrameRect(hDC, &_currentPosWnd->previewRect, GetSysColorBrush(COLOR_HOTLIGHT));
		}

		if (fittingPosWnd)
		{
			FrameRect(hDC, &fittingPosWnd->previewRect, GetSysColorBrush(COLOR_WINDOWFRAME));
		}


		EndPaint(hWnd, &ps);
		break;
	}

	} // end switch

	return  DefWindowProc(hWnd, message, wParam, lParam);
}

void ScreenToolWnd::Impl::OnKeyDown(UINT virtualKey)
{
	PostMessage(_hWnd, WM_KEYDOWN, virtualKey, 0);
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

