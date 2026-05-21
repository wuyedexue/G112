const http = require('http');
const https = require('https');
const fs = require('fs');
const path = require('path');
const { execFile, spawn } = require('child_process');
const os = require('os');

const PORT = process.env.PORT || 3000;
const DOWNLOAD_DIR = path.join(__dirname, 'downloads');
const BIN_DIR = path.join(__dirname, 'bin');

// Ensure directories exist
if (!fs.existsSync(DOWNLOAD_DIR)) fs.mkdirSync(DOWNLOAD_DIR, { recursive: true });
if (!fs.existsSync(BIN_DIR)) fs.mkdirSync(BIN_DIR, { recursive: true });

// yt-dlp binary path
const isWindows = os.platform() === 'win32';
const ytdlpBinary = path.join(BIN_DIR, isWindows ? 'yt-dlp.exe' : 'yt-dlp');

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

// ========== yt-dlp Setup ==========

// Track yt-dlp download status for the frontend
let ytdlpDownloading = false;
let ytdlpDownloadError = null;

function getYtdlpDownloadUrls() {
    const platform = os.platform();
    const arch = os.arch();
    
    let filename;
    if (platform === 'win32') {
        filename = 'yt-dlp.exe';
    } else if (platform === 'darwin') {
        filename = 'yt-dlp_macos';
    } else {
        if (arch === 'arm64' || arch === 'aarch64') {
            filename = 'yt-dlp_linux_aarch64';
        } else {
            filename = 'yt-dlp_linux';
        }
    }

    const githubUrl = `https://github.com/yt-dlp/yt-dlp/releases/latest/download/${filename}`;
    
    // Multiple mirrors for users in China where GitHub is slow/blocked
    return [
        `https://ghfast.top/${githubUrl}`,
        `https://gh-proxy.com/${githubUrl}`,
        `https://github.moeyy.xyz/${githubUrl}`,
        githubUrl
    ];
}

function downloadFile(url, dest, timeoutMs = 30000, maxRedirects = 10) {
    return new Promise((resolve, reject) => {
        if (maxRedirects <= 0) return reject(new Error('Too many redirects'));
        
        const client = url.startsWith('https') ? https : http;
        const req = client.get(url, { 
            headers: { 'User-Agent': 'Mozilla/5.0' },
            timeout: timeoutMs
        }, (res) => {
            if ([301, 302, 303, 307, 308].includes(res.statusCode)) {
                const location = res.headers.location;
                if (!location) return reject(new Error('Redirect without location'));
                const absoluteUrl = location.startsWith('http') ? location : new URL(location, url).toString();
                return downloadFile(absoluteUrl, dest, timeoutMs, maxRedirects - 1).then(resolve).catch(reject);
            }
            if (res.statusCode !== 200) {
                return reject(new Error(`HTTP ${res.statusCode}`));
            }
            const file = fs.createWriteStream(dest);
            res.pipe(file);
            file.on('finish', () => {
                file.close(() => {
                    // Make executable on unix
                    if (!isWindows) {
                        fs.chmodSync(dest, '755');
                    }
                    resolve();
                });
            });
            file.on('error', (err) => {
                fs.unlink(dest, () => {});
                reject(err);
            });
        });
        req.on('error', reject);
        req.on('timeout', () => {
            req.destroy();
            reject(new Error(`Download timed out after ${timeoutMs / 1000}s`));
        });
    });
}

async function ensureYtdlp() {
    if (fs.existsSync(ytdlpBinary)) {
        console.log('[OK] yt-dlp found (bundled).');
        return true;
    }
    
    ytdlpDownloading = true;
    ytdlpDownloadError = null;
    
    const urls = getYtdlpDownloadUrls();
    console.log('[Setup] yt-dlp not found in bin/ folder, attempting download...');
    console.log('[Setup] (This is normal on first run if you deleted the bin/ folder)');
    
    for (let i = 0; i < urls.length; i++) {
        const url = urls[i];
        console.log(`[Setup] Trying source ${i + 1}/${urls.length}...`);
        try {
            await downloadFile(url, ytdlpBinary, 90000);
            console.log('[Setup] yt-dlp downloaded successfully!');
            ytdlpDownloading = false;
            return true;
        } catch (err) {
            console.error(`[Setup] Source ${i + 1} failed: ${err.message}`);
            // Clean up partial file
            try { fs.unlinkSync(ytdlpBinary); } catch (e) {}
            if (i < urls.length - 1) {
                console.log('[Setup] Trying next source...');
            }
        }
    }
    
    ytdlpDownloading = false;
    ytdlpDownloadError = 'All download sources failed. Please re-download the complete ZIP from GitHub.';
    console.error('[Setup] Failed to download yt-dlp from all sources.');
    console.error('[Setup] Solution: Re-download the complete project ZIP from:');
    console.error('[Setup]   https://github.com/wuyedexue/G112');
    console.error('[Setup]   The ZIP already includes yt-dlp.exe in the bin/ folder.');
    return false;
}

// ========== URL Normalization ==========

// Normalize video URLs so yt-dlp can recognize them
function normalizeUrl(url) {
    try {
        const parsed = new URL(url);
        
        // Douyin: convert modal_id URLs to proper video URLs
        // e.g. https://www.douyin.com/jingxuan?modal_id=123 -> https://www.douyin.com/video/123
        // e.g. https://www.douyin.com/discover?modal_id=123 -> https://www.douyin.com/video/123
        // e.g. https://www.douyin.com/search/xxx?modal_id=123 -> https://www.douyin.com/video/123
        if (parsed.hostname === 'www.douyin.com' || parsed.hostname === 'douyin.com') {
            const modalId = parsed.searchParams.get('modal_id');
            if (modalId && /^\d+$/.test(modalId)) {
                const normalizedUrl = `https://www.douyin.com/video/${modalId}`;
                console.log(`[URL] Normalized Douyin URL: ${url} -> ${normalizedUrl}`);
                return normalizedUrl;
            }
        }
        
        // Bilibili: convert various URL formats
        // e.g. https://www.bilibili.com/video/BV1xx411c7mD?p=1 (keep as-is, yt-dlp handles it)
        // e.g. https://b23.tv/xxxxx (short link, keep as-is, yt-dlp follows redirects)
        
        // Kuaishou: convert various formats
        // e.g. https://www.kuaishou.com/short-video/xxx?modal_id=xxx
        if (parsed.hostname === 'www.kuaishou.com' || parsed.hostname === 'kuaishou.com') {
            const modalId = parsed.searchParams.get('modal_id');
            if (modalId) {
                const normalizedUrl = `https://www.kuaishou.com/short-video/${modalId}`;
                console.log(`[URL] Normalized Kuaishou URL: ${url} -> ${normalizedUrl}`);
                return normalizedUrl;
            }
        }
        
        return url;
    } catch (e) {
        return url;
    }
}

// ========== Cookie Management ==========

const COOKIES_FILE = path.join(__dirname, 'cookies.txt');

// Detect available browsers for cookie extraction
function getAvailableBrowsers() {
    // On Windows, try these browsers in order of likelihood to work
    // Firefox doesn't have DPAPI encryption issues
    return ['firefox', 'chrome', 'edge', 'chromium', 'opera', 'brave'];
}

// Build yt-dlp args with cookie support
function buildCookieArgs() {
    // Priority 1: cookies.txt file in project directory
    if (fs.existsSync(COOKIES_FILE)) {
        console.log('[Cookie] Using cookies.txt file');
        return ['--cookies', COOKIES_FILE];
    }
    
    // Priority 2: Try to use browser cookies
    // Note: On Windows with newer Chrome/Edge, DPAPI may prevent access
    // Firefox usually works without issues
    const browsers = getAvailableBrowsers();
    for (const browser of browsers) {
        // We'll try firefox first as it doesn't have DPAPI issues
        if (browser === 'firefox') {
            return ['--cookies-from-browser', 'firefox'];
        }
    }
    
    return [];
}

// Try running yt-dlp with different cookie strategies
async function runYtdlpWithCookies(baseArgs, videoUrl) {
    const strategies = [];
    
    // Strategy 1: cookies.txt file (if exists)
    if (fs.existsSync(COOKIES_FILE)) {
        strategies.push({
            name: 'cookies.txt',
            args: [...baseArgs, '--cookies', COOKIES_FILE, videoUrl]
        });
    }
    
    // Strategy 2: Firefox cookies (most reliable on Windows)
    strategies.push({
        name: 'firefox cookies',
        args: [...baseArgs, '--cookies-from-browser', 'firefox', videoUrl]
    });
    
    // Strategy 3: Chrome cookies
    strategies.push({
        name: 'chrome cookies',
        args: [...baseArgs, '--cookies-from-browser', 'chrome', videoUrl]
    });
    
    // Strategy 4: Edge cookies
    strategies.push({
        name: 'edge cookies',
        args: [...baseArgs, '--cookies-from-browser', 'edge', videoUrl]
    });
    
    // Strategy 5: No cookies (works for sites that don't require them)
    strategies.push({
        name: 'no cookies',
        args: [...baseArgs, videoUrl]
    });
    
    let lastError = null;
    
    for (const strategy of strategies) {
        try {
            const result = await executeYtdlp(strategy.args);
            console.log(`[Cookie] Success with strategy: ${strategy.name}`);
            return result;
        } catch (err) {
            lastError = err;
            const errMsg = err.message || '';
            // If the error is "cookies needed", try next strategy
            // If it's a different error (like network), also try next
            // But if it's "Unsupported URL", no point trying other cookie strategies
            if (errMsg.includes('Unsupported URL') || errMsg.includes('is not a valid URL')) {
                throw err; // No cookie strategy will fix an invalid URL
            }
            console.log(`[Cookie] Strategy "${strategy.name}" failed: ${errMsg.substring(0, 100)}`);
            continue;
        }
    }
    
    throw lastError || new Error('All cookie strategies failed');
}

function executeYtdlp(args) {
    return new Promise((resolve, reject) => {
        execFile(ytdlpBinary, args, {
            timeout: 60000,
            maxBuffer: 10 * 1024 * 1024,
            env: { ...process.env, PYTHONDONTWRITEBYTECODE: '1' }
        }, (error, stdout, stderr) => {
            if (error) {
                const errMsg = stderr || error.message;
                return reject(new Error(errMsg.substring(0, 500)));
            }
            resolve(stdout);
        });
    });
}

// ========== Video Info & Download ==========

// Friendly error messages in Chinese
function friendlyError(errMsg) {
    if (errMsg.includes('Fresh cookies') || errMsg.includes('cookies')) {
        return '该网站需要浏览器Cookie才能解析视频。\n\n解决方法：\n1. 用Firefox浏览器访问并登录该视频网站\n2. 重新尝试解析\n\n或者：安装浏览器扩展"Get cookies.txt LOCALLY"，导出cookies.txt文件放到程序目录下，重启程序即可。';
    }
    if (errMsg.includes('Unsupported URL')) {
        return '不支持该链接格式。请确认链接是否正确，或尝试复制视频页面的完整URL。';
    }
    if (errMsg.includes('Video unavailable') || errMsg.includes('not available')) {
        return '视频不可用，可能已被删除或设为私密。';
    }
    if (errMsg.includes('403') || errMsg.includes('Forbidden')) {
        return '访问被拒绝，该视频可能需要登录或有地区限制。';
    }
    if (errMsg.includes('404') || errMsg.includes('Not Found')) {
        return '视频不存在，请检查链接是否正确。';
    }
    if (errMsg.includes('timed out') || errMsg.includes('timeout')) {
        return '连接超时，请检查网络连接后重试。';
    }
    if (errMsg.includes('DPAPI')) {
        return '无法读取浏览器Cookie（Windows安全限制）。\n\n解决方法：安装Firefox浏览器，用Firefox访问并登录该视频网站后重试。\n\n或者：安装浏览器扩展"Get cookies.txt LOCALLY"，导出cookies.txt文件放到程序目录下。';
    }
    return errMsg;
}

// Get video info using yt-dlp
async function getVideoInfo(videoUrl) {
    // Normalize the URL first
    videoUrl = normalizeUrl(videoUrl);
    
    const baseArgs = [
        '--dump-json',
        '--no-playlist',
        '--no-warnings'
    ];
    
    try {
        const stdout = await runYtdlpWithCookies(baseArgs, videoUrl);
        try {
            const info = JSON.parse(stdout);
            return info;
        } catch (e) {
            throw new Error('Failed to parse video info');
        }
    } catch (error) {
        const errMsg = error.message || 'Unknown error';
        throw new Error(friendlyError(errMsg));
    }
}

// Download video using yt-dlp and stream to response
function downloadVideo(videoUrl, formatId, res) {
    // Normalize the URL first
    videoUrl = normalizeUrl(videoUrl);
    
    const filename = `video_${Date.now()}`;
    const outputTemplate = path.join(DOWNLOAD_DIR, `${filename}.%(ext)s`);
    
    const args = [
        '-o', outputTemplate,
        '--no-playlist',
        '--no-warnings',
        '--newline', // Progress on new lines
    ];
    
    // Add cookie support
    if (fs.existsSync(COOKIES_FILE)) {
        args.push('--cookies', COOKIES_FILE);
    } else {
        // Try firefox cookies by default (most reliable on Windows)
        args.push('--cookies-from-browser', 'firefox');
    }
    
    if (formatId && formatId !== 'best') {
        args.push('-f', formatId);
    } else {
        args.push('-f', 'bestvideo[ext=mp4]+bestaudio[ext=m4a]/best[ext=mp4]/best');
    }
    
    args.push(videoUrl);
    
    return new Promise((resolve, reject) => {
        const proc = spawn(ytdlpBinary, args, {
            env: { ...process.env, PYTHONDONTWRITEBYTECODE: '1' }
        });
        
        let outputFile = null;
        let lastProgress = '';
        
        proc.stdout.on('data', (data) => {
            const line = data.toString().trim();
            if (line.includes('[download]') && line.includes('%')) {
                lastProgress = line;
            }
            // Detect merge/output filename
            const mergeMatch = line.match(/\[Merger\] Merging formats into "(.+)"/);
            const dlMatch = line.match(/\[download\] Destination: (.+)/);
            const alreadyMatch = line.match(/\[download\] (.+) has already been downloaded/);
            
            if (mergeMatch) outputFile = mergeMatch[1];
            else if (dlMatch && !outputFile) outputFile = dlMatch[1];
            else if (alreadyMatch) outputFile = alreadyMatch[1];
        });
        
        proc.stderr.on('data', (data) => {
            console.error(`[yt-dlp stderr] ${data.toString().trim()}`);
        });
        
        proc.on('close', (code) => {
            if (code !== 0) {
                return reject(new Error(`yt-dlp exited with code ${code}`));
            }
            
            // Find the output file
            if (!outputFile) {
                // Search for the file in downloads dir
                const files = fs.readdirSync(DOWNLOAD_DIR)
                    .filter(f => f.startsWith(filename))
                    .map(f => ({
                        name: f,
                        path: path.join(DOWNLOAD_DIR, f),
                        time: fs.statSync(path.join(DOWNLOAD_DIR, f)).mtimeMs
                    }))
                    .sort((a, b) => b.time - a.time);
                
                if (files.length > 0) {
                    outputFile = files[0].path;
                }
            }
            
            if (outputFile && fs.existsSync(outputFile)) {
                resolve(outputFile);
            } else {
                reject(new Error('Download completed but file not found'));
            }
        });
        
        proc.on('error', (err) => {
            reject(new Error(`Failed to start yt-dlp: ${err.message}`));
        });
    });
}

// ========== HTTP Server ==========

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

function serveStatic(req, res) {
    let filePath = req.url === '/' ? '/index.html' : req.url;
    filePath = path.join(__dirname, filePath.split('?')[0]);

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

// API: Parse video info
async function handleParse(req, res) {
    try {
        const body = await parseBody(req);
        const videoUrl = body.url;

        if (!videoUrl) {
            res.writeHead(400, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ error: 'Missing url parameter' }));
            return;
        }

        console.log(`[Parse] Analyzing: ${videoUrl}`);
        
        const info = await getVideoInfo(videoUrl);
        
        // Extract relevant info
        const formats = (info.formats || [])
            .filter(f => f.vcodec !== 'none' || f.acodec !== 'none')
            .map(f => ({
                format_id: f.format_id,
                ext: f.ext,
                resolution: f.resolution || (f.height ? `${f.width || '?'}x${f.height}` : 'audio only'),
                filesize: f.filesize || f.filesize_approx || null,
                vcodec: f.vcodec,
                acodec: f.acodec,
                fps: f.fps,
                tbr: f.tbr,
                format_note: f.format_note || ''
            }));

        const result = {
            title: info.title || 'Unknown',
            duration: info.duration || 0,
            thumbnail: info.thumbnail || null,
            uploader: info.uploader || info.channel || '',
            webpage_url: info.webpage_url || videoUrl,
            description: (info.description || '').substring(0, 200),
            formats: formats
        };

        res.writeHead(200, { 
            'Content-Type': 'application/json',
            'Access-Control-Allow-Origin': '*'
        });
        res.end(JSON.stringify(result));

    } catch (error) {
        console.error(`[Parse] Error: ${error.message}`);
        res.writeHead(500, { 
            'Content-Type': 'application/json',
            'Access-Control-Allow-Origin': '*'
        });
        res.end(JSON.stringify({ error: error.message }));
    }
}

// API: Download video
async function handleDownload(req, res) {
    try {
        const body = await parseBody(req);
        const videoUrl = body.url;
        const formatId = body.format || 'best';

        if (!videoUrl) {
            res.writeHead(400, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ error: 'Missing url parameter' }));
            return;
        }

        console.log(`[Download] Starting download: ${videoUrl} (format: ${formatId})`);
        
        const filePath = await downloadVideo(videoUrl, formatId, res);
        const fileName = path.basename(filePath);
        const stat = fs.statSync(filePath);
        
        // Stream the file to client
        res.writeHead(200, {
            'Content-Type': 'application/octet-stream',
            'Content-Length': stat.size,
            'Content-Disposition': `attachment; filename="${encodeURIComponent(fileName)}"`,
            'Access-Control-Allow-Origin': '*',
            'X-File-Name': encodeURIComponent(fileName),
            'X-File-Size': stat.size
        });

        const stream = fs.createReadStream(filePath);
        stream.pipe(res);
        
        stream.on('end', () => {
            // Clean up downloaded file after sending
            setTimeout(() => {
                fs.unlink(filePath, () => {});
            }, 5000);
        });

    } catch (error) {
        console.error(`[Download] Error: ${error.message}`);
        if (!res.headersSent) {
            res.writeHead(500, { 
                'Content-Type': 'application/json',
                'Access-Control-Allow-Origin': '*'
            });
            res.end(JSON.stringify({ error: error.message }));
        }
    }
}

// API: Check yt-dlp status
function handleStatus(req, res) {
    const ready = fs.existsSync(ytdlpBinary);
    const hasCookies = fs.existsSync(COOKIES_FILE);
    res.writeHead(200, { 
        'Content-Type': 'application/json',
        'Access-Control-Allow-Origin': '*'
    });
    res.end(JSON.stringify({ 
        ready: ready,
        downloading: ytdlpDownloading,
        error: ytdlpDownloadError,
        hasCookies: hasCookies,
        platform: os.platform(),
        arch: os.arch()
    }));
}

// API: Upload cookies.txt file
async function handleUploadCookies(req, res) {
    try {
        let body = '';
        await new Promise((resolve, reject) => {
            req.on('data', chunk => { body += chunk; });
            req.on('end', resolve);
            req.on('error', reject);
        });
        
        // Parse multipart or raw text
        let cookieContent = body;
        
        // If it's JSON with a content field
        try {
            const json = JSON.parse(body);
            if (json.content) cookieContent = json.content;
        } catch (e) {
            // Not JSON, use raw body as cookie content
        }
        
        if (!cookieContent || cookieContent.trim().length === 0) {
            res.writeHead(400, { 'Content-Type': 'application/json', 'Access-Control-Allow-Origin': '*' });
            res.end(JSON.stringify({ error: 'Empty cookie content' }));
            return;
        }
        
        fs.writeFileSync(COOKIES_FILE, cookieContent, 'utf-8');
        console.log('[Cookie] cookies.txt file saved successfully');
        
        res.writeHead(200, { 'Content-Type': 'application/json', 'Access-Control-Allow-Origin': '*' });
        res.end(JSON.stringify({ success: true, message: 'Cookies saved successfully' }));
    } catch (error) {
        res.writeHead(500, { 'Content-Type': 'application/json', 'Access-Control-Allow-Origin': '*' });
        res.end(JSON.stringify({ error: error.message }));
    }
}

// Create HTTP server
const server = http.createServer(async (req, res) => {
    console.log(`[${new Date().toISOString()}] ${req.method} ${req.url}`);

    // CORS preflight
    if (req.method === 'OPTIONS') {
        res.writeHead(200, {
            'Access-Control-Allow-Origin': '*',
            'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
            'Access-Control-Allow-Headers': 'Content-Type',
            'Access-Control-Expose-Headers': 'X-File-Name, X-File-Size'
        });
        res.end();
        return;
    }

    const parsedUrl = req.url.split('?')[0];

    // API routes
    if (parsedUrl === '/api/parse' && req.method === 'POST') {
        await handleParse(req, res);
        return;
    }

    if (parsedUrl === '/api/download' && req.method === 'POST') {
        await handleDownload(req, res);
        return;
    }

    if (parsedUrl === '/api/status' && req.method === 'GET') {
        handleStatus(req, res);
        return;
    }

    if (parsedUrl === '/api/upload-cookies' && req.method === 'POST') {
        await handleUploadCookies(req, res);
        return;
    }

    // Static files
    serveStatic(req, res);
});

// ========== Start Server ==========

async function main() {
    console.log('\n==========================================');
    console.log('  Video Download Tool - yt-dlp Backend');
    console.log('==========================================\n');

    // Start HTTP server FIRST so the page is accessible immediately
    server.listen(PORT, () => {
        console.log(`[OK] Server running at: http://localhost:${PORT}`);
        console.log(`     Open this URL in your browser to use the tool.`);
        console.log(`\n     Press Ctrl+C to stop.\n`);
    });

    // Then check/download yt-dlp in the background (non-blocking)
    const ready = await ensureYtdlp();
    if (!ready) {
        console.error('\n[WARN] yt-dlp is not available. Video parsing will not work until it is installed.');
        console.error('[WARN] You can manually download yt-dlp.exe and place it in the bin/ folder.');
        console.error('[WARN] Download link: https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe\n');
    } else {
        console.log('[OK] yt-dlp is ready. You can now parse and download videos.\n');
    }
}

main();
