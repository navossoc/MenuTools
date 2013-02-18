#include <ShellAPI.h>
#include <strsafe.h>

#define TRAYICON_ID				1
#define TRAYICON_MESSAGE		WM_USER+100

class TrayIcon
{
	// Functions
public:
	TrayIcon(TCHAR* szTitle = NULL, HICON hIcon = NULL);
	~TrayIcon();

	BOOL SetIcon(HICON hIcon);
	BOOL SetIcon(HINSTANCE hInstance, ULONG uIcon);
	BOOL SetTooltip(TCHAR* szTitle);

	BOOL Show(HWND hWnd);
	BOOL Hide();

	// Members
protected:
	NOTIFYICONDATA nid;
};
