#!/bin/bash
echo ""
echo "=========================================="
echo "  全网视频下载工具 - 一键启动"
echo "=========================================="
echo ""

# Check Node.js
if ! command -v node &> /dev/null; then
    echo "[错误] 未检测到 Node.js！"
    echo ""
    echo "请先安装 Node.js:"
    echo "  macOS: brew install node"
    echo "  Linux: sudo apt install nodejs (Ubuntu/Debian)"
    echo "         sudo dnf install nodejs (Fedora)"
    echo "  或访问: https://nodejs.org"
    echo ""
    exit 1
fi

echo "[OK] Node.js 已安装"
echo ""
echo "正在启动服务器..."
echo "启动后会自动下载所需组件（首次启动需要等待）"
echo ""
echo "=========================================="
echo "  启动后请在浏览器中打开:"
echo "  http://localhost:3000"
echo "=========================================="
echo ""

node server.js
