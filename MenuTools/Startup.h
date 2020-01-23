class Startup
{
public:
	// Enums
	enum Flags
	{
		NONE = 0,
		HIDE_TRAY = 1
	};

	// Functions
	Startup();
	~Startup();

	BOOL ParseFlags(LPCWSTR lpCmdLine);
	BOOL CreateJob();

	int flags;

private:
	BOOL CreateJobChild();

	// Members
protected:
	HANDLE hJob;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
};