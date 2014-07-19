class Hooks
{
	// Functions
public:
	Hooks();
	~Hooks();

	BOOL Install();
	BOOL Uninstall();

	// Members
protected:
	HHOOK hhkCallWndProc;
	HHOOK hhkGetMessage;
};