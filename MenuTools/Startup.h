class Startup
{
	// Functions
public:
	Startup();
	~Startup();
	BOOL CreateJob();
private:
	BOOL CreateJobChild();

	// Members
protected:
	HANDLE hJob;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
};