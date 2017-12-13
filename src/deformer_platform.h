#ifndef DEFORMER_PLATFORM_H
#define DEFORMER_PLATFORM_H

#include <ssmath/platform.h>
#include <ssmath/vector_math.h>
#include <limits.h>

/// This is the prototype for the function that will be dynamically hotloaded.
typedef Vec3 (*DeformFunc)(Vec3&, float);


/// This is initialized to the path of the deformer's **business logic** DLL
/// whenever the plugin is initialized.
globalVar MString kPluginLogicLibraryPath;


#ifdef _WIN32
globalVar const char *kDeformerLogicLibraryName = "logic.dll";

#elif __linux__ || __APPLE__
globalVar const char *kDeformerLogicLibraryName = "logic.so";

#endif // Library filename


enum LibraryStatus
{
	LibraryStatus_Failure = INT_MIN,
	LibraryStatus_InvalidLibrary,
	LibraryStatus_InvalidSymbol,
	LibraryStatus_InvalidHandle,
	LibraryStatus_UnloadFailure,
	LibraryStatus_Success = 0
};


/// This is a data structure that contains information about the state of a DLL
/// that contains all the so-called *business logic* required for the deformer
/// to do its work.
struct DeformerLogicLibrary
{
	DLLHandle handle;
	FileTime lastModified;

	DeformFunc deformCB;
	bool isValid;
};


/// This is the global reference to the *business logic* DLL that is loaded.
globalVar DeformerLogicLibrary kLogicLibrary = {};


/**
 * This function gets the full path to the *business logic* DLL. This file may/may
 * not exist on disk yet at the time this path is formatted.
 *
 * @param pluginPath	The path to the host Maya plugin DLL. Must use the OS-specific
 * 					path separators.
 *
 * @return				The path to the *business logic* DLL.
 */
MString getDeformerLogicLibraryPath(const char *pluginPath);


LibraryStatus loadDeformerLogicDLL(DeformerLogicLibrary &library);


LibraryStatus unloadDeformerLogicDLL(DeformerLogicLibrary &library);


#endif /* DEFORMER_PLATFORM_H */
