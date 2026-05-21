const http = require('http');
const https = require('https');
const fs = require('fs');
const path = require('path');
const url = require('url');

const PORT = process.env.PORT || 3000;

// MIME types for static files
const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.css': 'text/css',
    '.js': 'application/javascript',
    '.json': 'application/json',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.ico': 'image/x-icon'
};

// Allowed video content types
const VIDEO_CONTENT_TYPES = [
    'video/', 'application/octet-stream', 'binary/octet-stream',
    'application/mp4', 'application/x-mpegURL', 'audio/', 'application/vnd'
];

function isAllowedContentType(contentType) {
    if (!contentType) return true; // Allow if no content-type header
    return VIDEO_CONTENT_TYPES.some(type => contentType.toLowerCase().includes(type));
}

// Parse request body
function parseBody(req) {
    return new Promise((resolve, reject) => {
        let body = '';
        req.on('data', chunk => { body += chunk; });
        req.on('end', () => {
            try {
                resolve(JSON.parse(body));
            } catch (e) {
                reject(new Error('Invalid JSON'));
            }
        });
        req.on('error', reject);
    });
}

// Make HTTP/HTTPS request following redirects
function fetchUrl(targetUrl, maxRedirects = 5) {
    return new Promise((resolve, reject) => {
        if (maxRedirects <= 0) {
            return reject(new Error('Too many redirects'));
        }

        const parsedUrl = new URL(targetUrl);
        const client = parsedUrl.protocol === 'https:' ? https : http;

        const options = {
            hostname: parsedUrl.hostname,
            port: parsedUrl.port,
            path: parsedUrl.pathname + parsedUrl.search,
            method: 'GET',
            headers: {
                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36',
                'Accept': '*/*',
                'Accept-Encoding': 'identity',
                'Connection': 'keep-alive'
            },
            timeout: 30000
        };

        const req = client.request(options, (res) => {
            // Handle redirects
            if ([301, 302, 303, 307, 308].includes(res.statusCode)) {
                const redirectUrl = res.headers.location;
                if (!redirectUrl) {
                    return reject(new Error('Redirect without Location header'));
                }
                const absoluteUrl = redirectUrl.startsWith('http')
                    ? redirectUrl
                    : new URL(redirectUrl, targetUrl).toString();
                return fetchUrl(absoluteUrl, maxRedirects - 1).then(resolve).catch(reject);
            }

            if (res.statusCode !== 200) {
                return reject(new Error(`Remote server returned HTTP ${res.statusCode}`));
            }

            resolve(res);
        });

        req.on('error', (e) => {
            reject(new Error(`Connection failed: ${e.message}`));
        });

        req.on('timeout', () => {
            req.destroy();
            reject(new Error('Connection timeout'));
        });

        req.end();
    });
}

// Serve static files
function serveStatic(req, res) {
    let filePath = req.url === '/' ? '/index.html' : req.url;
    filePath = path.join(__dirname, filePath.split('?')[0]);

    // Security: prevent directory traversal
    if (!filePath.startsWith(__dirname)) {
        res.writeHead(403);
        res.end('Forbidden');
        return;
    }

    const ext = path.extname(filePath);
    const contentType = MIME_TYPES[ext] || 'application/octet-stream';

    fs.readFile(filePath, (err, data) => {
        if (err) {
            res.writeHead(404);
            res.end('Not Found');
            return;
        }
        res.writeHead(200, { 'Content-Type': contentType });
        res.end(data);
    });
}

// Handle download API
async function handleDownload(req, res) {
    try {
        const body = await parseBody(req);
        const videoUrl = body.url;

        if (!videoUrl) {
            res.writeHead(400, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ error: 'Missing url parameter' }));
            return;
        }

        // Validate URL
        try {
            new URL(videoUrl);
        } catch (e) {
            res.writeHead(400, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ error: 'Invalid URL format' }));
            return;
        }

        console.log(`[Download] Fetching: ${videoUrl}`);

        const remoteRes = await fetchUrl(videoUrl);
        const contentType = remoteRes.headers['content-type'] || 'application/octet-stream';
        const contentLength = remoteRes.headers['content-length'];

        // Set response headers
        const headers = {
            'Content-Type': contentType,
            'Access-Control-Allow-Origin': '*'
        };

        if (contentLength) {
            headers['Content-Length'] = contentLength;
        }

        // Extract filename from Content-Disposition if available
        const disposition = remoteRes.headers['content-disposition'];
        if (disposition) {
            headers['Content-Disposition'] = disposition;
        }

        res.writeHead(200, headers);

        // Stream the response
        remoteRes.pipe(res);

        remoteRes.on('error', (err) => {
            console.error(`[Download] Stream error: ${err.message}`);
            if (!res.headersSent) {
                res.writeHead(500, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ error: 'Stream error' }));
            } else {
                res.end();
            }
        });

    } catch (error) {
        console.error(`[Download] Error: ${error.message}`);
        if (!res.headersSent) {
            res.writeHead(500, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ error: error.message }));
        }
    }
}

// Create server
const server = http.createServer(async (req, res) => {
    console.log(`[${new Date().toISOString()}] ${req.method} ${req.url}`);

    // CORS preflight
    if (req.method === 'OPTIONS') {
        res.writeHead(200, {
            'Access-Control-Allow-Origin': '*',
            'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
            'Access-Control-Allow-Headers': 'Content-Type'
        });
        res.end();
        return;
    }

    // API routes
    if (req.url === '/api/download' && req.method === 'POST') {
        await handleDownload(req, res);
        return;
    }

    // Health check
    if (req.url === '/api/health') {
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ status: 'ok', timestamp: new Date().toISOString() }));
        return;
    }

    // Static files
    serveStatic(req, res);
});

server.listen(PORT, () => {
    console.log(`\n==========================================`);
    console.log(`  Video Download Server`);
    console.log(`  Running at: http://localhost:${PORT}`);
    console.log(`==========================================\n`);
    console.log(`Open http://localhost:${PORT} in your browser.`);
    console.log(`Press Ctrl+C to stop.\n`);
});
