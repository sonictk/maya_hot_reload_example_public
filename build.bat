@echo off

REM    This is the Windows build script for the Maya hot reloadable deformer.
REM    usage: build.bat [debug|release]

REM Process command line arguments
set BuildType=%1
if "%BuildType%"=="" (set BuildType=release)


REM    Set up the Visual Studio environment variables for calling the MSVC compiler;
REM    we do this after the call to pushd so that the top directory on the stack
REM    is saved correctly; the check for DevEnvDir is to make sure the vcvarsall.bat
REM    is only called once per-session (since repeated invocations will screw up
REM    the environment)
if not defined DevEnvDir (
    call "%vs2017installdir%\VC\Auxiliary\Build\vcvarsall.bat" x64
    REM    Or maybe you're on VS 2015? Call this instead:
    REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
)


REM    Make a build directory to store artifacts; remember, %~dp0 is just a special
REM    FOR variable reference in Windows that specifies the current directory the
REM    batch script is being run in
set BuildDir=%~dp0msbuild
echo Building in directory: %BuildDir%

if not exist %BuildDir% mkdir %BuildDir%
pushd %BuildDir%


REM    Set up globals
set MayaRootDir=C:\Program Files\Autodesk\Maya2016
set MayaIncludeDir=%MayaRootDir%\include
set ThirdPartyDir=C:\Users\sonictk\Git\experiments\maya_hot_reload_example\thirdparty

set HostEntryPoint=%~dp0src\plugin_main.cpp
set LogicEntryPoint=%~dp0src\logic.cpp

set OutputLogicDLLBasename=logic
set OutputLogicDLLFilename=%BuildDir%\%OutputLogicDLLBasename%.dll
set OutputLogicDLLTempFilename=%OutputLogicDLLBasename%_%random%.dll
set OutputHostMLLFilename=%BuildDir%\maya_hot_reload_example.mll


REM    We get around Windows locking the DLL file this way; renaming doesn't invalidate the handle
if exist %OutputLogicDLLFilename% (rename %OutputLogicDLLFilename% %OutputLogicDLLTempFilename%)


REM    We pipe errors to null, since we don't care if it fails
del *.pdb > NUL 2> NUL


REM    Setup all the compiler flags
set CommonCompilerFlags=/c /MP /W3 /WX- /Gy /Zc:wchar_t /Zc:forScope /Zc:wchar_t /Zc:forScope /Zc:inline /openmp /fp:precise /nologo /EHsc /MD /D REQUIRE_IOSTREAM /D _CRT_SECURE_NO_WARNINGS /D _BOOL /D NT_PLUGIN /D _WINDLL /D _MBCS /Gm- /GS /Gy /Gd /TP

REM    Add the include directories for header files
set CommonCompilerFlags=%CommonCompilerFlags% /I"%MayaRootDir%\include" /I"%ThirdPartyDir%"

set CommonCompilerFlagsDebug=/Zi /Od %CommonCompilerFlags%
set CommonCompilerFlagsRelease=/O2 %CommonCompilerFlags%

set CompilerFlagsHostDebug=%CommonCompilerFlagsDebug% %HostEntryPoint%
set CompilerFlagsHostRelease=%CommonCompilerFlagsRelease% %HostEntryPoint%

set CompilerFlagsLogicDebug=%CommonCompilerFlagsDebug% %LogicEntryPoint%
set CompilerFlagsLogicRelease=%CommonCompilerFlagsRelease% %LogicEntryPoint%


REM    Setup all the linker flags
set CommonLinkerFlags= /NOLOGO /INCREMENTAL:no /OPT:REF /MANIFEST /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /manifest:embed /SUBSYSTEM:CONSOLE /TLBID:1 /DYNAMICBASE /NXCOMPAT /MACHINE:X64  /machine:x64 /DLL

REM    Add all the Maya libraries to link against
set CommonLinkerFlags=%CommonLinkerFlags% "%MayaRootDir%\lib\OpenMaya.lib" "%MayaRootDir%\lib\OpenMayaAnim.lib" "%MayaRootDir%\lib\OpenMayaFX.lib" "%MayaRootDir%\lib\OpenMayaRender.lib" "%MayaRootDir%\lib\OpenMayaUI.lib" "%MayaRootDir%\lib\Foundation.lib" "%MayaRootDir%\lib\clew.lib" "%MayaRootDir%\lib\OpenMaya.lib" "%MayaRootDir%\lib\Image.lib" "%MayaRootDir%\lib\Foundation.lib" "%MayaRootDir%\lib\IMFbase.lib" "%MayaRootDir%\lib\OpenMaya.lib" "%MayaRootDir%\lib\OpenMayaAnim.lib" "%MayaRootDir%\lib\OpenMayaFX.lib" "%MayaRootDir%\lib\OpenMayaRender.lib" "%MayaRootDir%\lib\OpenMayaUI.lib" "%MayaRootDir%\lib\clew.lib" "%MayaRootDir%\lib\Image.lib" "%MayaRootDir%\lib\IMFbase.lib"

REM    Now add the OS libraries to link against
set CommonLinkerFlags=%CommonLinkerFlags% Shlwapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib

set CommonLinkerFlagsDebug=%CommonLinkerFlags% /DEBUG
set CommonLinkerFlagsRelease=%CommonLinkerFlags%

set CommonLinkerFlagsHost=/PDB:"%BuildDir%\maya_hot_reload_example.pdb" /IMPLIB:"%BuildDir%\maya_hot_reload_example.lib" /export:initializePlugin /export:uninitializePlugin %BuildDir%\plugin_main.obj /OUT:"%BuildDir%\maya_hot_reload_example.mll"
set CommonLinkerFlagsLogic=/PDB:"%BuildDir%\logic_%random%.pdb" /IMPLIB:"%BuildDir%\logic.lib" /OUT:"%BuildDir%\logic.dll" %BuildDir%\logic.obj

set LinkerFlagsHostRelease=%CommonLinkerFlagsRelease% %CommonLinkerFlagsHost%
set LinkerFlagsHostDebug=%CommonLinkerFlagsDebug% %CommonLinkerFlagsHost%

set LinkerFlagsLogicRelease=%CommonLinkerFlagsRelease% %CommonLinkerFlagsLogic%
set LinkerFlagsLogicDebug=%CommonLinkerFlagsDebug% %CommonLinkerFlagsLogic%


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


set PDBLockFilename=pdb_lock.tmp
echo Waiting for PDB to be written... > %PDBLockFilename%

echo Compiling Logic...
cl %CompilerFlagsLogic%
if %errorlevel% neq 0 exit /b %errorlevel%

echo Linking Logic...
link %LinkerFlagsLogic%
if %errorlevel% neq 0 exit /b %errorlevel%

echo Deleting temporary artifacts...
if exist %OutputLogicDLLTempFilename% del %OutputLogicDLLTempFilename%
if exist %PDBLockFilename% del %PDBLockFilename%


echo Compiling Host..
echo %CompilerFlagsHost%
cl %CompilerFlagsHost%

echo Linking Host...
link %LinkerFlagsHost%


echo Build complete!
popd
