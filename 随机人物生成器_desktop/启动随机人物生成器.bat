@echo off
chcp 65001 >nul
title 随机人物生成器

echo ========================================
echo    随机人物生成器 - Python桌面版
echo ========================================
echo.

:: 检查Python是否安装
python --version >nul 2>&1
if errorlevel 1 (
    echo [错误] 未检测到Python，请先安装Python 3.9+
    echo 下载地址: https://www.python.org/downloads/
    pause
    exit /b 1
)

:: 检查是否已安装依赖
python -c "import PyQt6" >nul 2>&1
if errorlevel 1 (
    echo [提示] 首次运行，正在安装依赖...
    pip install PyQt6 -q
    if errorlevel 1 (
        echo [错误] 安装依赖失败，请手动运行: pip install PyQt6
        pause
        exit /b 1
    )
    echo [完成] 依赖安装成功！
    echo.
)

:: 启动程序
echo 正在启动随机人物生成器...
python "%~dp0main.py"

if errorlevel 1 (
    echo.
    echo [错误] 程序异常退出
    pause
)
