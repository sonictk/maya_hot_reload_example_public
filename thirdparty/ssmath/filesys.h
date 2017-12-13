/**
 * @brief  	This header contains filesystem-related utility functions that
 * 			are intended to work across different operating platforms. It is
 * 			included as part of ``platform.h``.
 */
#ifndef FILESYS_H
#define FILESYS_H

#include "ss_string.h"

// NOTE: (sonictk) We use the Windows implementation since the Linux one suffers
// from the [**Year 2038** issue](https://en.wikipedia.org/wiki/Year_2038_problem)
/// This contains a 64-bit value representing the number of 100-nanosecond
/// intervals since January 1, 1601 (UTC).
typedef u64 FileTime;

// TODO: (sonictk) Maybe add C++ checks for overloads so that this can be a
// C-compatible header as well

/**
 * Gets the directory of the given path.
 *
 * @param path 		The original path to get the directory of.
 * @param dirPath 		The buffer to write the directory to.
 *
 * @return				Returns ``-1`` if the path could not be retrieved; returns
 * 					the length of the directory path on success and sets ``dirPath``
 * 					to the directory path.
 */
int getDirPath(const char *path, char *dirPath);


/**
 * This function retrieves the full file path to the currently running executable.
 * On Windows, this requires linking against ``Kernel32.lib`` and ``Shlwapi.lib``.
 *
 * @param filename		The buffer to store the full file path in. Needs to be
 * 					long enough to store the full path in, else it will be
 * 					truncated.
 * @param len			The number of characters that will be copied to the buffer.
 *
 * @return				The number of characters copied to the buffer.
 * 					On error, returns ``-1``.
 */
inline int getAppPath(char *filename, unsigned int len);


/**
 * This function gets the last modified time of the given ``filename``.
 *
 * @param filename 	The file to find the last modification time for.
 *
 * @return 			The last modified time. On failure or if the file does
 * 					not exist, returns ``-1``.
 */
inline FileTime getLastWriteTime(const char *filename);


inline uint win32TicksToUnixSeconds(dlong win32Ticks)
{
	return (uint)((win32Ticks / WINDOWS_TICK) - SEC_TO_UNIX_EPOCH);
}


inline dlong unixSecondsToWin32Ticks(uint seconds)
{
	return (dlong)((seconds + SEC_TO_UNIX_EPOCH) * WINDOWS_TICK);
}

inline dlong unixSecondsToWin32Ticks(timet seconds)
{
	return unixSecondsToWin32Ticks((uint)seconds);
}


/**
 * This function will rename the ``oldPath`` to the ``newPath``.
 * **This will overwrite the file specified in the new location!**
 *
 * @param oldPath 		The path to the old file to rename.
 * @param newPath 		The path that the new file will have.
 *
 * @return			``0`` on success, a negative value on failure.
 */
inline int renameFile(const char *oldPath, const char *newPath);


#ifdef _WIN32
#include <Shlwapi.h>
#include <strsafe.h>

static const char kPathDelimiter = '\\';
static const char kAltPathDelmiter = '/';
static const unsigned int kMaxPathLen = MAX_PATH;


inline int getAppPath(char *filename, unsigned int len)
{
	DWORD pathLen = GetModuleFileName(NULL, (LPTSTR)filename, (DWORD)len);
	if (pathLen == 0) {
		perror("Failed to get executable path!\n");
		return -1;
	}
	return pathLen;
}


int getDirPath(const char *path, char *dirPath)
{
	StringCchCopy((LPTSTR)dirPath, MAX_PATH, (LPCTSTR)path);
	BOOL result = PathRemoveFileSpec((LPTSTR)dirPath);
	if (result == 0) {
		perror("Path was unmodified!\n");
		return -1;
	}
	size_t strLen;
	StringCchLength(dirPath, STRSAFE_MAX_CCH, &strLen);
	return (int)strLen;
}


inline bool win32IsTimeValid(FILETIME time)
{
	bool result = (time.dwLowDateTime != 0) || (time.dwHighDateTime != 0);
	return result;
}


inline FileTime getLastWriteTime(const char *filename)
{
	FileTime result = -1;

	FILETIME lastWriteTime;
	WIN32_FILE_ATTRIBUTE_DATA data;
	if (GetFileAttributesEx((LPCTSTR)filename, GetFileExInfoStandard, &data)) {
		lastWriteTime = data.ftLastWriteTime;
	} else {
		OSPrintLastError();
		return result;
	}

	result = (FileTime)lastWriteTime.dwHighDateTime << sizeof(DWORD)|lastWriteTime.dwLowDateTime;

	return result;
}


inline int convertPathSeparatorsToOSNative(char *filename)
{
	sizet len = strlen(filename);
	char *tmp = (char *)malloc((len + 1) * sizeof(char));
	int replaced = stringReplace(filename,
								 tmp,
								 kUnixPathSeparator,
								 kPathDelimiter,
								 (unsigned int)len + 1);
	if (replaced <=0) {
		return replaced;
	}

	strncpy(filename, tmp, len + 1);
	free(tmp);

	return replaced;
}


inline int win32RenameFile(const char *oldPath, const char *newPath, unsigned int flags)
{
	BOOL result = MoveFileEx((LPCTSTR)oldPath, (LPCTSTR)newPath, (DWORD)flags);
	if (result == 0) {
		OSPrintLastError();
		return -1;
	}

	return 0;
}

inline int renameFile(const char *oldPath, const char *newPath)
{
	return win32RenameFile(oldPath, newPath, MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
}


#elif __linux__ || __APPLE__
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


static const char kPathDelimiter = '/';
static const unsigned int kMaxPathLen = PATH_MAX;


inline int getAppPath(char *filename, unsigned int len)
{
	ssize_t pathLen = readlink("/proc/self/exe", filename, len);
	if (pathLen == -1) {
		perror("Failed to get executable path!\n");
		return -1;
	}
	return pathLen;
}


int getDirPath(const char *path, char *dirPath)
{
	size_t origPathLen = strlen(path);
	const char *tokenIdx = strrchr(path, kPathDelimiter);
	if (!tokenIdx) {
		perror("Path was unmodified!\n");
		return -1;
	}
	size_t dirPathLen = origPathLen - strlen(tokenIdx);
	strncpy(dirPath, path, dirPathLen);

	return (int)dirPathLen;
}


inline FileTime getLastWriteTime(const char *filename)
{
	FileTime result;

	struct stat attrib = {};
	int statResult = stat(filename, &attrib);
	if (statResult != 0) {
		OSPrintLastError();
	}
	timet mtime = attrib.st_mtim.tv_sec;

	return (FileTime)(unixSecondsToWin32Ticks(mtime));
}


inline int convertPathSeparatorsToOSNative(char *filename)
{
	sizet len = strlen(filename);
	char tmp[len + 1];
	int replaced = stringReplace(filename,
								 tmp,
								 kWin32PathSeparator,
								 kPathDelimiter,
								 (unsigned int)len + 1);
	if (replaced <=0) {
		return replaced;
	}

	strncpy(filename, tmp, len + 1);

	return replaced;
}


inline int renameFile(const char *oldPath, const char *newPath)
{
	int result = rename(oldPath, newPath);
	if (result != 0) {
		OSPrintLastError();
		return -1;
	}

	return 0;
}


#endif // Platform layer


#endif /* FILESYS_H */
