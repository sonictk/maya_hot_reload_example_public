#ifndef SS_STREAM_H
#define SS_STREAM_H

/**
 * This function prints the last error message that was set from an OS call.
 */
inline void OSPrintLastError();

#ifdef _WIN32
#include <Strsafe.h>

inline void OSPrintLastError()
{
	char errMsg[256];
	DWORD errCode = GetLastError();
	if (errCode == 0) {
		return;
	}
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
				   NULL,
				   errCode,
				   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				   (LPTSTR)errMsg,
				   sizeof(errMsg),
				   NULL);

	perror(errMsg);
}

#elif __linux__ || __APPLE__
#include <errno.h>
#include <string.h>

inline void OSPrintLastError()
{
	perror(strerror(errno));
}


#endif // Platform Layer


#endif /* SS_STREAM_H */
