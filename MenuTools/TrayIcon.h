#include <ShellAPI.h>
#include <strsafe.h>

#define TRAYICON_ID				MT_TRAY_ID
#define TRAYICON_MESSAGE		MT_TRAY_MESSAGE

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
