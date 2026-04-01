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
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```
---

## 运行

先生成缓存（VS 目录示例）：

```powershell
./build/Release/gs_cache_builder.exe <input.ply>
```

默认会把缓存写到原始 PLY 文件同目录，文件名为 `<input_stem>.gsplatcache`。

也可显式指定输出文件名：

```powershell
./build/Release/gs_cache_builder.exe <input.ply> <custom_name.gsplatcache>
```

无论是否传第二个参数，manifest 和所有 payload 都会写到原始 `.ply` 所在目录。
第二个参数只控制 manifest 文件名，不再改变输出目录。

当前缓存格式为 v4 manifest + payload 布局：

- `<name>.gsplatcache`：manifest 入口，记录版本、splat 数量、bounds、payload 文件表、checksum
- `<name>.pos.byte`
- `<name>.other.byte`
- `<name>.color.byte`
- `<name>.sh.byte`
- `<name>.chunk.byte`（可选，包含连续 splat 区间与每个 chunk 的包围球）

运行时优先使用当前 v4 `.gsplatcache` manifest 入口；旧 v3 cache 的 chunk 载荷仍可读取，但会回退到兼容路径。

如需生成不带 chunk 的缓存：

```powershell
./build/Release/gs_cache_builder.exe --no-chunk <input.ply>
```

默认情况下，builder 会以 `2048` 个 splats 为一组生成真实多 chunk payload。运行时现在会优先在 GPU 侧按 chunk 包围球做可见 schedule/compaction，直接写入 seeded indices，再仅对这个子集执行 depth/sort/view-data/draw；CPU 侧的 chunk visible-index 构建只保留为 fallback。`view_data.comp` 在 seeded path 上会直接信任这个预剔除后的 active domain，不再重复做 per-splat chunk 查找与 coarse culling；退回 full-domain 时仍保留原有兜底检查。

当可见 splat 比例接近全量时，renderer 会自适应退回 full-domain 路径，避免每帧上传接近全量的 visible indices。当前默认策略为：

- 可见比例 `< 0.80`：启用 seeded indices 路径，缩减排序域
- 可见比例 `>= 0.90`：退回 full-domain 路径
- 中间区间：保持上一帧路径，避免频繁抖动

当前调度会先以 chunk 为粒度生成 `scheduled_ranges`，并统一通过 GPU `schedule_compact` pass 展开到 `indices_buffer`，再供既有 depth/sort 路径消费。这意味着主路径已经去掉了每帧 CPU 逐 splat visible-index 构建与上传热路径；producer 端与 compaction 端现在都在 GPU 上完成。当前 downstream 的真实语义是：CPU 生成的 schedule 若按 `outputOffset` 有序，可直接按 `scheduled_ranges` 二分反查 chunk；GPU 生成的 schedule 会额外构建一份“按 `outputOffset` 排序后的 schedule 索引表”，depth/view-data 通过这份索引表做间接二分查找，从而在 seeded path 上继续直接消费 schedule domain，而不是再退回 `findChunkIndex()` 粗查路径。仅当这一步排序准备失败时，才回退为消费已经 compact 完成的 `indices_buffer` / `sortedIndices`。进一步地，draw 侧现在也不再默认假设 `view_data.comp` 已经把结果按最终 draw-order 打平成连续数组：在 seeded direct/indirect 模式下，`view_data.comp` 会改为按 compacted schedule domain 写 `GPUViewSplat`，而 `gaussian.vert` 再通过排序后的 `indices_buffer` 间接取回对应的 compacted 记录；只有 full/fallback 路径仍继续沿用 flat draw-order 语义。

### 环境变量

- `GS_RUNTIME_LOG_FILE`：可选。若设置为文件路径，运行时会把关键启动/验收日志追加写入该文件。
- `GS_DEBUG_CHUNKS`：可选。设为 `1` 后，运行时会定期输出 chunk 调试统计，包括 `producer`、`path`、`schedule_lookup`、`draw_domain`、`visible_ratio`、`visible_chunks`、`scheduled_ranges`、`compacted_splats`、`active_splats`、`active_sort_count`、`processed_splats`、`chunk_tests` 与 `chunk_culled_splats`。
- `GS_CHUNK_ENABLE_RATIO`：可选。覆盖 seeded path 的启用阈值，范围 `[0, 1]`。
- `GS_CHUNK_DISABLE_RATIO`：可选。覆盖 seeded path 的退出阈值，范围 `[0, 1]`，且必须大于 `GS_CHUNK_ENABLE_RATIO`。
- `GS_CHUNK_SCHEDULER_MODE`：可选。支持 `auto`、`cpu`、`gpu`、`full`，用于现场比较 GPU compaction、CPU fallback 和 full-domain 行为。
- `GS_CHUNK_FORCE_SEEDED_PATH`：可选。设为 `1` 后，只要当前帧成功生成了合法的 chunk schedule / compacted domain，就强制让 depth/view-data 下游走 seeded schedule-domain 路径，即使可见比例本来会按策略退回 full-domain。主要用于调试和验收，`GS_CHUNK_SCHEDULER_MODE=full` 时不会生效。

说明：即使切到 `cpu` fallback 模式，当前也只是 CPU 生成可见 chunk schedule，再交给 GPU `schedule_compact` 展开，不再回退到每帧上传整段 visible indices。

---

## 固定自测顺序（Step 9）

每次大改后，统一按下面顺序执行：

1. 编译：`cmake --build build --config Release`
2. 生成缓存：`./build/Release/gs_cache_builder.exe ./build/source_ply.txt`
3. 启动运行时：`./build/Release/gaussian_splatting_gl.exe`
4. 人工确认画面：检查资源加载、深度排序、view-data 着色、最终合成
5. 记录结果：把结论、问题、下一步写回 `Contexts.md`

也可以直接执行项目内脚本：

```powershell
./tools/run_step9_smoke.ps1 -SkipVisualCheckPrompt
```

如果要继续 Step 8 的自动化验收，建议执行：

```powershell
./tools/run_step9_smoke.ps1 -RequireAcceptanceMarker -RunMissingPayloadCheck -SkipVisualCheckPrompt
```

如果要同时验证 GPU chunk scheduler 生成的 compaction 域与 CPU 参考结果一致，可执行：

```powershell
./tools/run_step9_smoke.ps1 -ChunkSchedulerMode gpu -RequireAcceptanceMarker -RequireGpuCompactionValidation -ExpectedDrawPath full -ExpectedDrawDomain flat -ExpectedScheduleLookup full -SkipVisualCheckPrompt
```

如果要强制 seeded downstream 并确认 draw 侧继续消费 compacted domain，可执行：

```powershell
./tools/run_step9_smoke.ps1 -ChunkSchedulerMode gpu -ForceSeededPath -RequireAcceptanceMarker -RequireGpuCompactionValidation -ExpectedDrawPath seeded -ExpectedDrawDomain compacted -ExpectedScheduleLookup indirect -SkipVisualCheckPrompt
```

这会额外校验：

- 首帧日志中出现 `ACCEPT: pipeline frame completed`，证明 depth sort、view-data、draw、composite 参考链路都实际执行过
- 首个 draw pass 会输出 `DRAW_SUBMISSION: ...`，可用于核对当前帧是否按预期走 `path=full|seeded` 与 `draw_domain=flat|compacted`
- 缺失 payload 的负例会在启动期 fail-fast，并输出明确的 cache 读取错误
- 若启用 GPU compaction 校验，则日志中必须出现 `GPU_COMPACTION_VALIDATE_OK`，且不能出现 `GPU_COMPACTION_VALIDATE_MISMATCH`

可用参数：

- `-Configuration`：构建配置，默认 `Release`
- `-SourceConfigPath`：PLY 来源配置，默认 `build/source_ply.txt`
- `-OutputCachePath`：可选输出文件名或路径；最终仍会写到原始 PLY 同目录，路径部分会被忽略
- `-RuntimeConfigPath`：运行时模型配置文件，默认 `assets/configs/model_path.txt`
- `-ObserveSeconds`：运行时观察秒数，默认 `5`
- `-RequireAcceptanceMarker`：要求运行时日志出现完整渲染链路通过标记
- `-RequireGpuCompactionValidation`：要求运行时打开 `GS_VALIDATE_GPU_COMPACTION=1` 并校验 CPU/GPU compaction 结果一致
- `-RequireDrawSubmissionMarker`：要求运行时日志出现 `DRAW_SUBMISSION` 标记；启用 `-RequireAcceptanceMarker` 时会自动检查
- `-ExpectedDrawPath`：可选，要求 `DRAW_SUBMISSION` 中的 `path=` 字段匹配指定值（如 `full`、`seeded`）
- `-ExpectedDrawDomain`：可选，要求 `DRAW_SUBMISSION` 中的 `draw_domain=` 字段匹配指定值（如 `flat`、`compacted`）
- `-ExpectedScheduleLookup`：可选，要求 `DRAW_SUBMISSION` 中的 `schedule_lookup=` 字段匹配指定值（如 `full`、`direct`、`indirect`、`fallback`）
- `-ChunkSchedulerMode`：可选，覆盖本次 smoke 的 `GS_CHUNK_SCHEDULER_MODE`；脚本会先清理继承来的终端环境变量，再按参数显式设置
- `-ForceSeededPath`：可选，覆盖本次 smoke 的 `GS_CHUNK_FORCE_SEEDED_PATH=1`
- `-RunMissingPayloadCheck`：额外执行一次缺失 payload 的负例校验
- `-SkipVisualCheckPrompt`：跳过人工确认提示
- `-KeepViewerOpen`：保持查看器窗口打开

如果需要保留窗口做人工观察：

```powershell
./tools/run_step9_smoke.ps1 -KeepViewerOpen
```

然后运行查看器：

### Ninja 生成目录

```powershell
./build/gaussian_splatting_gl.exe
```

### VS 生成目录

```powershell
./build/Release/gaussian_splatting_gl.exe
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

配置值必须是 `.gsplatcache` 路径。支持绝对路径，或相对 `assets/configs/` 的相对路径。

---

