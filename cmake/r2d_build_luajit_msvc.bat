@echo off
setlocal

set "VS_DEV_CMD=%~1"
set "R2D_MSVC_ARCH=%~2"
set "LUAJIT_SRC=%~3"

if "%VS_DEV_CMD%"=="" exit /b 1
if "%R2D_MSVC_ARCH%"=="" exit /b 1
if "%LUAJIT_SRC%"=="" exit /b 1

call "%VS_DEV_CMD%" -arch=%R2D_MSVC_ARCH% -host_arch=%R2D_MSVC_ARCH% -no_logo
if errorlevel 1 exit /b %errorlevel%

cd /d "%LUAJIT_SRC%"
if errorlevel 1 exit /b %errorlevel%

call msvcbuild.bat static
exit /b %errorlevel%
