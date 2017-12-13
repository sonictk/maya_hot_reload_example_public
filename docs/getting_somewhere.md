# Getting somewhere #

Ok, there's been a lot of text and diagrams. How about we finally start writing
some code?

!!! warning
    Throughout the following code samples, I am purposely *not* writing
    ready-to-use code; if you want that, go to the Git repository itself and
    look at the source files there. I am abbreviating code here for both
    legibility reasons and also because I would rather you attempt to implement
    the code yourself, rather than cargo-culting it wholesale and learning
    nothing in the process.

## Making the basic plug-in setup ##

Firstly, we'll just get a basic skeleton setup of the plugin going. We
know we're going to create a deformer plugin here, so that means we're going to
need a ``MPxGeometryFilter`` or ``MPxDeformerNode`` type of node. For
simplicity's sake, we'll go with a ``MPxGeometryFilter``. As a refresher,
you first need to create a defintion that implements a creator
function and an initializer function, which we will call ``creator()`` and
``initialize()`` respectively.

Thus, in ``deformer.h``:

```c++
// We'll use these to help us identify the node later on
static const MTypeId kHotReloadableDeformerID = 0x0008002E;
static const char *kHotReloadableDeformerName = "hotReloadableDeformer";

// Remember, in C++, a struct is the same thing as a class, except you type fewer
// access specifiers!
struct HotReloadableDeformer : MPxGeometryFilter
{
	static void *creator();

	static MStatus initialize();

	MStatus deform(MDataBlock &block,
				   MItGeometry &iterator,
				   const MMatrix &matrix,
				   unsigned int multiIndex);
}
```

And ``deformer.cpp``, which, for now, looks pretty sparse:

```c++
#include "deformer.h"

void *HotReloadableDeformer::creator()
{
	return new HotReloadableDeformer;
}


MStatus HotReloadableDeformer::initialize()
{
	MStatus result;
	return result;
}


MStatus HotReloadableDeformer::deform(MDataBlock &block,
									  MItGeometry &iter,
									  const MMatrix &matrix,
									  unsigned int multiIndex)
{
	MStatus result;
	for (; !iter.isDone(); iter.next())
	{

		MPoint curPtPosPt = iter.position();
		iter.setPosition(curPtPosPt);
	}

	return result;
}
```
Great! We've got our node now, let's write the basic plugin structure to
register it. In case you needed a refresher:

In ``plugin_main.cpp``:

```c++
#include "plugin_main.h"
#include <maya/MFnPlugin.h>

const char *kAUTHOR = "Me, the author";
const char *kVERSION = "1.0.0";
const char *kREQUIRED_API_VERSION = "Any";


MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, kAUTHOR, kVERSION, kREQUIRED_API_VERSION);

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

	status =  plugin.deregisterNode(kHotReloadableDeformerID);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}
```

In ``plugin_main.h``, I go ahead and setup a [Single Translation Unit (STU)
Build/Unity build](https://stackoverflow.com/questions/543697/include-all-cpp-
files-into-a-single-compilation-unit) (which is easy, since there's only one
source file right now):

```c++
#include "deformer.cpp"

MStatus initializePlugin(MObject obj);

MStatus uninitializePlugin(MObject obj);
```

!!! note
    I will not go into the details regarding a STU build here, but suffice to say
    that I have found them much more beneficial to build times than any other
    compiler feature (LTO, IncrediBuild, splitting the code out into pre-compiled
    libs, whatever). For such a small project, it doesn't matter; you can switch back
    to a more traditional build setup if you prefer.


## Writing the build script

For building this plugin, I'm going to go off-the-rails from what I usually do
in my other tutorials and use a ``build.bat`` file. Yes, you read that right,
we're *not* using CMake for once!

...That comes later. For now, I'd like us to focus on *what's actually
happening*, rather than dealing with both that *and* the frustration of trying
to get CMake to do what we want. (And it's good practice to be able to write in
the scripting language of the OS that you actually use! Trust me, if not for the
Visual Studio project generation feature of CMake, I'd be using batch scripts on
Windows anyway.)

!!! tip "Crossing the platforms"
    For Linux/OSX users, you should (hopefully) be familiar enough with Bash
    scripting and GCC/Clang to write the equivalent commands as needed. If not,
    please try to follow along for now or refer to the ``CMakeLists.txt`` in the
    repository and convert that to your own build script as needed later on.

What does this mean exactly? Well, we'll need to create a ``build.bat`` file,
for one thing. If you're used to building from within the Visual Studio IDE
itself instead of from the command line, let's start from the very beginning
here and work our way through it step-by-step:

```batch
@echo off
REM    Set up the Visual Studio environment variables for calling the MSVC compiler
call "%vs2017installdir%\VC\Auxiliary\Build\vcvarsall.bat" x64

REM    Or maybe you're on VS 2015? Call this instead:
REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

REM    Make a build directory to store artifacts; remember, %~dp0 is just a special
REM    FOR variable reference in Windows that specifies the current directory the
REM    batch script is being run in
if not exist %~dp0msbuild mkdir %~dp0msbuild
pushd %~dp0msbuild

REM    Obviously, change these to point to the proper locations for your filesystem
set MayaRootDir=C:\Program Files\Autodesk\Maya2016
set MayaIncludeDir=%MayaRootDir%\include

set HostEntryPoint=%~dp0src\plugin_main.cpp

set OutputHostMLLFilename=%BuildDir%\maya_hot_reload_example.mll
```

So far, so good: we first set up the Visual Studio environment by calling a
convenience batch script (which will allow us to call ``cl.exe`` and
``link.exe`` from the command line directly, which are the MSVC compiler and
linker respectively), basically create a ``msbuild`` directory if it doesn't
exist, and set some basic constants up.

Let's add a bit of code just to check if the user typed ``debug`` or ``release``
on the command line, so that we can build different configurations of the plugin
as needed.

```batch
REM Process command line arguments
set BuildType=%1
if "%BuildType%"=="" (set BuildType=release)
```

*Now* it gets a little gnarly:

```batch
REM    Setup all the compiler flags
set CommonCompilerFlags=/c /MP /W3 /WX- /Gy /Zc:wchar_t /Zc:forScope /Zc:wchar_t /Zc:forScope /Zc:inline /openmp /fp:precise /nologo /EHsc /MD /D REQUIRE_IOSTREAM /D _CRT_SECURE_NO_WARNINGS /D _BOOL /D NT_PLUGIN /D _WINDLL /D _MBCS /Gm- /GS /Gy /Gd /TP

REM    Add the include directories for header files
set CommonCompilerFlags=%CommonCompilerFlags% /I"%MayaRootDir%\include"

set CommonCompilerFlagsDebug=/Zi /Od %CommonCompilerFlags%
set CommonCompilerFlagsRelease=/O2 %CommonCompilerFlags%

set CompilerFlagsHostDebug=%CommonCompilerFlagsDebug% %HostEntryPoint%
set CompilerFlagsHostRelease=%CommonCompilerFlagsRelease% %HostEntryPoint%
```

*Holy compiler flags Batman*, what is that wall of options? Basically, you can
[take a look yourself](https://msdn.microsoft.com/en-us/library/fwkeyyhe.aspx),
but it's essentially the options we need to build a Maya plugin DLL. The
important option (well, all of them are *technically* important) is the ``/I``
include directory option, which tells the compiler where to look for our ``.h``
header files during compilation; we set that to the Maya headers include
directory.

We also specify two different sets of compilation flags, one for **Release**
builds, and one for **Debug** builds, which both specify different optimization
options (the debug build will generate debugging information in the binary).

Hopefully that made sense, because the flags for the linker aren't any less complicated:

```batch
REM    Setup all the linker flags
set CommonLinkerFlags= /NOLOGO /INCREMENTAL:no /OPT:REF /MANIFEST /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /manifest:embed /SUBSYSTEM:CONSOLE /TLBID:1 /DYNAMICBASE /NXCOMPAT /MACHINE:X64  /machine:x64 /DLL

REM    Add all the Maya libraries to link against
set CommonLinkerFlags=%CommonLinkerFlags% "%MayaRootDir%\lib\OpenMaya.lib" "%MayaRootDir%\lib\OpenMayaAnim.lib" "%MayaRootDir%\lib\OpenMayaFX.lib" "%MayaRootDir%\lib\OpenMayaRender.lib" "%MayaRootDir%\lib\OpenMayaUI.lib" "%MayaRootDir%\lib\Foundation.lib" "%MayaRootDir%\lib\clew.lib" "%MayaRootDir%\lib\OpenMaya.lib" "%MayaRootDir%\lib\Image.lib" "%MayaRootDir%\lib\Foundation.lib" "%MayaRootDir%\lib\IMFbase.lib" "%MayaRootDir%\lib\OpenMaya.lib" "%MayaRootDir%\lib\OpenMayaAnim.lib" "%MayaRootDir%\lib\OpenMayaFX.lib" "%MayaRootDir%\lib\OpenMayaRender.lib" "%MayaRootDir%\lib\OpenMayaUI.lib" "%MayaRootDir%\lib\clew.lib" "%MayaRootDir%\lib\Image.lib" "%MayaRootDir%\lib\IMFbase.lib"

REM    Now add the OS libraries to link against
set CommonLinkerFlags=%CommonLinkerFlags% Shlwapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib

set CommonLinkerFlagsDebug=%CommonLinkerFlags% /DEBUG
set CommonLinkerFlagsRelease=%CommonLinkerFlags%

set CommonLinkerFlagsHost=/PDB:"%BuildDir%\maya_hot_reload_example.pdb" /IMPLIB:"%BuildDir%\maya_hot_reload_example.lib" /export:initializePlugin /export:uninitializePlugin %BuildDir%\plugin_main.obj /OUT:"%BuildDir%\maya_hot_reload_example.mll"

set LinkerFlagsHostRelease=%CommonLinkerFlagsRelease% %CommonLinkerFlagsHost%
set LinkerFlagsHostDebug=%CommonLinkerFlagsDebug% %CommonLinkerFlagsHost%
```

Again, MSDN is your friend to
[find out what all those options do](https://docs.microsoft.com/en-us/cpp/build/reference/linker-options).

It's not that terribly complicated when we break it down a little: we bascially
link against all the static Maya libraries that we're supposed to (in order to
be able to call Maya functions in our DLL), along with the Windows static
libraries as well (in order to call Windows functions as well). We tell the
linker that we're building a DLL by specifying the ``/DLL`` flag, say that we
would like the linker to do some basic optimization through the ``/OPT:REF``
flag, and tell it to write the **Program Database (PDB)** file out, along with
specifying the **Import Library** (i.e. ``*.lib`` file)and output DLL names
explicitly. If you're familiar with building a Maya plugin, you'll also notice
the infamous ``initializePlugin`` and ``uninitializePlugin`` symbols being
exported as well in the flags; these symbols *must* be made visible in the DLL
so that Maya can call the functions to load/unload the plugin respectively.

!!! tip "Crossing the platforms"
    The
    [Program Database (PDB)](https://support.microsoft.com/en-us/help/121366/description-of-the--pdb-files-and-of-the--dbg-files)
    file is a Windows-specific file that is used by Visual Studio to look up
    information during a debugging session, such as the location of the source
    files, the positions within the source files that symbols correspond to, and
    other project information as well. They are the most commonly-used debug
    format on Windows these days. Linux/OSX do not have this concept; the
    debugging information is "baked" into the compiled binaries themselves.

We're not done yet; we need to actually compile and link *something*!

```batch
if "%BuildType%"=="debug" (
    echo Building in debug mode...
    set CompilerFlagsHost=%CompilerFlagsHostDebug%
    set LinkerFlagsHost=%LinkerFlagsHostDebug%

    set CompilerFlagsLogic=%CompilerFlagsLogicDebug%
    set LinkerFlagsLogic=%LinkerFlagsLogicDebug%
) else (
    echo Building in release mode...
    set CompilerFlagsHost=%CompilerFlagsHostRelease%
    set LinkerFlagsHost=%LinkerFlagsHostRelease%

    set CompilerFlagsLogic=%CompilerFlagsLogicRelease%
    set LinkerFlagsLogic=%LinkerFlagsLogicRelease%
)

echo Compiling Host...
cl %CompilerFlagsHost%
if %errorlevel% neq 0 exit /b %errorlevel%

echo Linking Host...
link %LinkerFlagsHost%
if %errorlevel% neq 0 exit /b %errorlevel%


echo Compiling Logic...
cl %CompilerFlagsLogic%
if %errorlevel% neq 0 exit /b %errorlevel%

echo Linking Logic...
link %LinkerFlagsLogic%
if %errorlevel% neq 0 exit /b %errorlevel%

echo Build complete!
popd
```

Luckily, that's a *lot* easier once all the flags are set up. Whew!

If you got past all that and got a plugin building, great! We have a node that
does...well, *nothing*.

Now we just need to make it work.


## Writing the Client Plugin ##

We know from the previous section (and my fancy graphs)that we need a second DLL
to handle the **business logic** of the deformation. This client DLL will be
reloaded by the **Host DLL** every time the timestamp on it changes, and the
**function pointers** fixed up every time this happens. For simplicity's sake,
let's assume that all the business logic we'll be dealing with is just a
function that takes a point from the ``deform()`` function, does some work on
it, and returns us a new position for it to be set at.

Let's start with the first part of that sentence: making a second **client
DLL**.

### Name mangling & visibility ###

In ``logic.h``:

```c++
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


Shared
{
	DLLExport MVector getValue(MVector &v, float factor);
}
```

Ok, what did we just add? Let's go over it bit-by-bit:

``extern "C"`` is basically a **storage class specifier** that denotes *external
language linkage*. We ``#define`` the keyword ``Shared`` to basically say that
"anytime we use the ``Shared`` keyword, it means that we want the following
symbols that follow it to have external linkage, to be *shared*". We give it
``C`` linkage, to avoid our symbol names getting **mangled** by the compiler; we
do want to be able to call them later on without having to type weird names like
``_ZN9getValue76E``.

!!! note "Name mangling"
    If you're new to C++, you might not be familiar with this term. Basically,
    because C++ supports **overloading**, the compiler *mangles* symbol names
    based on their signatures to ensure that each overload ends up having a
    unique name, so that the compiler can mix and match the correct function
    calls as needed based on the input arguments. More information on this
    feature is available [here](http://www.geeksforgeeks.org/extern-c-in-c/).

    This also means that we should *not* define overloaded versions of our
    business logic functions, as the compiler won't know which version to use at
    link time (This is also *undefined behaviour*, which means anything could happen!)

Of course, this is C++, so things aren't as simple as that. We also define the
``DLLExport`` alias to do something called ``__declspec(dllexport)``, which
basically is a directive to the MSVC compiler that tells it to export the given
symbols which use it. This will make the symbols available to the interrogating
process that loads the DLL. (More information
[here](https://docs.microsoft.com/en-us/cpp/build/exporting-from-a-dll-using-declspec-dllexport))

We also make two ``typedef``s: one for DLL handles, and one for function
pointers. These will use Windows-specific types, which are defined in ``Windows.h``.

!!! tip "Crossing the platforms"
    On Linux, we use the ``visibility`` ``__attribute__`` directive instead,
    which GCC/Clang supports to achieve similar functionality. File handles and
    function pointers on Unix-based OSes are, thankfully, defined to use the
    basic ``void`` pointer. It's of note here that by default, unless we
    specifically strip the symbols ourselves, **all symbols are available to an
    interrogating process by default on Linux.** The reason we still make use of
    the visibility features is just in case we ever do decide to strip the
    symbol information in the compilation process, the directive will continue
    to make sure it remains visible in the output binary.

    More information on symbol visibility in GCC is available [here](https://gcc.gnu.org/wiki/Visibility).

We're basically doing the same thing as we did above when specifying
``initializePlugin`` and ``uninitializePlugin`` to be exported symbols, except
that we're doing it through code instead of the command line.

### Simple geometry ###

Let's go ahead and implement ``getValue`` in ``logic.cpp`` now:

```c++
MVector lerp(MVector &v1, float t, MVector &v2)
{
	MVector result = ((1.0f - t) * v1) + (t * v2);
	return result;
}

Shared
{
	DLLExport MVector getValue(MVector &v, float factor)
	{
		MVector result;

		result.x = v.x * 6;
		result.y = v.y * 4;
		result.z = v.z * 15;

		result = lerp(v, factor, result);

		return result;
	}
}
```

As we can see, it's pretty basic. Our business logic function essentially
*linearly interpolates* between a vector, and a non-uniformly scaled version of
itself based on a normalized``factor`` value. As it happens, the default ``envelope``
attribute on a Maya deformer is perfect for acting as the input to this
``factor``:

```c++
MStatus HotReloadableDeformer::initialize()
{
	MStatus result;
	attributeAffects(envelope, outputGeom);

	return result;
}

MStatus HotReloadableDeformer::deform(MDataBlock &block,
									  MItGeometry &iter,
									  const MMatrix &matrix,
									  unsigned int multiIndex)
{
	MStatus result;

	MDataHandle envelopeHandle = block.inputValue(envelope, &result);
	CHECK_MSTATUS_AND_RETURN_IT(result);

	float envelope = envelopeHandle.asFloat();

	for (; !iter.isDone(); iter.next())
	{

		MPoint curPtPosPt = iter.position();
		// Hmm...but how do we access getValue()?
		// MVector finalPosPt = getValue(MVector(curPtPosPt), envelope);
		MPoint finalPosPt = MPoint(finalPos.x, finalPos.y, finalPos.z, 1);

		iter.setPosition(finalPosPt);
	}

	return result;
}
```

We basically tell Maya that the ``envelope`` attribute is going to end up
affecting the geometry, and then...we realize that we can't actually call
``getValue()``, yet; at least, not without compiling both ``logic.cpp`` and
``plugin_main.cpp`` within the same **Translation Unit (TU)** so that we have
access to that symbol. So how are we going to do this?

### Building the plugin

Let's worry about that later. For now, let's focus on compiling that
``logic.cpp`` file we just wrote into its own DLL:

```batch
REM    Previous stuff...

set LogicEntryPoint=%~dp0src\logic.cpp

set OutputLogicDLLFilename=%BuildDir%\logic.dll

REM    Previous stuff...

set CommonLinkerFlagsLogic=/PDB:"%BuildDir%\logic.pdb" /IMPLIB:"%BuildDir%\logic.lib" /OUT:"%BuildDir%\logic.dll" %BuildDir%\logic.obj

REM    Previous stuff...

set LinkerFlagsLogicRelease=%CommonLinkerFlagsRelease% %CommonLinkerFlagsLogic%
set LinkerFlagsLogicDebug=%CommonLinkerFlagsDebug% %CommonLinkerFlagsLogic%

REM   Previous stuff...

echo Compiling Logic...
cl %CompilerFlagsLogic%
if %errorlevel% neq 0 exit /b %errorlevel%

echo Linking Logic...
link %LinkerFlagsLogic%
if %errorlevel% neq 0 exit /b %errorlevel%
```

Go ahead and run ``build.bat debug``. Hopefully, you should get the following
files in your ``msbuild`` folder:

```
logic.dll
logic.exp
logic.lib
logic.obj
maya_hot_reload_example.exp
maya_hot_reload_example.lib
maya_hot_reload_example.mll
plugin_main.obj
```

!!! tip "Crossing the platforms"

    The ``.exp`` file is known as an **exports** file, and the ``.lib`` file is
    an **import library**. They are mainly used to resolve *circular exports*,
    where a program exports to another program that it also imports from, or
    when more than two programs both import/export to each other; the linker
    needs to know to resolve the dependencies at link-time. They contain
    information about exported functions and other constant data (such as global
    variables). For our purposes, they are largely superfluous, but it is
    important to know about them when dealing with larger, more complicated
    setups.

    Again, Linux/OSX SOs do not have this sort of machinery; it is Windows-specific.

    More information is available [here.](https://msdn.microsoft.com/en-us/library/kkt2hd12.aspx?f=255&MSPPError=-2147217396)

Try loading the plugin into Maya and applying it to a deformer:

```mel
file -f -new;

loadPlugin "c:/Users/sonictk/Git/experiments/maya_hot_reload_example/msbuild/maya_hot_reload_example.dll";

polySphere;
deformer -type "hotReloadableDeformer";
```



If you've made it to this point, great! Let's move on to the next section, where
we'll start figuring out how to get access to that ``getValue`` function from
our main ``deformer.cpp`` TU.
