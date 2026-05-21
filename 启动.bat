@echo off
title Video Download Tool

echo.
echo ==========================================
echo   Video Download Tool
echo ==========================================
echo.

:: Check if Node.js is installed
where node >nul 2>&1
if %errorlevel% neq 0 goto NONODE

echo [OK] Node.js found
echo.
echo Starting server...
echo (First launch will download required components, please wait)
echo.
echo ==========================================
echo   Open in browser:
echo   http://localhost:3000
echo ==========================================
echo.

node server.js
if %errorlevel% neq 0 goto SERVERFAIL
goto END

:NONODE
echo.
echo [ERROR] Node.js is NOT installed!
echo.
echo Please install Node.js first:
echo   Download: https://nodejs.org/zh-cn
echo   Choose the "LTS" version, install with defaults
echo.
echo After installation, double-click this file again to start.
echo.
pause
goto END

:SERVERFAIL
echo.
echo [ERROR] Server failed to start. Check error messages above.
echo.
pause
goto END

:END
