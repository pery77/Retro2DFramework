@echo off
setlocal

cd /d "%~dp0"

set "BOOT_LOG=%~dp0build_gui_boot.log"

echo [%date% %time%] Starting build launcher > "%BOOT_LOG%"
powershell -NoProfile -STA -ExecutionPolicy Bypass -File "%~dp0tools\build_gui.ps1" >> "%BOOT_LOG%" 2>&1
set "LAUNCHER_EXIT=%ERRORLEVEL%"

if not "%LAUNCHER_EXIT%"=="0" (
    echo.
    echo Build launcher failed before the window could stay open.
    echo Check build_gui.log and build_gui_boot.log for details.
    pause
    exit /b %LAUNCHER_EXIT%
)

exit /b 0
