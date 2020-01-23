#include "stdafx.h"
#include "Startup.h"

#include <ShellAPI.h>

Startup::Startup()
{
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	flags = NONE;
}

Startup::~Startup()
{
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	CloseHandle(hJob);
}

BOOL Startup::ParseFlags(LPCWSTR lpCmdLine)
{
	LPWSTR *szArglist;
	int nArgs;
	szArglist = CommandLineToArgvW(lpCmdLine, &nArgs);
	if (szArglist == NULL)
	{
		return FALSE;
	}

	// Parse command line
	for (int i = 0; i < nArgs; i++)
	{
		if (wcscmp(szArglist[i], L"-hide") == 0)
		{
			flags |= Flags::HIDE_TRAY;
		}
	}

	// Free memory
	LocalFree(szArglist);

	return TRUE;
}

BOOL Startup::CreateJob()
{
	// Create a job
	hJob = CreateJobObject(NULL, MT_JOB_NAME);
	if (!hJob)
	{
		return FALSE;
	}

#ifdef _WIN64
#if !MT_DEBUG_ONLY_X64
	// Check if application x64 is running with a job
	BOOL bIsJob;
	if(!IsProcessInJob(GetCurrentProcess(), hJob, &bIsJob))
	{
		return FALSE;
	}

	// Not running inside a job
	if(!bIsJob)
	{
		return FALSE;
	}
#endif
#endif

	// If the job already exists
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
#ifdef _WIN64
		// Close the job handle
		CloseHandle(hJob);
		// Don't create child and allow process creation
		return TRUE;
#else
		// Can't run more than one instance
		return FALSE;
#endif
	}

#ifndef _WIN64
#if !MT_DEBUG_ONLY_X86
	// Check if application x86 is running on Windows x64
	BOOL bIsWOW64;
	if (!IsWow64Process(GetCurrentProcess(), &bIsWOW64))
	{
		return FALSE;
	}

	// Create a child to handle x64 processes
	if (bIsWOW64)
	{
		return CreateJobChild();
	}
#endif
#endif

	return TRUE;
}

BOOL Startup::CreateJobChild()
{
	// Causes all processes associated with the job to terminate when the last handle to the job is closed.
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	if (!SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION)))
	{
		return FALSE;
	}

	// Creates a new process is created in a suspended state
	if (!CreateProcess(MT_EXE_NAME64, NULL, NULL, NULL, FALSE,
		CREATE_SUSPENDED | CREATE_BREAKAWAY_FROM_JOB,
		NULL, NULL, &si, &pi))
	{
		return FALSE;
	}

	// Assigns a process to an existing job object
	if (!AssignProcessToJobObject(hJob, pi.hProcess))
	{
		return FALSE;
	}

	// Execution of the thread is resumed
	if (ResumeThread(pi.hThread) == -1)
	{
		return FALSE;
	}

	return TRUE;
}