# 3D Gaussian Splatting（OpenGL Compute）

这是一个基于 `C++17 + OpenGL 4.3` 的 3D Gaussian Splat 查看器。

当前实现包含：
- PLY 高斯模型加载（`binary_little_endian` 与 `ascii`）
- Compute Shader 深度键生成
- Compute Shader bitonic 排序
- 基于 instanced quad 的高斯泼溅渲染
- SH 视角相关着色（支持到 3 阶）

---

## 环境要求

- Windows
- CMake `3.23+`
- Visual Studio 2022（MSVC）
- 本地第三方库目录 `libs/`（项目内已按以下路径引用）：
  - `libs/GLAD`
  - `libs/GLFW`
  - `libs/glm`
  - `libs/stb`

---

## 配置与构建

### 方案一：Ninja（推荐）

```powershell
cmake -S . -B build -G Ninja
cmake --build build
```

### 方案二：Visual Studio 2022

```powershell
cmake -S . -B build-local -G "Visual Studio 17 2022" -A x64
cmake --build build-local --config Release
```
---

## 运行

### Ninja 生成目录

```powershell
./build/gaussian_splatting_gl.exe
```

### VS 生成目录

```powershell
./build-local/Release/gaussian_splatting_gl.exe
```

---

## 操作控制（当前主程序）

- 鼠标右键 + 鼠标移动：环视
- `W / A / S / D`：平移
- `Q / E`：上升 / 下降
- 关闭窗口按钮：退出

> 当前 `main.cpp` 为精简交互主循环，未启用截图/批处理快捷键流程。

---

## SH 说明（已对齐到 3 阶能力）

- SH0（DC）来自 `f_dc_0..2`
- SH1 使用 `f_rest_0..8`
- SH2 使用 `f_rest_9..23`
- SH3 使用 `f_rest_24..44`

程序会根据模型实际字段自动检测最大可用阶数。

---

## 模型路径配置

编辑文件：

- `assets/configs/model_path.txt`

填入待加载的 `.ply` 绝对路径或相对路径。

---

## 当前对齐目标

当前工作重点为：与 Unity 插件渲染链路进行观感对齐（排序、混合、SH、泼溅形状与合成路径）。
