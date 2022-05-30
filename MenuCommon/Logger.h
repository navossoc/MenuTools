#pragma once
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


inline void log_debug(std::wstring&& msg)
{
	OutputDebugString(msg.c_str());
	if(!msg.ends_with(L"\n"))
		OutputDebugString(L"\n");
}

//template<typename Tx>
//inline void log_debug(std::wstring&& f, Tx&& p1)
//{
//	log_debug(std::format(f, p1));
//}

template<typename... Tx>
inline void log_debug(std::wstring&& f, Tx&&... px) {
	//log_debug(std::format(f, std::forward<Tx>(px)...));
}
