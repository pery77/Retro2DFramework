@echo off
setlocal

cd /d "%~dp0"

call "%~dp0build.bat" release all
exit /b %errorlevel%
