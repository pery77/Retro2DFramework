@echo off
setlocal

cd /d "%~dp0"

call "%~dp0build.bat" debug r2d_sandbox
exit /b %errorlevel%
