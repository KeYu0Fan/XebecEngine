# XebecEngine

个人制作的游戏引擎（开发中）。

## 状态

- 语言：C（C11）
- 平台：Windows（后续扩展 Linux / Android）
- 模块：core（math / struct / system / as / resmgr / diag / mory）
- 上层骨架：drivers / platform / servers / scene / editor / modules
- 构建：构建脚本待补充

## 目录

```
core      核心：算法/容器/数学/内存/系统调用
drivers   驱动：图形/音频/输入/摄像头/传感器/视频
platform  平台：窗口/事件/上下文
servers   服务：display/render/audio/input/physics/navigation/text
scene     场景：节点树/资源/2D/3D/GUI
editor    编辑器
modules   可选模块
doc       文档与测试
```

## 文档

- `doc/架构.txt`：分层依赖、类型擦除、接口放置规则
- `doc/结构.txt`：目录与文件清单
- `doc/godot结构.txt`：Godot 结构参考

## 许可

MIT
