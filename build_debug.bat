@echo off
setlocal

cd /d "%~dp0"

call "%~dp0build.bat" debug all
exit /b %errorlevel%
