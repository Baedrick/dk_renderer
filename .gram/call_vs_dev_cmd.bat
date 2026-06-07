@echo off

where /Q cl.exe && exit /b 0

set __VSCMD_ARG_NO_LOGO=1
for /f "tokens=*" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.VisualStudio.Workload.NativeDesktop -property installationPath') do set VS=%%i
if "%VS%" == "" (
  echo ERROR: Visual Studio installation not found.
  exit /b 1
)
call "%VS%\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 -startdir=none -no_logo || exit /b 1
