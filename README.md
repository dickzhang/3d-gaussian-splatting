# 3D Gaussian Splatting（OpenGL Compute）

这是一个基于 C++/OpenGL 4.3+ 的 Gaussian Splat 查看器骨架，已实现 GPU 深度键生成与 GPU bitonic 排序。

## 环境要求
- Windows
- CMake 3.23+
- Visual Studio 2022（MSVC）
- `libs/` 目录下的本地第三方库：
  - `GLAD`
  - `GLFW`
  - `glm`

## 配置
```powershell
cmake -S . -B build-local -G "Visual Studio 17 2022" -A x64
```

## 构建
```powershell
cmake --build build-local --config Release
```

## 运行
```powershell
./build-local/Release/gaussian_splatting_gl.exe
```

## 操作控制
- 鼠标右键 + 鼠标移动：环视
- W/A/S/D：移动
- Q/E：上升/下降
- Esc：退出
- T：切换各向异性模式
- Y：循环切换 SH 阶数（0/1/2，0 为关闭）
- R：切换效果模式（original / Unity-like）用于同视角对比
- F6：自动输出同机位对比截图（original / Unity-like / diff）到 `captures/`
- F12：保存当前帧截图到 `captures/`
- F10：保存对比图（各向异性 OFF 与 ON）到 `captures/`
- F9：保存固定机位批量对比图（OFF 与 ON）到 `captures/`
- F8：保存固定机位 SH 基线图（按模型支持阶数导出 0..N）到 `captures/`
- F7：已临时停用（当前阶段聚焦效果对比）

当前工作重点：与 Unity 插件结果做同视角效果对齐。

SH 说明：
- 阶数 1 使用 `f_rest_0..8`
- 阶数 2 额外使用 `f_rest_9..23`
- 若模型不含二阶数据，切到 2 时会自动退化为接近一阶效果
- 若请求阶数高于模型支持阶数，会自动降级并在控制台打印提示

## 模型路径
编辑 `assets/configs/model_path.txt` 以修改输入 PLY 路径。

默认值：

`I:/引擎工作组/模型/3DGS高斯泼溅/models/garden/point_cloud/iteration_30000/point_cloud.ply`

## 批量截图预设
F9/F8 批量截图会读取 `assets/configs/capture_presets.txt` 中的预设。

每行格式：

`name px py pz yaw pitch`

示例：

`front 0.0 1.8 6.5 -90.0 -8.0`

说明：
- 空行会被忽略
- `#` 表示注释起始
- 如果文件缺失或为空，会使用内置默认预设

F8 输出命名规则：
- `batch_sh_<preset>_d<degree>_<timestamp>.png`

F6 输出命名规则：
- `look_original_<timestamp>.png`
- `look_unity_<timestamp>.png`
- `look_diff_<timestamp>.png`（灰度差分热度图）

无交互自动执行对比：
- 设置 `GS_AUTO_LOOK_COMPARE=1`：启动后自动执行一次 F6 同等对比
- 叠加 `GS_AUTO_LOOK_COMPARE_EXIT=1`：对比完成后自动退出程序

F7 说明：
- benchmark 功能代码保留，但运行入口已停用；后续对齐完成后可再恢复。

## 当前状态
- 已实现 PLY 加载（binary little-endian 与 ascii 顶点属性）
- 已实现用于深度键生成的 Compute Shader Pass
- 已实现 Compute Shader bitonic 排序
- 已实现点高斯渲染路径
- 已实现 SH 视角相关颜色（支持 0/1/2 阶运行时切换）
- 下一步：进一步画质调优与性能优化
