# 坐标工具 MVP

这是一个可本地运行的坐标识别、范围校验与数据导出工具。用户无需理解代码，只要安装 Node.js 后按启动命令打开网页即可使用。

## 工具位置

- 目录：`G112`
- 启动命令：`npm install` 后运行 `npm start`
- 访问地址：`http://localhost:3000`

## 最短使用路径

```bash
npm install
npm start
```

启动成功后，在浏览器打开：

```text
http://localhost:3000
```

Windows 用户也可以双击 `启动.bat`；macOS/Linux 用户可以运行 `./start.sh`。

## 功能说明

### 1. 上传图片

点击页面左上角的“上传图片”，选择一张地图、截图或包含坐标点的图片。图片会显示在预览区，后续候选点和范围边界会叠加在图片上。

### 2. 识别坐标

上传图片后点击“识别坐标”。当前 MVP 提供可替换的 mock OCR 接口：

```text
POST /api/recognize
```

接口返回结构稳定，后续接入真实 OCR 服务时保持以下字段即可：

```json
{
  "provider": "mock-ocr",
  "points": [
    { "id": "mock-1", "x": 120, "y": 180, "source": "mock-ocr:image.png", "confidence": 0.93, "selected": true }
  ]
}
```

### 3. 输入范围坐标

在“范围点”输入框中输入至少 3 个点，支持以下格式：

```text
100,100
(700,120)
760 460
140,520
```

点击“确认坐标范围”后，工具会绘制范围边界并重新判断候选点是否在范围内。

范围算法可选择：

- `多边形内点判断`：适合不规则区域，边界点算作范围内。
- `矩形包围盒`：取所有范围点的最小/最大 x、y 形成矩形范围。

### 4. 确认坐标

识别结果会显示在“候选坐标确认”列表中，每条坐标包含：坐标值、来源、置信度、是否在范围内、保留状态、确认状态。

- 勾选“保留”：表示该点仍参与当前列表。
- 勾选“确认”：表示该点会被导出。
- 点击“剔除”：从候选列表中移除该点。
- 点击“添加手工坐标”：可手动粘贴或输入坐标补充候选点。

### 5. 前缀命名

在“前缀命名”中输入导出前缀，例如：

```text
enemy_spawn
```

导出名称会自动生成：

```text
enemy_spawn_001
enemy_spawn_002
enemy_spawn_003
```

### 6. 导出 CSV/JSON

选择导出格式：

- `CSV`：适合 Excel、表格工具或数据流水线。
- `JSON`：适合程序读取和后续服务端处理。

点击“导出文件”会下载文件；点击“复制预览”会将当前导出内容复制到剪贴板。工具只导出已确认的坐标。

CSV 字段：

```text
name,x,y,source,confidence,inRange,confirmed
```

JSON 字段：

```json
{
  "prefix": "coord",
  "exportedAt": "2026-07-16T00:00:00.000Z",
  "points": [
    { "name": "coord_001", "x": 100, "y": 200, "source": "mock", "confidence": 0.9, "inRange": true, "confirmed": true }
  ]
}
```

## 验证命令

```bash
npm test
node --check server.js
node --check coordinate-core.js
```

## 联调说明

- 当前识别能力为 mock OCR，真实 OCR 服务只需替换 `server.js` 中 `/api/recognize` 的实现。
- 坐标系默认使用图片自然像素坐标；如果真实服务返回裁剪或缩放坐标，需要同步返回转换参数。
- 导出文件由浏览器本地生成，移动端浏览器下载行为建议补充真机验证。
