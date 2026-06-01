@echo off
setlocal enabledelayedexpansion
cd /D "%~dp0"

:: -- Usage (2026/05/22) -------------------------------------------------------
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
:: `build dkrend debug`
:: `build dkrend asan`
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

:: --- Shader Compile Definitions ------------------------------------------------------
set glslang= ..\tools\glslangValidator.exe
set glslang_include= --preamble-text "#extension GL_GOOGLE_include_directive : require" -I..\src\shaders\

:: --- Choose Compile Lines ----------------------------------------------------
if "%msvc%"=="1"         set compile_debug=%cl_debug%
if "%msvc%"=="1"         set compile_release=%cl_release%
if "%msvc%"=="1"         set compile_link=%cl_link%
if "%msvc%"=="1"         set out=%cl_out%
if "%debug%"=="1"        set compile=%compile_debug%
if "%release%"=="1"      set compile=%compile_release%

:: --- Prep Directories --------------------------------------------------------
if not exist bin mkdir bin
if not exist .tmp mkdir .tmp
if not exist src\shaders\.spirv mkdir src\shaders\.spirv

:: --- Build (@build_targets)---------------------------------------------------
pushd bin
if "%all%"=="1" (
    echo [building all targets]
    set dkcook=1
    set dkrend=1
    set assets=1
)
if "%dkcook%"=="1" set didbuild=1 && %compile% ..\src\dkcook\dkcook_main.cpp %compile_link% %out%dkcook.exe || exit /b 1
if "%dkrend%"=="1" set didbuild=1 && %compile% ..\src\dkrend\dkrend_main.cpp %compile_link% %out%dkrend.exe || exit /b 1
if "%assets%"=="1" (
    set didbuild=1
    echo [building shaders]
    for %%f in (..\src\shaders\*.vert ..\src\shaders\*.frag ..\src\shaders\*.comp) do (
        %glslang% %glslang_include% -G -o "..\src\shaders\.spirv\%%~nxf.spv" "%%f" || exit /b 1
    )
    %compile% ..\src\pak_make\pak_make_main.cpp %compile_link% %out%pak_make.exe || exit /b 1
    if exist pakgen.exe (
        echo [running pak_make]
        pak_make.exe --output=dkrend.pak --verbose || exit /b 1
    )
)
popd

:: --- Warn On No Builds -------------------------------------------------------
if "%didbuild%"=="" (
  echo [WARNING] no valid build target specified; must use build target names as arguments to this script, like `build dkrend` or `build all`.
  exit /b 1
)
