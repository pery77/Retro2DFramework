@echo off
setlocal

cd /d "%~dp0"

set "CONFIG=%~1"
set "TARGET=%~2"

if "%CONFIG%"=="" set "CONFIG=Debug"
if "%TARGET%"=="" set "TARGET=all"

if /I "%CONFIG%"=="help" goto help
if /I "%CONFIG%"=="-h" goto help
if /I "%CONFIG%"=="--help" goto help
if /I "%CONFIG%"=="configure" goto configure

if /I "%CONFIG%"=="debug" set "CONFIG=Debug"
if /I "%CONFIG%"=="release" set "CONFIG=Release"

if /I not "%CONFIG%"=="Debug" if /I not "%CONFIG%"=="Release" goto usage

cmake -S . -B build
if errorlevel 1 exit /b %errorlevel%

if /I "%TARGET%"=="all" (
    cmake --build build --config %CONFIG%
) else (
    cmake --build build --config %CONFIG% --target %TARGET%
)

exit /b %errorlevel%

:usage
echo Usage:
echo   build.bat [debug^|release] [all^|target]
echo   build.bat configure
echo.
echo Examples:
echo   build.bat debug
echo   build.bat release
echo   build.bat debug r2d_collect
echo   build.bat release r2d_sandbox
echo   build.bat debug all
exit /b 1

:help
echo Usage:
echo   build.bat [debug^|release] [all^|target]
echo   build.bat configure
echo.
echo Examples:
echo   build.bat debug
echo   build.bat release
echo   build.bat debug r2d_collect
echo   build.bat release r2d_sandbox
echo   build.bat debug all
exit /b 0

:configure
cmake -S . -B build
exit /b %errorlevel%
