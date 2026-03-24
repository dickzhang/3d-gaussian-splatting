# OpenGL 4.3+ + C++ 3D Gaussian Splatting（Compute Shader）实施计划

## 目标
在 `E:\3d-gaussian-splatting` 搭建一个基于 C++ + OpenGL 4.3+ 的可运行项目，加载以下 PLY 数据并实现接近 UnityGaussianSplatting 的渲染效果：

- 输入模型：`I:\引擎工作组\模型\3DGS高斯泼溅\models\garden\point_cloud\iteration_30000\point_cloud.ply`

## 技术边界
- 渲染 API：OpenGL 4.3+ Core
- 语言：C++17+
- 平台：Windows
- 约束：使用 Compute Shader 完成每帧关键计算（深度计算、排序、视图缓存）

## 项目结构
建议目录：

- `E:\3d-gaussian-splatting\CMakeLists.txt`
- `E:\3d-gaussian-splatting\cmake\`
- `E:\3d-gaussian-splatting\external\`
- `E:\3d-gaussian-splatting\assets\shaders\`
- `E:\3d-gaussian-splatting\assets\configs\`
- `E:\3d-gaussian-splatting\src\app\`
- `E:\3d-gaussian-splatting\src\core\`
- `E:\3d-gaussian-splatting\src\io\`
- `E:\3d-gaussian-splatting\src\render\`
- `E:\3d-gaussian-splatting\src\scene\`
- `E:\3d-gaussian-splatting\src\util\`
- `E:\3d-gaussian-splatting\tests\`

模块职责：
- `app`：程序入口、主循环、输入与窗口生命周期
- `io`：PLY 解析与属性映射
- `scene`：高斯数据结构与相机
- `render`：OpenGL 资源、Pass、Shader
- `core`：日志、计时、配置
- `util`：数学与排序工具

## 依赖选型
- 构建：CMake + Ninja
- 依赖管理：vcpkg
- OpenGL Loader：GLAD（4.3+ Core）
- 窗口/输入：GLFW
- 数学库：GLM
- PLY 解析：tinyply
- 日志：spdlog
- 调试截图：stb_image_write
- 可选 UI：Dear ImGui

## 渲染路线（Compute Shader 方案）
1. 加载阶段（CPU）：
   - 解析 3DGS PLY 属性（位置、旋转、尺度、opacity、颜色/SH）
   - 预计算协方差表达并打包上传 GPU

2. 每帧阶段（Compute Shader）：
  - 在 GPU 计算 splat 到相机深度键值
  - 在 GPU 进行 radix sort（key-value，按深度 back-to-front）
  - 在 GPU 计算 view-dependent 数据（如屏幕空间椭圆参数）

3. 绘制阶段（GPU）：
   - 使用 instanced quad（每个 splat 一个 billboard 椭圆）
  - Vertex Shader 从 GPU 排序索引读取 splat，并基于协方差投影到屏幕椭圆
   - Fragment Shader 计算高斯衰减，输出预乘 alpha
   - 开启 blend，关闭 depth write（可选保留 depth test）

4. 颜色策略：
   - Phase A：先用 base color 或 SH0 快速成像
   - Phase B：补全 SH 视角相关颜色，提高接近 Unity 的观感

## 里程碑
1. 项目引导（1-2 天）
- 完成 CMake、窗口、OpenGL 4.3+ 上下文、相机控制
- 验收：能显示基础三角形

2. PLY 读取与校验（1-2 天）
- 读取目标文件，打印属性统计与数量
- 验收：成功加载 garden PLY，无崩溃

3. 最小可视化（2-3 天）
- 先绘制 unsorted 点/小四边形
- 验收：可看到 garden 点云整体形状

4. 高斯椭圆渲染（3-4 天）
- 协方差投影 + 高斯 fragment + blend
- 验收：得到软化的 splat 重建效果

5. 排序与剔除（2-4 天）
- Compute Shader 深度键计算 + GPU Radix Sort + GPU/CPU Frustum Culling
- 验收：透明叠加更稳定，拖动相机伪影减少，CPU 占用明显下降

6. 视觉对齐（3-5 天）
- SH、曝光/伽马、半径/alpha 参数调优
- 验收：与目标截图在关键机位下观感接近

7. 稳定化（1-2 天）
- 错误处理、配置文件、Release 构建说明
- 验收：一条命令构建 + 一条命令运行

## 风险与缓解
- PLY 属性名差异：
  - 方案：实现属性别名映射与严格日志

- 中文路径兼容：
  - 方案：全程使用 `std::filesystem::path`，确保 UTF-8/宽字符文件访问

- GPU 排序实现复杂度：
  - 方案：优先接入成熟的 GPU radix sort 实现，再做工程化封装

- OpenGL 版本兼容性：
  - 方案：启动时强校验 GL 4.3+，若不满足则提示降级到 CPU 路径或退出

- 填充率压力：
  - 方案：限制最大屏幕半径、早期 alpha discard、可选动态分辨率

- 透明顺序误差：
  - 方案：优先严格 back-to-front；必要时增加 Weighted Blended OIT 作为备选

## 成功标准
- 程序可在 Windows 上稳定构建运行
- 指定 PLY 可直接加载并渲染
- 结果与 Unity 插件截图在主要视角下接近
- 性能达到可交互（可先以功能正确为主，后续优化）

## 下一步执行建议
1. 先完成项目脚手架（CMake + GLFW + GLAD(4.3+) + GLM + tinyply）
2. 第一时间打通“加载指定 PLY + 可见点云”
3. 接入 Compute Shader 的深度计算与排序
4. 完成高斯椭圆渲染与 SH 调参
