@echo off
setlocal enabledelayedexpansion
cd /D "%~dp0"

:: --- Environment -------------------------------------------------------------
where /Q cl.exe || (
    set __VSCMD_ARG_NO_LOGO=1
    for /f "tokens=*" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.VisualStudio.Workload.NativeDesktop -property installationPath') do set VS=%%i
    if "!VS!" equ "" (
        echo [ERROR] visual studio installation not found.
        exit /b 1
    )
    call "!VS!\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 -startdir=none -no_logo || exit /b 1
)

:: --- Unpack Arguments --------------------------------------------------------
for %%a in (%*) do set "%%~a=1"
if not "%release%"=="1" set debug=1
if "%debug%"=="1" set release=0 && echo [debug mode]
if "%release%"=="1" set debug=0 && echo [release mode]

set compile_flags=
if "%profile%"=="1" set compile_flags=%compile_flags% -DDK_PROFILE=1 && echo [profiling enabled]
if "%asan%"=="1"    set auto_compile_flags=%auto_compile_flags% -fsanitize=address && echo [asan enabled]

:: --- Compile/Link Time Definitions -------------------------------------------
set cl_common=  /I..\src\ /nologo /FC /Z7 /W4 /WX /std:c++17 /Zc:__cplusplus /Fo..\.tmp\ %compile_flags%
set cl_debug=   call cl /Od /MTd /D_DEBUG /RTC1 %cl_common%
set cl_release= call cl /O2 /MT /DNDEBUG %cl_common%
set cl_link=    /link /MANIFEST:EMBED /INCREMENTAL:NO /pdbaltpath:%%%%_PDB%%%%
set cl_out=     /out:
set cl_linker=  

:: --- Choose Compile Lines ----------------------------------------------------
if "%debug%"=="1"     set compile=%cl_debug%
if "%release%"=="1"   set compile=%cl_release%

:: --- Prep Directories --------------------------------------------------------
if not exist build mkdir build
if not exist .tmp mkdir .tmp

:: --- Build -------------------------------------------------------------------
pushd build
%compile% ..\src\main.cpp %cl_link% %cl_out%toy_viewer.exe || exit /b 1
popd

if %ERRORLEVEL% neq 0 (
    echo [ERROR] build failed.
    exit /b %ERRORLEVEL%
)
