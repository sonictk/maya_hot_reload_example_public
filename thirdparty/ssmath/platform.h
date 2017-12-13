/**
 * @brief		This monolithic header contains common program functionality
 * 			that is intended to work across different operating systems.
 * 			If only basic types are desired without the full range of OS
 * 			functionality (along with the dependencies that brings), define
 * 			``PLATFORM_LEAN`` somewhere.
 */
#ifndef PLATFORM_H
#define PLATFORM_H


/// For unity builds, this definition is used for easy grepping.
#define globalVar static
/// For non-unity builds, see above
#define localVar static


#ifdef __cplusplus

#define Shared extern "C"
#define Import extern "C"

#endif // __cplusplus


/// DLL machinery types
#ifdef _WIN32
#include <Windows.h>

#define DLLExport __declspec(dllexport)
#define DLLImport __declspec(dllimport)

typedef HMODULE DLLHandle;
typedef FARPROC FuncPtr;

#elif __linux__ || __APPLE__

// NOTE: (sonictk) This will only work on GCC/Clang
#define DLLExport __attribute__ ((visibility ("default")))
#define DLLImport __attribute__ ((visibility ("default")))

typedef void * DLLHandle;
typedef void * FuncPtr;

#endif /* Platform layer for DLL machinery */


// NOTE: (yliangsiew) C++11 support is required for ``cstdint``
#if __cplusplus >= 201103L
#include <cstdint>

using std::uint8_t;
using std::int8_t;

using std::uint16_t;
using std::int16_t;

using std::uint32_t;
using std::int32_t;

using std::uint64_t;
using std::int64_t;

using std::size_t;

#include <ctime>
using std::time_t;

#include <cstdlib>
#include <cstdio>
using std::rand;
using std::srand;
using std::perror;

using std::malloc;
using std::free;

#else
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#endif // __cplusplus >= 201103L


/// Aliases for basic size types
typedef short int sint;
typedef unsigned short int usint;

typedef unsigned int uint;

typedef uint8_t uint8;
typedef uint8 u8;
typedef int8_t int8;
typedef int8 i8;

typedef uint16_t uint16;
typedef uint16 u16;
typedef int16_t int16;
typedef int16 i16;

typedef uint32_t uint32;
typedef uint32 u32;
typedef int32_t int32;
typedef int32 i32;

typedef uint64_t uint64;
typedef uint64 u64;
typedef int64_t int64;
typedef int64 i64;

typedef size_t sizet;

typedef time_t timet;

typedef long long dlong;
typedef unsigned long long udlong;


/// Standard OS file separators for filesystems
globalVar const char kWin32PathSeparator = '\\';
globalVar const char kUnixPathSeparator = '/';


/// This is the difference between the Windows and Unix epoch start times.
/// Windows: 1601-01-01T00:00:00Z
/// Linux: 1970-01-01T00:00:00Z
#define WINDOWS_TICK 10000000 // NOTE: (sonictk) Windows uses 100ns intervals
#define SEC_TO_UNIX_EPOCH 11644473600LL


// Platform layer includes other common OS functionality here.
// Anything that requires linking to additional library dependencies/anything
// beyond the **C99/C++03 standard** should **not** be defined in the lean configuration.
#include "ss_stream.h"
#include "ss_string.h"

#ifndef PLATFORM_LEAN

#include "timer.h"		// NOTE: (yliangsiew) On linux, requires C++11 support
#include "library.h" 	// NOTE: (yliangsiew) Requires linking with ``dl`` on Linux
#include "filesys.h" 	// NOTE: (yliangsiew) Requires linking with ``Shlwapi.dll`` on Windows

#endif // PLATFORM_LEAN


#endif /* PLATFORM_H */
