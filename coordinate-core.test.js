const assert = require('assert');
const {
    parseCoordinates,
    pointInPolygon,
    pointInBoundingBox,
    evaluatePoints,
    exportCsv,
    exportJson,
    createMockRecognition
} = require('./coordinate-core');

function test(name, fn) {
    try {
        fn();
        console.log(`[PASS] ${name}`);
    } catch (error) {
        console.error(`[FAIL] ${name}`);
        throw error;
    }
}

test('parseCoordinates supports common coordinate formats', () => {
    assert.deepStrictEqual(parseCoordinates('10,20\n(30, 40)\n-5.5 6'), [
        { x: 10, y: 20 },
        { x: 30, y: 40 },
        { x: -5.5, y: 6 }
    ]);
});

test('pointInPolygon includes inside and edge points', () => {
    const polygon = [{ x: 0, y: 0 }, { x: 10, y: 0 }, { x: 10, y: 10 }, { x: 0, y: 10 }];
    assert.strictEqual(pointInPolygon({ x: 5, y: 5 }, polygon), true);
    assert.strictEqual(pointInPolygon({ x: 10, y: 5 }, polygon), true);
    assert.strictEqual(pointInPolygon({ x: 11, y: 5 }, polygon), false);
});

test('pointInBoundingBox checks min and max range', () => {
    const range = [{ x: 10, y: 20 }, { x: 30, y: 40 }, { x: 20, y: 60 }];
    assert.strictEqual(pointInBoundingBox({ x: 25, y: 50 }, range), true);
    assert.strictEqual(pointInBoundingBox({ x: 5, y: 50 }, range), false);
});

test('evaluatePoints applies in-range default confirmation', () => {
    const range = [{ x: 0, y: 0 }, { x: 100, y: 0 }, { x: 100, y: 100 }, { x: 0, y: 100 }];
    const result = evaluatePoints([{ id: 'a', x: 50, y: 50 }, { id: 'b', x: 150, y: 50 }], range);
    assert.strictEqual(result[0].confirmed, true);
    assert.strictEqual(result[1].confirmed, false);
});

test('exportCsv and exportJson only include confirmed points with prefix names', () => {
    const points = [
        { x: 1, y: 2, source: 'mock', confidence: 0.9, inRange: true, confirmed: true },
        { x: 3, y: 4, source: 'mock', confidence: 0.8, inRange: true, confirmed: false }
    ];
    assert.strictEqual(exportCsv(points, 'enemy').split('\n')[1].startsWith('enemy_001,1,2'), true);
    const data = JSON.parse(exportJson(points, 'enemy'));
    assert.strictEqual(data.points.length, 1);
    assert.strictEqual(data.points[0].name, 'enemy_001');
});

test('createMockRecognition returns stable replacement API shape', () => {
    const result = createMockRecognition({ imageName: 'map.png', imageWidth: 1000, imageHeight: 500 });
    assert.strictEqual(result.provider, 'mock-ocr');
    assert.strictEqual(result.points.length, 3);
    assert.strictEqual(result.points[0].selected, true);
});

console.log('All coordinate core tests passed.');
