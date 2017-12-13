# Getting into it #

Picking up from the previous chapter, we got both a ``logic.dll`` and a
``maya_hot_reload_example.dll`` compiled and working, and (hopefully) loadable
by Maya. Great!

Now let's dive into what else the deformer needs to do:

> ... we need a second DLL
> to handle the business logic of the deformation. **This client DLL will be
> reloaded by the Host DLL every time the timestamp on it changes**, and the
> function pointers fixed up every time this happens.

OK, let's focus on that next!


## Checking file modified timestamps ##

Rather than write some generic file-timestamp checking functions, let's take a
more data-oriented approach and *write the client code first*. What this means
is basically, "what would the code for such a thing even look like at the end of
the day?"

First, we're going to need to load the logic DLL. We're probably going to want
to do this the moment the plugin is loaded, otherwise we wouldn't be able to do
anything anyway. The ``postConstructor()`` virtual method is one possibility to
utilize to achieve this goal:

```c++
/// This is the global reference to the *business logic* DLL that is loaded.
static DeformerLogicLibrary kLogicLibrary = {};


void HotReloadableDeformer::postConstructor()
{
	LibraryStatus result = loadDeformerLogicDLL(kLogicLibrary);
	if (result != LibraryStatus_Success) {
		MGlobal::displayError("Failed to load shared library!");
		return;
	}

	return;
}
```

What is a ``LibraryStatus``? It's nothing more complicated than a status code.

```c++
enum LibraryStatus
{
	LibraryStatus_Failure = INT_MIN,
	LibraryStatus_InvalidLibrary,
	LibraryStatus_InvalidSymbol,
	LibraryStatus_InvalidHandle,
	LibraryStatus_UnloadFailure,
	LibraryStatus_Success = 0
};
```

And what is a ``DeformerLogicLibrary``? It's a simple creature too:

```c++
typedef MVector (*DeformFunc)(MVector&, float);

struct DeformerLogicLibrary
{
	DLLHandle handle;
	FileTime lastModified;

	DeformFunc deformCB;
	bool isValid;
};
```

As you can see, it's a mere data structure with pointers to the DLL handle, some
sort of ``FileTime`` thing that basically tells us when the DLL was last
modified (so that we know if we need to reload it), a function pointer to the
actual deformation business logic, and a boolean we can query to check if the
data is valid. Note that the signature of the function that the function pointer
points to matches the ``getValue`` function that we previously defined.

Let's focus now on what ``loadDeformerLogicDLL`` will actually do:

```c++
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
```

Ok, a couple of things here: what is ``kPluginLogicLibraryPath``? It's basically
the path to the logic library, which we'll assume to be in the same plugin as
the host DLL. Thus, we can retrieve it easily:

```c++
MString pluginPath = plugin.loadPath();
const char *pluginPathC = pluginPath.asChar();
const sizet lenPluginPath = strlen(pluginPathC);
char OSPluginPath[kMaxPathLen];
strncpy(OSPluginPath, pluginPathC, lenPluginPath + 1);
int replaced = convertPathSeparatorsToOSNative(OSPluginPath);
if (replaced < 0) {
    MGlobal::displayError("Failed to format path of plugin to OS native version!");
    return MStatus::kFailure;
}
if (strlen(OSPluginPath) <= 0) {
    MGlobal::displayError("Could not find a path to the plugin!");
    return MStatus::kFailure;
}

kPluginLogicLibraryPath = getDeformerLogicLibraryPath(OSPluginPath);
```

The source code for ``convertPathSeparatorsToOSNative`` looks roughly like this:

```c++
static const char kWin32PathSeparator = '\\';
static const char kPathDelimiter = '\\';


inline int stringReplace(const char *input,
						 char *output,
						 const char token,
						 const char replace,
						 unsigned int size)
{
	sizet len = strlen(input);
	if (len <= 0) {
		return 0;
	}
	int replaced = 0;
	unsigned int i = 0;
	for (; i < size && i < (len + 1) && input[i] != '\0'; ++i) {
		if (input[i] == token) {
			output[i] = replace;
			replaced++;
		} else {
			output[i] = input[i];
		}
	}
	output[i] = '\0';

	return replaced;
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
```

And that of ``getDeformerLogicLibraryPath``:

```c++
#ifdef _WIN32
globalVar const char *kDeformerLogicLibraryName = "logic.dll";

#elif __linux__ || __APPLE__
globalVar const char *kDeformerLogicLibraryName = "logic.so";

#endif // Library filename

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
```

We basically have a bog-standard character-replacement function (since Maya
returns paths with Unix path separators by default) that helps us format a
Windows path to the ``logic.dll`` file.

Now, let's take a look at what ``getLastWriteTime`` looks like:

### Windows ###

On Windows, we make use of
the
[``GetFileAttributesEx``](https://msdn.microsoft.com/en-us/library/windows/desktop/aa364946%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396)
call to retrieve the file's attributes, and get the last modified time from there.

```c++
typedef uint_64t FileTime;


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
```

!!! note "Why not use ``GetFileTime``?"
    For those of you more familiar with the Win32 API, you might have noticed
    the existence of a ``GetFileTime`` function that seems to do the same thing
    we do here, except with less typing. The reason we don't use it is that it
    requires a handle to the file, while ``GetFileAttributesEx`` does not.

The ``FILETIME`` structure on Windows is essentially two 32-bit integers smushed
together, so we do some work to cast it to a single 64-bit value instead. For
platform compatbility's sake, we call that a ``FileTime`` data type of our own.

On Windows, ``OSPrintLastError`` looks like this:

```c++
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
```

(Yes, it's kind of a little silly how much you need to type to actually print a
system exception string.)

On Windows, each tick is a 100-nanosecond interval, which is significantly more
granular than Linux's default legacy behaviour, which measures in 1-second
intervals. Windows also starts its measurement from a different **epoch** than
Linux; Windows uses ``1601-01-01T00:00:00Z`` as its start date, while Linux uses
``1970-01-01T00:00:00Z`` instead. This means that it's useful to define some
constants/functions for converting between the two:

```c++
#define WINDOWS_TICK 10000000 // NOTE: (sonictk) Windows uses 100ns intervals
#define SEC_TO_UNIX_EPOCH 11644473600LL

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
```

!!! tip "Crossing the platforms"
    Why do we favour the Windows implementation when it comes to file times?
    Well, the Linux one suffers from
    the [**Year 2038** issue](https://en.wikipedia.org/wiki/Year_2038_problem)
    due to its use of a single signed 32-bit integer, unlike Windows. This is
    rectified in newer versions of the kernel, but for backwards compatibility
    reasons, any code that relies on the legacy behaviour will fail to work in
    the future.


### Linux ###

With that out of the way, the Linux implementation will look similar to the following:

```c++
inline void OSPrintLastError()
{
	perror(strerror(errno));
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
```

Instead of ``GetFileAttributesEx``, which is a Windows-specific function, we
make use of [``stat``](https://linux.die.net/man/2/stat) instead to get
ourselves a ``time_t`` structure that we can then extract the ``st_mtim`` field
from, which is the last modified time.

Ok, we can now get file times of modified files on disk through code. Let's take
a look at what those mysterious ``loadSharedLibrary`` and ``loadSymbolFromLibrary``
functions do next.


## Loading a shared library ##

### Windows ###

On Windows, we load a shared libary using the function call [``LoadLibrary``](https://msdn.microsoft.com/en-us/library/windows/desktop/ms684175(v=vs.85).aspx),
funnily enough! This makes implementation fairly trivial:

```c++
inline DLLHandle loadSharedLibrary(const char *filename)
{
	DLLHandle libHandle = LoadLibrary((LPCTSTR)filename);
	if (!libHandle) {
		OSPrintLastError();
		return NULL;
	}

	return libHandle;
}
```

Let's do the unload equivalent as well using another function called
``FreeLibrary``, which unloads the DLL from memory.

```c++
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
```

Now that we can load a DLL into memory, we need a way to inspect that DLL and
find the address of a given symbol that we're interested in (like ``getValue``!)
Let's see what that looks like in code:

```c++
inline FuncPtr loadSymbolFromLibrary(DLLHandle handle, const char *symbol)
{
	FuncPtr symbolAddr = GetProcAddress(handle, (LPCSTR)symbol);
	if (!symbolAddr) {
		OSPrintLastError();
	}

	return symbolAddr;
}
```

We use the
function
[``GetProcAddress``](https://msdn.microsoft.com/en-us/library/windows/desktop/ms683212%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396)
in order to perform run-time dynamic linking, getting the address of the symbol
that we're interested in. Thankfully, this entire process is fairly straightforward.

### Linux ###

On Linux, the process is very similar, except we use ``dlopen`` and ``dlclose``
instead for our OS library calls:

```c++
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
```

We specify the ``RTLD_LAZY`` flag when loading our ``.so`` since we only want to
resolve our ``getValue`` symbol when code that references it is executed; if no
code ever references it, well, why would we want to waste time performing the
binding of symbols otherwise?

When it comes to loading symbols at run-time, ``dlsym()`` is our function of
choice, however, Linux is slightly more obtuse when it comes to error-checking:

```c++
inline FuncPtr loadSymbolFromLibrary(DLLHandle handle, const char *symbol)
{
	if (!handle) {
		perror("The given handle was not valid!\n");
		return NULL;
	}

	void *symbolAddr = dlsym(handle, symbol);
	if (symbolAddr == NULL) {
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
```
Basically, if the returned address was ``NULL``, we clear the global status
code, call the ``dlsym`` function again, and check what the message is, since
the returned symbol might legimately *be* a null pointer.


## Putting things together ##

So we've now got our ``loadDeformerLogicDLL`` sorted out, let's write the
symmetric ``unloadDeformerLogicDLL`` version:

```c++
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
```

Basically, we check if the library given is valid, and unload it, thereafter
clearing it out and re-initializing it to default values.

We can now update the main ``deform()`` function to reload the DLL every time it
changes:

```c++
MStatus HotReloadableDeformer::deform(MDataBlock &block,
									  MItGeometry &iter,
									  const MMatrix &matrix,
									  unsigned int multiIndex)
{
	LibraryStatus status;

	if (!kLogicLibrary.isValid) {
		unloadDeformerLogicDLL(kLogicLibrary);
		status = loadDeformerLogicDLL(kLogicLibrary);
		if (status != LibraryStatus_Success) {
			return MStatus::kFailure;
		}
	}

	if (kPluginLogicLibraryPath.numChars() == 0) {
		return MStatus::kFailure;
	}

	FileTime lastModified = getLastWriteTime(kPluginLogicLibraryPath.asChar());
	if (lastModified >= 0 && lastModified != kLogicLibrary.lastModified) {
		status = unloadDeformerLogicDLL(kLogicLibrary);
		if (status != LibraryStatus_Success) {
			return MStatus::kFailure;
		}
		status = loadDeformerLogicDLL(kLogicLibrary);
		if (status != LibraryStatus_Success) {
			return MStatus::kFailure;
		}
	}

	MStatus result;

	MDataHandle envelopeHandle = block.inputValue(envelope, &result);
	CHECK_MSTATUS_AND_RETURN_IT(result);

	float envelope = envelopeHandle.asFloat();

	for (; !iter.isDone(); iter.next())
	{

		MPoint curPtPosPt = iter.position();
		MPoint finalPosPt = kLogicLibrary.deformCB(curPtPos, envelope);

		iter.setPosition(finalPosPt);
	}

	return result;
}
```

Things might make more sense now. We check the last modified timestamp of the
current DLL file on disk and if it is newer than the one we have loaded in
memory, we unload the current one and load the new one instead. We modify the
deformation function to call our function pointer that we fixed up in the
``loadDeformerLogicDLL`` function as well.

We're almost done, but we've got some unfinished business to attend to in both
our plugin entry/exit points:

```c++
MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, kAUTHOR, kVERSION, kREQUIRED_API_VERSION);

	MString pluginPath = plugin.loadPath();
	const char *pluginPathC = pluginPath.asChar();
	const sizet lenPluginPath = strlen(pluginPathC);
	char OSPluginPath[kMaxPathLen];
	strncpy(OSPluginPath, pluginPathC, lenPluginPath + 1);
	int replaced = convertPathSeparatorsToOSNative(OSPluginPath);
	if (replaced < 0) {
		return MStatus::kFailure;
	}
	if (strlen(OSPluginPath) <= 0) {
		return MStatus::kFailure;
	}

	kPluginLogicLibraryPath = getDeformerLogicLibraryPath(OSPluginPath);

	status = plugin.registerNode(kHotReloadableDeformerName,
								 kHotReloadableDeformerID,
								 &HotReloadableDeformer::creator,
								 &HotReloadableDeformer::initialize,
								 MPxNode::kGeometryFilter);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}


MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);
	MStatus status;

	if (kLogicLibrary.isValid && kLogicLibrary.handle) {
		unloadDeformerLogicDLL(kLogicLibrary);
	}

	status =  plugin.deregisterNode(kHotReloadableDeformerID);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}
```

Basically, we get the path to the ``logic.dll`` and store it as a global
variable when our plugin is loaded (so we don't need to keep computing it each
time in the ``deform`` function), and unload it when the plugin is unloaded.

At this point, our high-level implementation is complete. You can go ahead and
load the plugin, change the ``logic.cpp`` file and try to re-complile (and if
you're on Linux, things will probably work!) However, as they say, the devil is
in the details, and there are two Windows-specific issues that we need to deal
with before we can reach a true hot-reloadable workflow.


## Dealing with Windows ##

### DLL file handle lock ###

You might have noticed the following error message in your build output when you
tried to run the ``build.bat`` script while the plugin was loaded in Maya:

```
Compiling Logic...
logic.cpp
Linking Logic...
LINK : fatal error LNK1104: cannot open file 'C:\Users\sonictk\Git\experiments\maya_hot_reload_example\msbuild\logic.dll
```

Uh-oh. Why is the linker complaining that it cannot open the business logic DLL?

Basically, when we called ``LoadLibrary`` earlier, Windows will *helpfully* lock
the file handle to it until it detects that there are no references left to it,
either through the use of ``FreeLibrary`` calls (or all applications that
reference it have crashed!) or otherwise. What this means is that while the DLL
is in use, we can't overwrite it in-place.

However...renaming a DLL file while it's in use does not invalidate the file
handle to it. Thus:

```batch
if exist %OutputLogicDLLFilename% (rename %OutputLogicDLLFilename% %OutputLogicDLLTempFilename%)

REM after compilation and everything...

timeout /t 1 > NUL

echo Deleting temporary artifacts...
if exist %OutputLogicDLLTempFilename% del %OutputLogicDLLTempFilename%
```

We put a artificial wait time of 1 second to give Maya enough time to load the
new library before deleting the old one; unlike renaming, deleting a DLL while
it's in use basically means undefined behaviour the next time an app requests
memory to/from it, and almost always results in a crash.

You can try now; things work as expected; we're able to modify ``logic.cpp``,
re-run the build script, and watch the deformer update in real-time as the
``deform`` function loads the new version of our library!

!!! tip "Crossing the platforms"

    Why doesn't this problem occur on Linux? Basically, on Linux, when ``.so``s
    are loaded into memory, we are able to overwrite the original file handle
    since the mechanisms for reference counting work a little differently: the
    *directory entry* for the file gets removed, but any exsting processes that
    use the file *still have access to the file itself*. Only when the reference
    count reaches zero does the file finally get deleted automatically by the
    OS. Linux does actually lock something called
    the [inode](https://en.wikipedia.org/wiki/Inode), which remains untouched,
    since all we deleted was merely the link to it.


### PDB lock ###

One other problem is the issue of **PDB lock**; if we launched Maya under the
control of the Visual Studio debugger, or attached the debugger to Visual Studio
while we were working with our plugin, we might get an error during compilation
where the PDB file instead is the one being locked. What's happening is that
Visual Studio will automatically lock any PDBs that are loaded during a
debugging session and *will keep them locked indefinitely until the end of the
debugging session*. (This lock will persist even if you unload the DLL) This
[post](http://ourmachinery.com/post/little-machines-working-together-part-2/)
here details some possible solutions to this issue; we'll be making use of the
third one, which is to generate a random filename for the PDB file:

```batch
set CommonLinkerFlagsLogic=/PDB:"%BuildDir%\logic_%random%.pdb" /IMPLIB:"%BuildDir%\logic.lib" /OUT:"%BuildDir%\logic.dll" %BuildDir%\logic.obj
```
The ``%random%`` macro on Windows will generate a random string of numbers for
us, which will then be used in the resulting PDB's filename. What happens here
is that the ``logic.dll`` will now have the path to this new ``.pdb`` "baked"
inside its debug section; the Visual Studio debugger will lock that file when a
debugging session is initiated. When we re-compile the PDB file, we generate a
new one and point to that one instead in the DLL, thus repeating the process
and allowing us to get past the PDB lock.

!!! note "How the debugger finds ``.pdb`` files"
    I recommend reading up on how the Visual Studio
    debugger
    [finds ``.pdb`` files](https://msdn.microsoft.com/en-us/library/ms241613.aspx)
    in order to get a better understanding of the behaviour that's happening. Of
    course, keep in mind that this is Windows-specific; neither ``gdb`` nor
    XCode exhibit this behaviour.

## Is it working? ##

If you managed to get past all that, congratulations! You should be able to have
hot-reloadable code working in your Maya deformer! If you're stuck somewhere,
take a look at the repository's code to see where you might have gone a bit
off-base. (be warned, however, that the code there is organized a little bit
differently from what you might have seen in this tutorial)

Good luck, and I hope you've found this technique interesting (and perhaps useful)!
