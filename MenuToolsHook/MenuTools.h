#include <tchar.h>

namespace MenuTools
{
	// Functions
	BOOL Install(HWND hWnd);
	BOOL Uninstall(HWND hWnd);
	VOID Status(HWND hWnd);

	// Callback
	BOOL WndProc(HWND hWnd, WPARAM wParam);
};


// Helpers
BOOL InsertSubMenu(HMENU hMenu, HMENU hSubMenu, UINT uPosition, UINT uFlags, UINT uIDNewItem, LPCWSTR lpNewItem);
BOOL IsMenuItem(HMENU hMenu, UINT item);