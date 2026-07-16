function normalizePoint(point) {
    return {
        x: Number(point.x),
        y: Number(point.y)
    };
}

function parseCoordinates(text) {
    if (!text || typeof text !== 'string') return [];

    const points = [];
    const pattern = /\(?\s*(-?\d+(?:\.\d+)?)\s*[,，\s]+\s*(-?\d+(?:\.\d+)?)\s*\)?/g;
    let match;

    while ((match = pattern.exec(text)) !== null) {
        const x = Number(match[1]);
        const y = Number(match[2]);
        if (Number.isFinite(x) && Number.isFinite(y)) {
            points.push({ x, y });
        }
    }

    return points;
}

function pointOnSegment(point, a, b) {
    const cross = (point.y - a.y) * (b.x - a.x) - (point.x - a.x) * (b.y - a.y);
    if (Math.abs(cross) > 1e-9) return false;

    const minX = Math.min(a.x, b.x) - 1e-9;
    const maxX = Math.max(a.x, b.x) + 1e-9;
    const minY = Math.min(a.y, b.y) - 1e-9;
    const maxY = Math.max(a.y, b.y) + 1e-9;
    return point.x >= minX && point.x <= maxX && point.y >= minY && point.y <= maxY;
}

function pointInPolygon(pointInput, polygonInput) {
    const point = normalizePoint(pointInput);
    const polygon = (polygonInput || []).map(normalizePoint).filter(p => Number.isFinite(p.x) && Number.isFinite(p.y));

    if (polygon.length < 3) return false;

    let inside = false;
    for (let i = 0, j = polygon.length - 1; i < polygon.length; j = i++) {
        const pi = polygon[i];
        const pj = polygon[j];

        if (pointOnSegment(point, pi, pj)) return true;

        const intersects = ((pi.y > point.y) !== (pj.y > point.y))
            && point.x < ((pj.x - pi.x) * (point.y - pi.y)) / (pj.y - pi.y) + pi.x;
        if (intersects) inside = !inside;
    }

    return inside;
}

function getBoundingBox(pointsInput) {
    const points = (pointsInput || []).map(normalizePoint).filter(p => Number.isFinite(p.x) && Number.isFinite(p.y));
    if (points.length === 0) return null;

    return points.reduce((box, point) => ({
        minX: Math.min(box.minX, point.x),
        maxX: Math.max(box.maxX, point.x),
        minY: Math.min(box.minY, point.y),
        maxY: Math.max(box.maxY, point.y)
    }), {
        minX: points[0].x,
        maxX: points[0].x,
        minY: points[0].y,
        maxY: points[0].y
    });
}

function pointInBoundingBox(pointInput, pointsInput) {
    const point = normalizePoint(pointInput);
    const box = getBoundingBox(pointsInput);
    if (!box) return false;

    return point.x >= box.minX && point.x <= box.maxX && point.y >= box.minY && point.y <= box.maxY;
}

function evaluatePoints(pointsInput, rangePointsInput, mode = 'polygon') {
    const rangePoints = rangePointsInput || [];
    return (pointsInput || []).map((point, index) => {
        const normalized = {
            id: point.id || `pt-${index + 1}`,
            x: Number(point.x),
            y: Number(point.y),
            source: point.source || 'manual',
            confidence: Number.isFinite(Number(point.confidence)) ? Number(point.confidence) : null,
            selected: point.selected !== false
        };

        const inRange = mode === 'bbox'
            ? pointInBoundingBox(normalized, rangePoints)
            : pointInPolygon(normalized, rangePoints);

        return {
            ...normalized,
            inRange,
            confirmed: point.confirmed === true || (point.confirmed == null && inRange),
            exportName: point.exportName || ''
        };
    });
}

function padNumber(value, width = 3) {
    return String(value).padStart(width, '0');
}

function assignExportNames(pointsInput, prefix = 'coord') {
    const safePrefix = String(prefix || 'coord').trim() || 'coord';
    let count = 1;

    return (pointsInput || []).map(point => {
        if (!point.confirmed) return { ...point, exportName: '' };
        const exportName = `${safePrefix}_${padNumber(count)}`;
        count += 1;
        return { ...point, exportName };
    });
}

function getExportRows(pointsInput, prefix = 'coord') {
    return assignExportNames(pointsInput, prefix)
        .filter(point => point.confirmed)
        .map(point => ({
            name: point.exportName,
            x: point.x,
            y: point.y,
            source: point.source,
            confidence: point.confidence,
            inRange: point.inRange,
            confirmed: point.confirmed
        }));
}

function escapeCsv(value) {
    if (value == null) return '';
    const text = String(value);
    if (/[",\r\n]/.test(text)) return `"${text.replace(/"/g, '""')}"`;
    return text;
}

function exportCsv(pointsInput, prefix = 'coord') {
    const rows = getExportRows(pointsInput, prefix);
    const headers = ['name', 'x', 'y', 'source', 'confidence', 'inRange', 'confirmed'];
    const lines = [headers.join(',')];

    for (const row of rows) {
        lines.push(headers.map(header => escapeCsv(row[header])).join(','));
    }

    return lines.join('\n');
}

function exportJson(pointsInput, prefix = 'coord') {
    return JSON.stringify({
        prefix: String(prefix || 'coord').trim() || 'coord',
        exportedAt: new Date().toISOString(),
        points: getExportRows(pointsInput, prefix)
    }, null, 2);
}

function createMockRecognition(options = {}) {
    const width = Number(options.imageWidth) || 960;
    const height = Number(options.imageHeight) || 640;
    const name = options.imageName || 'uploaded-image';

    const rawPoints = [
        { x: Math.round(width * 0.22), y: Math.round(height * 0.25), confidence: 0.93 },
        { x: Math.round(width * 0.52), y: Math.round(height * 0.48), confidence: 0.88 },
        { x: Math.round(width * 0.78), y: Math.round(height * 0.68), confidence: 0.81 }
    ];

    return {
        provider: 'mock-ocr',
        imageName: name,
        imageWidth: width,
        imageHeight: height,
        points: rawPoints.map((point, index) => ({
            id: `mock-${index + 1}`,
            x: point.x,
            y: point.y,
            source: `mock-ocr:${name}`,
            confidence: point.confidence,
            selected: true
        }))
    };
}

const api = {
    parseCoordinates,
    pointInPolygon,
    pointInBoundingBox,
    getBoundingBox,
    evaluatePoints,
    assignExportNames,
    getExportRows,
    exportCsv,
    exportJson,
    createMockRecognition
};

if (typeof module !== 'undefined' && module.exports) {
    module.exports = api;
}

if (typeof window !== 'undefined') {
    window.CoordinateCore = api;
}
