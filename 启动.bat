@echo off
chcp 65001 >nul 2>&1
title 视频下载工具

echo.
echo ==========================================
echo   全网视频下载工具 - 一键启动
echo ==========================================
echo.

:: Check if Node.js is installed
where node >nul 2>&1
if %errorlevel% neq 0 (
    echo [错误] 未检测到 Node.js！
    echo.
    echo 请先安装 Node.js:
    echo   下载地址: https://nodejs.org/zh-cn
    echo   选择 "LTS" 版本下载安装即可
    echo.
    echo 安装完成后重新双击此文件启动。
    echo.
    pause
    exit /b 1
)

echo [OK] Node.js 已安装
echo.
echo 正在启动服务器...
echo 启动后会自动下载所需组件（首次启动需要等待）
echo.
echo ==========================================
echo   启动后请在浏览器中打开:
echo   http://localhost:3000
echo ==========================================
echo.

node server.js

if %errorlevel% neq 0 (
    echo.
    echo [错误] 服务器启动失败
    pause
)
