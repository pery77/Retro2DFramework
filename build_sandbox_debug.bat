@echo off
setlocal

cd /d "%~dp0"

cmake -S . -B build
if errorlevel 1 exit /b %errorlevel%

cmake --build build --config Debug --target r2d_sandbox
exit /b %errorlevel%
