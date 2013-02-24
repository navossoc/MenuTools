// Includes
#include <tchar.h>

// Defines
#define MAX_BUFFER				1000
#define LOGMESSAGE(format, ...)		\
	{ \
		TCHAR buffer[MAX_BUFFER+1]; \
		StringCchPrintfEx(buffer, MAX_BUFFER, NULL, NULL, STRSAFE_IGNORE_NULLS, _T(format) _T("\n"), __VA_ARGS__); \
		OutputDebugString(buffer); \
	}