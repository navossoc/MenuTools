#include <map>
#include <ShellAPI.h>

#define TRAYICON_MESSAGE		MT_HOOK_MSG_TRAY

class TrayIcon
{
	// Functions
public:
	TrayIcon(HWND hWnd);
	~TrayIcon();

	void SetCallBackMessage(UINT uMessage);

	UINT Show();
	BOOL Hide();

private:
	HICON GetWindowIcon(HWND hWnd);

	// Members
protected:
	NOTIFYICONDATA nid;
	static UINT uId;
};

// Tray Map
typedef std::map<HWND, TrayIcon> Tray_Map;
typedef std::map<HWND, TrayIcon>::iterator Tray_It;
typedef std::pair<HWND, TrayIcon> Tray_Pair;

extern Tray_Map mTrays;