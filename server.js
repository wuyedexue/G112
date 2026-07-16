const http = require('http');
const fs = require('fs');
const path = require('path');
const { createMockRecognition } = require('./coordinate-core');

const PORT = process.env.PORT || 3000;
const ROOT = __dirname;

const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.css': 'text/css; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
    '.json': 'application/json; charset=utf-8',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.jpeg': 'image/jpeg',
    '.gif': 'image/gif',
    '.svg': 'image/svg+xml'
};

function sendJson(res, status, payload) {
    res.writeHead(status, {
        'Content-Type': 'application/json; charset=utf-8',
        'Access-Control-Allow-Origin': '*'
    });
    res.end(JSON.stringify(payload));
}

function parseBody(req) {
    return new Promise((resolve, reject) => {
        let body = '';
        req.on('data', chunk => {
            body += chunk;
            if (body.length > 12 * 1024 * 1024) {
                req.destroy(new Error('Payload too large'));
            }
        });
        req.on('end', () => {
            if (!body) return resolve({});
            try {
                resolve(JSON.parse(body));
            } catch (error) {
                reject(new Error('Invalid JSON'));
            }
        });
        req.on('error', reject);
    });
}

function serveStatic(req, res) {
    const cleanPath = decodeURIComponent(req.url.split('?')[0]);
    const relativePath = cleanPath === '/' ? 'index.html' : cleanPath.replace(/^\/+/, '');
    const filePath = path.resolve(ROOT, relativePath);

    if (!filePath.startsWith(ROOT)) {
        res.writeHead(403, { 'Content-Type': 'text/plain; charset=utf-8' });
        res.end('Forbidden');
        return;
    }

    fs.readFile(filePath, (error, data) => {
        if (error) {
            res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
            res.end('Not Found');
            return;
        }

        const contentType = MIME_TYPES[path.extname(filePath).toLowerCase()] || 'application/octet-stream';
        res.writeHead(200, { 'Content-Type': contentType });
        res.end(data);
    });
}

async function handleRecognize(req, res) {
    try {
        const body = await parseBody(req);
        const imageName = body.imageName || 'uploaded-image';
        const imageWidth = Number(body.imageWidth) || 960;
        const imageHeight = Number(body.imageHeight) || 640;

        const result = createMockRecognition({ imageName, imageWidth, imageHeight });
        sendJson(res, 200, result);
    } catch (error) {
        sendJson(res, 400, { error: error.message });
    }
}

const server = http.createServer(async (req, res) => {
    const route = req.url.split('?')[0];

    if (req.method === 'OPTIONS') {
        res.writeHead(204, {
            'Access-Control-Allow-Origin': '*',
            'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
            'Access-Control-Allow-Headers': 'Content-Type'
        });
        res.end();
        return;
    }

    if (route === '/api/recognize' && req.method === 'POST') {
        await handleRecognize(req, res);
        return;
    }

    if (route === '/api/health' && req.method === 'GET') {
        sendJson(res, 200, { ok: true, tool: 'coordinate-tool-mvp' });
        return;
    }

    serveStatic(req, res);
});

server.listen(PORT, () => {
    console.log('==========================================');
    console.log('  坐标工具 MVP 已启动');
    console.log('==========================================');
    console.log(`访问地址: http://localhost:${PORT}`);
    console.log('按 Ctrl+C 停止服务');
});
