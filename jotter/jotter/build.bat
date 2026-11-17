@echo off
setlocal
rem Run from any working directory. An optional argument selects a Qt kit.
rem Example: build.bat "C:\Qt\6.10.0\msvc2022_64"
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\build-windows.ps1" %*
set "BUILD_RESULT=%ERRORLEVEL%"
echo.
if not "%BUILD_RESULT%"=="0" echo Jotter build failed. See the message above.
rem Keep the result visible when launched by double-clicking.
if not defined JOTTER_NO_PAUSE pause
exit /b %BUILD_RESULT%
