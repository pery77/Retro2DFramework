@echo off
setlocal

cd /d "%~dp0"

call "%~dp0build.bat" debug r2d_midi_player
exit /b %errorlevel%
