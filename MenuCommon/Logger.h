// Includes
#include <tchar.h>

// Defines
#define MAX_BUFFER				1000
#define LOGMESSAGE(format, ...)		\
		{ \
		TCHAR buffer[MAX_BUFFER+1]; \
		_sntprintf_s(buffer, MAX_BUFFER, _TRUNCATE, _T(format) _T("\n"), __VA_ARGS__); \
		OutputDebugString(buffer); \
		}