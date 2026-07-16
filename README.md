# MetroRunning

> 字符画伪 3D 地铁跑酷游戏 · C++17 · 南京大学大一 C 程序项目（C++ 复刻）

躲避障碍、收集金币和道具的终端跑酷游戏。使用字符画实现伪 3D 视觉效果，支持双轨道随机生成。

## 快速开始

```bash
# 构建
cmake -B build
cmake --build build --config Release

# 运行
build\Release\MetroRunning.exe
```

## 项目结构

```
MetroRunning/
├── MetroRunningA.cpp     # C++ 游戏源码（~950 行）
├── CMakeLists.txt         # CMake 构建配置
├── .github/workflows/     # CI（GitHub Actions）
├── README.md
└── LICENSE                # MIT
```

## 更新日志

- C 语言改为 C++ 语言编写
- 障碍、金币和道具添加颜色
- 更新随机逻辑，修复两边轨道长时间不出现障碍或出现相同障碍的问题
