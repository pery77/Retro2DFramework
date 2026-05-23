@echo off
setlocal

cd /d "%~dp0"

set "CONFIG=%~1"
set "TARGET=%~2"

if "%CONFIG%"=="" set "CONFIG=Debug"
if "%TARGET%"=="" set "TARGET=@PROJECT_TARGET@"

if /I "%CONFIG%"=="debug" set "CONFIG=Debug"
if /I "%CONFIG%"=="release" set "CONFIG=Release"

if /I "%CONFIG%"=="dist" (
    cmake -S . -B build
    if errorlevel 1 exit /b %errorlevel%
    cmake --build build --config Release --target r2d_pack_game_@PROJECT_TARGET@
    exit /b %errorlevel%
)

cmake -S . -B build
if errorlevel 1 exit /b %errorlevel%

cmake --build build --config %CONFIG% --target %TARGET%
exit /b %errorlevel%

