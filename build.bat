@echo off
setlocal

cd /d "%~dp0"

set "CONFIG=%~1"
set "TARGET=%~2"
set "ENABLE_LUAJIT=%R2D_ENABLE_LUAJIT%"
set "CONFIG_LUAJIT_ROOT=%LUAJIT_ROOT%"

if "%CONFIG%"=="" set "CONFIG=Debug"
if "%TARGET%"=="" set "TARGET=all"

if /I "%~2"=="luajit" (
    set "ENABLE_LUAJIT=1"
    set "CONFIG_LUAJIT_ROOT=%~3"
)

if /I "%~3"=="luajit" (
    set "ENABLE_LUAJIT=1"
    set "CONFIG_LUAJIT_ROOT=%~4"
)

if "%ENABLE_LUAJIT%"=="" set "ENABLE_LUAJIT=0"

if /I "%CONFIG%"=="help" goto help
if /I "%CONFIG%"=="-h" goto help
if /I "%CONFIG%"=="--help" goto help
if /I "%CONFIG%"=="configure" goto configure
if /I "%CONFIG%"=="dist" goto dist

if /I "%CONFIG%"=="debug" set "CONFIG=Debug"
if /I "%CONFIG%"=="release" set "CONFIG=Release"

if /I not "%CONFIG%"=="Debug" if /I not "%CONFIG%"=="Release" goto usage

call :configure_cmake
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
echo   build.bat dist [all^|target]
echo   build.bat configure
echo.
echo Examples:
echo   build.bat debug
echo   build.bat release
echo   build.bat debug r2d_collect
echo   build.bat dist r2d_collect
echo   build.bat dist all
echo   build.bat debug all
exit /b 1

:help
echo Usage:
echo   build.bat [debug^|release] [all^|target]
echo   build.bat dist [all^|target]
echo   build.bat configure
echo.
echo Examples:
echo   build.bat debug
echo   build.bat release
echo   build.bat debug r2d_collect
echo   build.bat dist r2d_collect
echo   build.bat dist all
echo   build.bat debug all
exit /b 0

:configure
call :configure_cmake
exit /b %errorlevel%

:dist
set "CONFIG=Release"
if /I "%TARGET%"=="all" (
    set "PACKAGE_TARGET=r2d_pack_game"
) else (
    set "PACKAGE_TARGET=r2d_pack_game_%TARGET%"
)

call :configure_cmake
if errorlevel 1 exit /b %errorlevel%

cmake --build build --config %CONFIG% --target %PACKAGE_TARGET%
exit /b %errorlevel%

:configure_cmake
set "CMAKE_CONFIG_ARGS="
if /I "%ENABLE_LUAJIT%"=="1" set "CMAKE_CONFIG_ARGS=%CMAKE_CONFIG_ARGS% -DR2D_ENABLE_LUAJIT=ON"
if /I "%ENABLE_LUAJIT%"=="true" set "CMAKE_CONFIG_ARGS=%CMAKE_CONFIG_ARGS% -DR2D_ENABLE_LUAJIT=ON"
if /I "%ENABLE_LUAJIT%"=="on" set "CMAKE_CONFIG_ARGS=%CMAKE_CONFIG_ARGS% -DR2D_ENABLE_LUAJIT=ON"
if /I "%ENABLE_LUAJIT%"=="yes" set "CMAKE_CONFIG_ARGS=%CMAKE_CONFIG_ARGS% -DR2D_ENABLE_LUAJIT=ON"
if /I "%ENABLE_LUAJIT%"=="0" set "CMAKE_CONFIG_ARGS=%CMAKE_CONFIG_ARGS% -DR2D_ENABLE_LUAJIT=OFF"
if /I "%ENABLE_LUAJIT%"=="false" set "CMAKE_CONFIG_ARGS=%CMAKE_CONFIG_ARGS% -DR2D_ENABLE_LUAJIT=OFF"
if /I "%ENABLE_LUAJIT%"=="off" set "CMAKE_CONFIG_ARGS=%CMAKE_CONFIG_ARGS% -DR2D_ENABLE_LUAJIT=OFF"
if /I "%ENABLE_LUAJIT%"=="no" set "CMAKE_CONFIG_ARGS=%CMAKE_CONFIG_ARGS% -DR2D_ENABLE_LUAJIT=OFF"
if /I "%ENABLE_LUAJIT%"=="1" if "%CONFIG_LUAJIT_ROOT%"=="" set "CMAKE_CONFIG_ARGS=%CMAKE_CONFIG_ARGS% -DLUAJIT_ROOT="
if /I "%ENABLE_LUAJIT%"=="true" if "%CONFIG_LUAJIT_ROOT%"=="" set "CMAKE_CONFIG_ARGS=%CMAKE_CONFIG_ARGS% -DLUAJIT_ROOT="
if /I "%ENABLE_LUAJIT%"=="on" if "%CONFIG_LUAJIT_ROOT%"=="" set "CMAKE_CONFIG_ARGS=%CMAKE_CONFIG_ARGS% -DLUAJIT_ROOT="
if /I "%ENABLE_LUAJIT%"=="yes" if "%CONFIG_LUAJIT_ROOT%"=="" set "CMAKE_CONFIG_ARGS=%CMAKE_CONFIG_ARGS% -DLUAJIT_ROOT="
if not "%CONFIG_LUAJIT_ROOT%"=="" set "CMAKE_CONFIG_ARGS=%CMAKE_CONFIG_ARGS% ^"-DLUAJIT_ROOT=%CONFIG_LUAJIT_ROOT%^""

cmake -S . -B build %CMAKE_CONFIG_ARGS%
exit /b %errorlevel%
