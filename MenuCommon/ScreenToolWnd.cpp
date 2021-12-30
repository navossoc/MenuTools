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

using namespace std::placeholders;
std::wstring wm_to_wstring(UINT msg);


enum class WinPos : unsigned char { HT, HB, HL, HR, /*TL, TR, BL, BR,*/ SC, BC };

//constexpr std::wostringstream& operator<<(std::wostringstream& os, WinPos wp)
//{
//	switch (wp)
//	{
//	case WinPos::HT:
//		 os << L"HalfTop";
//		 break;
//	case WinPos::HB:
//		os << L"HalfBottom";
//		break;
//	case WinPos::HL:
//		os << L"HalfLeft";
//		break;
//	case WinPos::HR:
//		os << L"HalfRight";
//		break;
//	case WinPos::TL:
//		os << L"TopLeft";
//		break;
//	case WinPos::TR:
//		os << L"TopRight";
//		break;
//	case WinPos::BL:
//		os << L"BottomLeft";
//		break;
//	case WinPos::BR:
//		os << L"BottomRight";
//		break;
//	case WinPos::SC:
//		os << L"SmallCenter";
//		break;
//	case WinPos::BC:
//		os << L"BigCenter";
//		break;
//	default:
//		os << L"";
//		break;
//	}
//	return os;
//}

//template <>
//struct std::formatter<WinPos> : std::formatter<std::wstring> {
//	auto format(WinPos wp, wformat_context& ctx) {
//		std::wostringstream os;
//		os << wp;
//		return os.str();
//	}
//};

//constexpr WinPos& operator ++(WinPos& wp)
//{
//	switch (wp)
//	{
//	case WinPos::HT:
//		wp = WinPos::HB;
//		break;
//	case WinPos::HB:
//		wp = WinPos::HL;
//		break;
//	case WinPos::HL:
//		wp = WinPos::HR;
//		break;
//	case WinPos::HR:
//		wp = WinPos::TL;
//		break;
//	case WinPos::TL:
//		wp = WinPos::TR;
//		break;
//	case WinPos::TR:
//		wp = WinPos::BL;
//		break;
//	case WinPos::BL:
//		wp = WinPos::BR;
//		break;
//	case WinPos::BR:
//		wp = WinPos::SC;
//		break;
//	default:
//		wp = WinPos::HT;
//		break;
//	//case WinPos::SC:
//	//	wp = WinPos::BC;
//	//	break;
//	//case WinPos::BC:
//	//	wp = WinPos::SC;
//	//	break;
//	}
//	return wp;
//}


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

LRESULT ScreenToolWnd::Impl::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	OutputDebugString(std::format(L"Message: {}\n", wm_to_wstring(message)).c_str());
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
			//InflateRect(&r, -ix * 4, -iy * 4);
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
			if (i1 < _bigPartRects.size() && i2 < _bigScrRects.size()) 
			{
				RECT r = _bigPartRects[i1];
				RECT scr = _bigScrRects[i2];
				//if (r.left == scr.left || r.top == scr.top || r.right == scr.right || r.bottom == scr.bottom) 
				//{
				//	LONG inflateX = ((scr.right - scr.left) / 100) * -2;
				//	LONG inflateY = ((scr.bottom - scr.top) / 100) * -2;
				//	InflateRect(&r, inflateX, inflateY);
				//}
				HWND hParent = GetParent(hWnd);
				SetWindowPos(hParent, HWND_NOTOPMOST, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_SHOWWINDOW);
			}

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
		//InflateRect(&currentPart, -ix * 4, -iy * 4);

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
			//InflateRect(&pr, -ix * 4, -iy * 4);
			FillRect(hDC, &pr, GetSysColorBrush(COLOR_INACTIVECAPTION));
			FrameRect(hDC, &pr, GetSysColorBrush(COLOR_HOTLIGHT));
			//std::wostringstream os;
			//os << L"x: " << pr.left << L" y: " << pr.top << L" w: " << pr.right - pr.left << L" h: " << pr.bottom - pr.top;
			//DrawText(hDC, os.str().c_str(), -1, &pr, DT_LEFT | DT_TOP | DT_WORDBREAK);
		}

		FillRect(hDC, &currentPart, GetSysColorBrush(COLOR_ACTIVECAPTION));
		FrameRect(hDC, &currentPart, GetSysColorBrush(COLOR_HOTLIGHT));

		EndPaint(hWnd, &ps);
		break;
	}

	case WM_KEYDOWN: if (wParam == VK_ESCAPE)
			DestroyWindow(hWnd);
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

	//RECT wa = { 0 };
	//SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0);

	LONG ox = 0, oy = 0; // offset

	for (RECT& sr : _bigScrRects) // screen rect
	{
		LONG F = 1;
		sr.right = ((sr.right - sr.left) + sr.left) / F;
		sr.bottom = ((sr.bottom - sr.top) + sr.top) / F;
		sr.left = sr.left / F;
		sr.top = sr.top / F;
		ox = std::min(sr.left, ox);
		oy = std::min(sr.top, oy);
	}


	RECT wr = { 0 }; // window rect
	for (RECT& sr : _bigScrRects)
	{
		OffsetRect(&sr, -ox, -oy);

		wr.left = std::min( sr.left, wr.left );
		wr.top = std::min( sr.top, wr.top );
		wr.right = std::max( sr.right, wr.right );
		wr.bottom = std::max( sr.bottom, wr.bottom );
	}


	for (RECT& sr : _bigScrRects)
	{
		LONG sw = (sr.right - sr.left); // screen width
		LONG sh = (sr.bottom - sr.top); // screen height
		LONG os = ((sr.right - sr.left) / 100) * 10; // offset

		if (sw < sh)
		{
			// top
			RECT top = { sr.left, sr.top, sr.left + sw, sr.bottom - sh / 2 };
			//InflateRect(&top, 2, 2);
			_bigPartRects.push_back(top);

			// bottom
			RECT bottom = { sr.left, sr.top + sh / 2, sr.left + sw, sr.bottom };
			_bigPartRects.push_back(bottom);
		}
		else
		{
			// left
			RECT left = { sr.left, sr.top, sr.left + sw / 2, sr.bottom };
			//InflateRect(&left, os, os);
			_bigPartRects.push_back(left);

			// right
			RECT right = { sr.left + sw / 2, sr.top, sr.left + sr.left + sw, sr.bottom };
			_bigPartRects.push_back(right);
		}
		/*
		*/
/*

		// top left
		RECT tl = { sr.left, sr.top, (sr.left + sw) / 2, (sr.top + sh) / 2};
		//OffsetRect(&tl, os, os);
		//InflateRect(&tl, inflate, inflate);
		_bigPartRects.push_back(tl);

		// top right
		RECT tr = tl;
		OffsetRect(&tr, sw / 2 , 0);
		_bigPartRects.push_back(tr);

		// bottom left
		RECT bl = tl;
		OffsetRect(&bl, 0, sh / 2);
		_bigPartRects.push_back(bl);

		// bottom right
		RECT br = tl;
		OffsetRect(&br, sw / 2, sh / 2);
		_bigPartRects.push_back(br);
*/
		// small center
		RECT sc = sr;
		InflateRect(&sc, -(sw / 12), -(sh / 12));
		//OffsetRect(&sc, ox, oy);
		_bigPartRects.push_back(sc);

		// big center
		RECT bc = sr;
		InflateRect(&bc, -(sw / 5), -(sh / 5));
		_bigPartRects.push_back(bc);
	}

	_smallScrRects.clear();
	std::ranges::transform(_bigScrRects, std::back_inserter(_smallScrRects),
		[](RECT& r) {return ScaleRect(r, F); });


	_smallPartRects.clear();
	std::ranges::transform(_bigPartRects, std::back_inserter(_smallPartRects),
		[](RECT& r) {return ScaleRect(r, F); });

	LONG ib = 20;
	LONG is = 8;
	InflateRect(&_bigPartRects[(size_t)WinPos::HL], -ib, -ib);
	OffsetRect(&_bigPartRects[(size_t)WinPos::HL], ib, 0);
	InflateRect(&_smallPartRects[(size_t)WinPos::HL], -is / 2, 0);
	OffsetRect(&_smallPartRects[(size_t)WinPos::HL], -ib /2, 0);

	InflateRect(&_bigPartRects[(size_t)WinPos::HR], -ib, -ib);
	OffsetRect(&_bigPartRects[(size_t)WinPos::HR], -ib, 0);
	InflateRect(&_smallPartRects[(size_t)WinPos::HR], -is, 0 / 2);
	OffsetRect(&_smallPartRects[(size_t)WinPos::HR], ib / 2, 0);

	InflateRect(&_bigPartRects[(size_t)WinPos::HT], -ib, -ib);
	OffsetRect(&_bigPartRects[(size_t)WinPos::HT], 0, ib);
	InflateRect(&_smallPartRects[(size_t)WinPos::HT], 0, -is / 2);
	OffsetRect(&_smallPartRects[(size_t)WinPos::HT], 0, -ib / 2);

	InflateRect(&_bigPartRects[(size_t)WinPos::HB], -ib, -ib);
	OffsetRect(&_bigPartRects[(size_t)WinPos::HB], 0, -ib);
	InflateRect(&_smallPartRects[(size_t)WinPos::HB], 0, -is / 2);
	OffsetRect(&_smallPartRects[(size_t)WinPos::HB], 0, ib / 2);
	/*

	InflateRect(&_bigPartRects[(size_t)WinPos::TL], -ib, -ib);
	OffsetRect(&_bigPartRects[(size_t)WinPos::TL], ib, ib);
	InflateRect(&_smallPartRects[(size_t)WinPos::TL], -is, -is);

	InflateRect(&_bigPartRects[(size_t)WinPos::TR], -ib, -ib);
	OffsetRect(&_bigPartRects[(size_t)WinPos::TR], -ib, ib);
	InflateRect(&_smallPartRects[(size_t)WinPos::TR], -is, -is);

	InflateRect(&_bigPartRects[(size_t)WinPos::BL], -ib, -ib);
	OffsetRect(&_bigPartRects[(size_t)WinPos::BL], ib, -ib);
	InflateRect(&_smallPartRects[(size_t)WinPos::BL], -is, -is);

	InflateRect(&_bigPartRects[(size_t)WinPos::BR], -ib, -ib);
	OffsetRect(&_bigPartRects[(size_t)WinPos::BR], -ib, -ib);
	InflateRect(&_smallPartRects[(size_t)WinPos::BR], -is, -is);
	*/

	InflateRect(&_smallPartRects[(size_t)WinPos::SC], -is, -is);
	InflateRect(&_smallPartRects[(size_t)WinPos::BC], -is, -is);

	// after all calculations, reset the offset in case of more than one screen with differrent resolution 
	for (RECT& bpr : _bigPartRects)
	{
		OffsetRect(&bpr, ox, oy);
	}

	//for (WinPos wp = std::numeric_limits<WinPos>::min(); wp <= std::numeric_limits<WinPos>::max(); ++wp)
	//{
	//	std::wostringstream os;
	//	os << wp;
	//	OutputDebugString(std::format(L"{}\n", os.str()).c_str());
	//}

	wr = ScaleRect(wr, F);

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