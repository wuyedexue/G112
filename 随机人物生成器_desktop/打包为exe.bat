@echo off
chcp 65001 >nul
title 打包为独立exe

echo ========================================
echo    打包随机人物生成器为独立exe
echo ========================================
echo.

:: 检查Python
python --version >nul 2>&1
if errorlevel 1 (
    echo [错误] 未检测到Python
    pause
    exit /b 1
)

:: 检查pyinstaller
python -c "import PyInstaller" >nul 2>&1
if errorlevel 1 (
    echo [提示] 正在安装PyInstaller...
    pip install pyinstaller -q
)

echo 正在打包，请稍候...
echo.

pyinstaller --noconfirm --onefile --windowed ^
    --name "随机人物生成器" ^
    --add-data "data.py;." ^
    "%~dp0main.py"

if errorlevel 1 (
    echo.
    echo [错误] 打包失败
    pause
    exit /b 1
)

echo.
echo ========================================
echo [完成] 打包成功！
echo 可执行文件位于: dist\随机人物生成器.exe
echo ========================================
pause
