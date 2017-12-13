#include "deformer_platform.h"
#include <maya/MString.h>
#include <maya/MGlobal.h>


MString getDeformerLogicLibraryPath(const char *pluginPath)
{
	if (strlen(pluginPath) <= 0) {
		return MString();
	}
	char pathDelimiter[2] = {kPathDelimiter, '\0'};
	MString delimiter(pathDelimiter);
	MString pluginPathStr(pluginPath);
	MString libFilename = pluginPathStr + delimiter + kDeformerLogicLibraryName;

	return libFilename;
}


LibraryStatus loadDeformerLogicDLL(DeformerLogicLibrary &library)
{
	const char *libFilenameC = kPluginLogicLibraryPath.asChar();

	FileTime lastModified = getLastWriteTime(libFilenameC);
	library.lastModified = lastModified;

	DLLHandle handle = loadSharedLibrary(libFilenameC);
	if (!handle) {
		MGlobal::displayError("Unable to load logic library!");
		library.handle = NULL;
		library.lastModified = {};
		library.isValid = false;

		return LibraryStatus_InvalidLibrary;
	}

	library.handle = handle;

	FuncPtr getValueFuncAddr = loadSymbolFromLibrary(handle, "getValue");
	if (!getValueFuncAddr) {
		MGlobal::displayError("Could not find symbols in library!");
		return LibraryStatus_InvalidSymbol;
	}

	library.deformCB = (DeformFunc)getValueFuncAddr;
	library.isValid = true;

	MGlobal::displayInfo("Loaded library from: " + kPluginLogicLibraryPath);

	return LibraryStatus_Success;
}


LibraryStatus unloadDeformerLogicDLL(DeformerLogicLibrary &library)
{
	if (!kLogicLibrary.isValid) {
		return LibraryStatus_InvalidHandle;
	}
	int unload = unloadSharedLibrary(kLogicLibrary.handle);
	if (unload != 0) {
		MGlobal::displayError("Unable to unload shared library!");
		return LibraryStatus_UnloadFailure;
	}

	library.deformCB = NULL;
	library.lastModified = {};
	library.isValid = false;

	return LibraryStatus_Success;
}
