# 3D 弹球小游戏 - UE4.26 项目

## 项目概述

基于 Unreal Engine 4.26 的 3D 弹球（Pinball）游戏，包含完整的物理系统、数值系统和UI界面。

## 项目结构

```
PinballGame/
├── PinballGame.uproject          # UE4项目文件
├── Source/
│   ├── PinballGame.Target.cs     # 游戏构建目标
│   ├── PinballGameEditor.Target.cs # 编辑器构建目标
│   └── PinballGame/
│       ├── PinballGame.Build.cs   # 模块构建配置
│       ├── PinballGame.h/.cpp     # 模块入口
│       ├── PinballGameMode.h/.cpp # 游戏模式（分数/生命/流程）
│       ├── PinballPlayerController.h/.cpp # 玩家输入控制
│       ├── PinballBall.h/.cpp     # 弹球（物理球体）
│       ├── PinballFlipper.h/.cpp  # 挡板（左右翻转器）
│       ├── PinballBumper.h/.cpp   # 保险杠（碰撞加分）
│       ├── PinballLauncher.h/.cpp # 发射器（弹簧蓄力）
│       ├── PinballTable.h/.cpp    # 弹球台面（物理边界）
│       ├── PinballRamp.h/.cpp     # 轨道/坡道（加分通道）
│       ├── PinballHUD.h/.cpp      # 游戏UI界面
│       └── PinballSaveGame.h/.cpp # 存档系统
├── Config/
│   ├── DefaultEngine.ini          # 引擎配置（物理参数）
│   ├── DefaultGame.ini            # 游戏设置
│   ├── DefaultInput.ini           # 输入映射
│   └── DefaultEditor.ini          # 编辑器配置
└── Content/
    ├── Blueprints/                # 蓝图资产
    ├── Maps/                      # 关卡地图
    ├── Materials/                 # 材质
    └── UI/                        # UI Widget蓝图
```

## 操作说明

| 按键 | 功能 |
|------|------|
| A / 左方向键 / Z | 左挡板 |
| D / 右方向键 / / | 右挡板 |
| 空格 / 下方向键 / Enter | 发射器（按住蓄力，松开发射） |
| Q | 左推台面 |
| E | 右推台面 |
| R | 重新开始（游戏结束后） |

## 数值系统

### 分数
- 圆形弹射器: 100分
- 三角形挡板: 50分
- 蘑菇头: 500分
- 击落目标: 1000分
- 完成轨道: 2000分 + 连续奖励

### 连击倍率
- 每次击中弹射器增加1x倍率
- 最高10x倍率
- 球掉落时重置

### 生命系统
- 初始3条命
- 每50000分奖励1条命
- 生命归零 = 游戏结束

## 快速开始

### 前置条件
- Unreal Engine 4.26 已安装
- Visual Studio 2019 (推荐)

### 步骤

1. **打开项目**
   - 双击 `PinballGame.uproject`
   - 或从 UE4 编辑器 "Browse" 选择该文件

2. **等待编译**
   - 首次打开会自动编译C++代码
   - 编译成功后进入编辑器

3. **创建关卡**
   - 新建一个空关卡 (File > New Level > Empty Level)
   - 保存到 Content/Maps/PinballMap

4. **搭建弹球台**
   - 拖入 `PinballTable` Actor（台面+墙壁）
   - 拖入 2个 `PinballFlipper` Actor（设置Left/Right标签）
   - 拖入 `PinballLauncher` Actor
   - 拖入若干 `PinballBumper` Actor
   - 可选：拖入 `PinballRamp` Actor

5. **设置GameMode**
   - World Settings > GameMode Override = PinballGameMode
   - 在GameMode的BallClass属性中设置球的蓝图类

6. **创建UI**
   - 基于 PinballHUD 类创建 UMG Widget Blueprint
   - 布局分数、生命、倍率等UI元素

7. **运行测试**
   - 点击 Play 即可测试

## 物理说明

- 球使用 PhysX 物理模拟
- 启用 CCD（连续碰撞检测）防止穿墙
- 台面有 6.5° 倾斜角模拟真实弹球台
- 挡板使用 Kinematic 运动学模式
- 弹射器通过 Impulse 施加冲量

## 扩展建议

1. **美术资源**: 替换 BasicShapes 为自定义3D模型
2. **音效**: 为每种碰撞添加不同音效
3. **粒子特效**: 球碰撞时的火花/光效
4. **多关卡**: 不同台面布局
5. **成就系统**: 基于分数和连击的解锁
6. **球道灯光**: 根据游戏状态变化的霓虹灯效果
