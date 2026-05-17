@echo off
setlocal enabledelayedexpansion
cd /D "%~dp0"

:: -- Usage (2026/05/17) -------------------------------------------------------
::
:: Derived from RADDBG build.bat
::
:: This is a build script for use in Windows development environments. It takes
:: a list of alphanumeric only arguments that specifies the build options. By 
:: default if no options are passed, all build targets are built in debug.
::
:: Below is a non-exhaustive list of possible ways to use the script:
:: `build all`
:: `build all release`
:: `build viewer debug`
:: `build viewer asan`
::
:: For a full list of possible build targets and their build command lines,
:: search for @build_targets in this file.
::
:: This is a list of all possible command line options:
:: - debug: build in debug
:: - release: build in release
:: - asan: enable address sanitizer
:: - profile: enable address sanitizer

:: --- Unpack Arguments --------------------------------------------------------
for %%a in (%*) do set "%%~a=1"
if not "%msvc%"=="1" if not "%clang%"=="1" set msvc=1
if not "%release%"=="1" set debug=1
if "%debug%"=="1"   set release=0 && echo [debug mode]
if "%release%"=="1" set debug=0 && echo [release mode]
if "%msvc%"=="1"    set clang=0 && echo [msvc compile]
if "%~1"==""                     echo [default mode, assuming `all` build] && set all=1
if "%~1"=="release" if "%~2"=="" echo [default mode, assuming `all` build] && set all=1

:: --- Unpack Command Line Build Arguments ------------------------------------- 
set compile_flags=
if "%asan%"=="1"    set compile_flags=%compile_flags% -fsanitize=address && echo [asan enabled]
if "%profile%"=="1" set compile_flags=%compile_flags% -DDK_PROFILE_ENABLE && echo [profiling enabled]

:: --- Compile/Link Time Definitions -------------------------------------------
set cl_include= /I..\src\
set cl_common=  /nologo /FC /Z7 /W4 /WX /std:c++20 /EHsc /Zc:__cplusplus /Fo..\.tmp\ -D_CRT_SECURE_NO_WARNINGS %cl_include% %compile_flags%
set cl_debug=   call cl /Od /MTd /D_DEBUG /RTC1 %cl_common%
set cl_release= call cl /O2 /MT /DNDEBUG %cl_common%
set cl_link=    /link /MANIFEST:EMBED /INCREMENTAL:NO /pdbaltpath:%%%%_PDB%%%% /noexp /nocoffgrpinfo
set cl_out=     /out:
set cl_linker=  

:: --- Choose Compile Lines ----------------------------------------------------
if "%msvc%"=="1"         set compile_debug=%cl_debug%
if "%msvc%"=="1"         set compile_release=%cl_release%
if "%msvc%"=="1"         set compile_link=%cl_link%
if "%msvc%"=="1"         set out=%cl_out%
if "%debug%"=="1"        set compile=%compile_debug%
if "%release%"=="1"      set compile=%compile_release%

:: --- Per-Build Settings ------------------------------------------------------
set glslang= ..\tools\glslangValidator.exe

:: --- Prep Directories --------------------------------------------------------
if not exist bin mkdir bin
if not exist .tmp mkdir .tmp

:: --- Build (@build_targets)---------------------------------------------------
pushd bin
if "%all%"=="1" (
    echo [building all targets]
    set cooker=1
    set viewer=1
    set shaders=1
)
if "%viewer%"=="1" set didbuild=1 && %compile% ..\src\viewer\viewer_main.cpp %compile_link% %out%viewer.exe || exit /b 1
if "%cooker%"=="1" set didbuild=1 && %compile% ..\src\cooker\cooker_main.cpp %compile_link% %out%cooker.exe || exit /b 1
if "%shaders%"=="1" (
    echo [building shaders]
    set didbuild=1
    for %%f in (..\src\shaders\*.vert ..\src\shaders\*.frag ..\src\shaders\*.comp) do (
        %glslang% -G -o "%%~nxf.spv" "%%f" || exit /b 1
    )
)
popd

if "%didbuild%"=="" (
  echo [WARNING] no valid build target specified; must use build target names as arguments to this script, like `build viewer` or `build all`.
  exit /b 1
)
