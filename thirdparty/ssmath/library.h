#ifndef LIBRARY_H
#define LIBRARY_H


// NOTE: (yliangsiew) All DLL platform-related machinery goes here
#ifdef _WIN32

inline DLLHandle loadSharedLibrary(const char *filename)
{
	DLLHandle libHandle = LoadLibrary((LPCTSTR)filename);
	if (!libHandle) {
		OSPrintLastError();
		return NULL;
	}

	return libHandle;
}


inline int unloadSharedLibrary(DLLHandle handle)
{
	if (!handle) {
		perror("The handle is not valid! Cannot unload!\n");
		return -1;
	}

	BOOL result = FreeLibrary(handle);
	if (result == 0) {
		OSPrintLastError();
		return -2;
	}

	return 0;
}


inline FuncPtr loadSymbolFromLibrary(DLLHandle handle, const char *symbol)
{
	FuncPtr symbolAddr = GetProcAddress(handle, (LPCSTR)symbol);
	if (!symbolAddr) {
		OSPrintLastError();
	}

	return symbolAddr;
}


#elif __linux__ || __APPLE__
#include <dlfcn.h>

inline DLLHandle loadSharedLibrary(const char *filename, int flags)
{
	DLLHandle libHandle = dlopen(filename, flags);
	if (!libHandle) {
		char *errMsg = dlerror();
		fprintf(stderr, "Failed to load library %s: %s!\n", filename, errMsg);
		return NULL;
	}

	return libHandle;
}

inline DLLHandle loadSharedLibrary(const char *filename)
{
	DLLHandle libHandle = loadSharedLibrary(filename, RTLD_LAZY);
	return libHandle;
}


inline int unloadSharedLibrary(DLLHandle handle)
{
	if (!handle) {
		perror("The handle is not valid! Cannot unload!\n");
		return -1;
	}

	int result = dlclose(handle);
	if (result != 0) {
		char *errMsg = dlerror();
		fprintf(stderr, "Could not free library! %s\n", errMsg);
	}

	return 0;
}


inline FuncPtr loadSymbolFromLibrary(DLLHandle handle, const char *symbol)
{
	if (!handle) {
		perror("The given handle was not valid!\n");
		return NULL;
	}

	void *symbolAddr = dlsym(handle, symbol);
	if (symbolAddr == NULL) {
		// NOTE: (yliangsiew) Following recommendation of Linux manual; clear the
		// old error code, call ``dlsym()`` again, and then call ``dlerror()``
		// again since the symbol returned could actually be ``NULL``.
		dlerror();
		dlsym(handle, symbol);
		char *errMsg = dlerror();
		if (!errMsg) {
			return symbolAddr;
		}
		fprintf(stderr, "Unable to find symbol: %s! %s\n", symbol, errMsg);

		return NULL;
	}

	return symbolAddr;
}

#endif // Exports platform layer


#endif /* LIBRARY_H */
