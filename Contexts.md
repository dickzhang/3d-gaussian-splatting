# Contexts

## 保存时间
- Date: 2026-03-25

## 环境信息
- OS: Windows
- Workspace root: `e:/3d-gaussian-splatting`
- 当前编辑文件: `src/main.cpp`

## 工作区结构（来自当前会话上下文）
```text
CMakeLists.txt
README.md
assets/
	configs/
		capture_presets.txt
		model_path.txt
	shaders/
		bitonic_sort.comp
		composite.frag
		composite.vert
		depth_keys.comp
		gaussian.frag
		gaussian.vert
build/
	ALL_BUILD.vcxproj
	ALL_BUILD.vcxproj.filters
	cmake_install.cmake
	CMakeCache.txt
	gaussian_splatting_gl.vcxproj
	gaussian_splatting_gl.vcxproj.filters
	GaussianSplattingGL.sln
	ZERO_CHECK.vcxproj
	ZERO_CHECK.vcxproj.filters
	CMakeFiles/
		cmake.check_cache
		CMakeConfigureLog.yaml
		generate.stamp
		generate.stamp.depend
		generate.stamp.list
		TargetDirectories.txt
		3.27.3/
			CMakeCCompiler.cmake
			CMakeCXXCompiler.cmake
			CMakeRCCompiler.cmake
			CMakeSystem.cmake
			VCTargetsPath.txt
			VCTargetsPath.vcxproj
			CompilerIdC/
			CompilerIdCXX/
			VCTargetsPath/
			x64/
		9f14f5a15bb4a885078fe2db1f19714e/
			generate.stamp.rule
		pkgRedirects/
	gaussian_splatting_gl.dir/
		Release/
			gaussian_splatting_gl.exe.recipe
	Release/
		assets/
			configs/
			shaders/
	x64/
		Release/
			ALL_BUILD/
			ZERO_CHECK/
libs/
	GL3Plus/
		gl3w.cpp
		include/
			GL/
	GLAD/
		include/
			glad/
			KHR/
		src/
			glad_egl.c
			glad_glx.c
			glad_wgl.c
			glad.c
	GLFW/
		include/
			GLFW/
		lib/
	glm/
		glm/
			CMakeLists.txt
			common.hpp
			exponential.hpp
			ext.hpp
			fwd.hpp
			geometric.hpp
			glm.cppm
			glm.hpp
			integer.hpp
			mat2x2.hpp
			mat2x3.hpp
			mat2x4.hpp
			mat3x2.hpp
			mat3x3.hpp
			mat3x4.hpp
			mat4x2.hpp
			mat4x3.hpp
			mat4x4.hpp
			matrix.hpp
			packing.hpp
			...
	imgui/
		imconfig.h
		imgui_demo.cpp
		imgui_draw.cpp
		imgui_internal.h
		imgui_tables.cpp
		imgui_widgets.cpp
		imgui.cpp
		imgui.h
		imstb_rectpack.h
		imstb_textedit.h
		imstb_truetype.h
		LICENSE.txt
		backends/
			...
		docs/
		examples/
		misc/
include/
	gl/
stb/
	stb_image_write.h
src/
	main.cpp
	core/
		ShaderProgram.cpp
		ShaderProgram.h
	io/
		PlyLoader.cpp
		PlyLoader.h
	render/
		GaussianRenderer.cpp
		GaussianRenderer.h
	scene/
		Camera.cpp
		Camera.h
		GaussianModel.h
```

## User Memory（持久记忆）
- `debugging.md`
  - On this Windows setup, rg may be unavailable; use PowerShell Get-ChildItem + Select-String for cross-folder searches.

## Session Memory
- `/memories/session/` is empty.

## Repo Memory
- `/memories/repo/` is empty.

## 附加上下文摘要

### Chat customizations index
- 用户要求先读取并遵守以下规则文件（含 common + 多语言规则）
- 其中与当前项目直接相关的是 C/C++ 规则：
  - `c:/Users/PZ03/.claude/rules/cpp/coding-style.md`
  - `c:/Users/PZ03/.claude/rules/cpp/hooks.md`
  - `c:/Users/PZ03/.claude/rules/cpp/patterns.md`
  - `c:/Users/PZ03/.claude/rules/cpp/security.md`
  - `c:/Users/PZ03/.claude/rules/cpp/testing.md`
- 另外包含 common 规则：
  - `c:/Users/PZ03/.claude/rules/common/agents.md`
  - `c:/Users/PZ03/.claude/rules/common/coding-style.md`
  - `c:/Users/PZ03/.claude/rules/common/development-workflow.md`
  - `c:/Users/PZ03/.claude/rules/common/git-workflow.md`
  - `c:/Users/PZ03/.claude/rules/common/hooks.md`
  - `c:/Users/PZ03/.claude/rules/common/patterns.md`
  - `c:/Users/PZ03/.claude/rules/common/performance.md`
  - `c:/Users/PZ03/.claude/rules/common/security.md`
  - `c:/Users/PZ03/.claude/rules/common/testing.md`

### Skills 列表（会话中可用）
- ai-regression-testing
- android-clean-architecture
- api-design
- backend-patterns
- coding-standards
- compose-multiplatform-patterns
- configure-ecc
- continuous-learning
- continuous-learning-v2
- cpp-coding-standards
- cpp-testing
- django-patterns
- django-tdd
- django-verification
- e2e-testing
- eval-harness
- frontend-patterns
- frontend-slides
- golang-patterns
- golang-testing
- iterative-retrieval
- java-coding-standards
- kotlin-coroutines-flows
- kotlin-exposed-patterns
- kotlin-ktor-patterns
- kotlin-patterns
- kotlin-testing
- laravel-patterns
- laravel-tdd
- laravel-verification
- mcp-server-patterns
- perl-patterns
- perl-testing
- plankton-code-quality
- project-guidelines-example
- python-patterns
- python-testing
- rust-patterns
- rust-testing
- springboot-patterns
- springboot-tdd
- springboot-verification
- strategic-compact
- tdd-workflow
- verification-loop
- agent-customization
- 当前请求未触发特定 skill 执行。

### Agents 列表（会话中可用）
- architect
- build-error-resolver
- chief-of-staff
- code-reviewer
- cpp-build-resolver
- cpp-reviewer
- database-reviewer
- doc-updater
- docs-lookup
- e2e-runner
- go-build-resolver
- go-reviewer
- harness-optimizer
- java-build-resolver
- java-reviewer
- kotlin-build-resolver
- kotlin-reviewer
- loop-operator
- planner
- python-reviewer
- refactor-cleaner
- rust-build-resolver
- rust-reviewer
- security-reviewer
- tdd-guide
- Explore
- 当前请求为文档保存操作，未调用子代理。

## 当前用户请求
- “将上下文都写到项目下的Contexts.md中保存，如果没有这个md就新建一个”

## 执行结果
- 已在项目根目录创建本文件：`Contexts.md`
- 检查结果：此前不存在 `Contexts.md`

## 本次推进（2026-03-26）

### 本次修改
- 在 `src/render/GaussianRenderer.h/.cpp` 中补上了 GPU compaction 的可选 CPU/GPU 对照校验逻辑。
- 新增环境变量：`GS_VALIDATE_GPU_COMPACTION`
	- 置为非 `0` 时，GPU chunk scheduler 成功后会：
		1. 用 CPU 重新按 chunk 可见性生成参考索引域
		2. 回读 GPU 写入的 `indices_buffer` 前缀
		3. 对比 visible chunk 数、schedule entry 数、compacted splat 数、active/sort domain 以及索引序列
	- 若不一致，则输出 `GPU_COMPACTION_VALIDATE_MISMATCH` 日志，并让当前帧退回 CPU/full fallback 路径
	- 若一致，则输出 `GPU_COMPACTION_VALIDATE_OK` 日志（限次）

### 本次验证
- 已执行：`cmake --build build --config Release --target gaussian_splatting_gl`
- 结果：运行时目标编译通过

### 当前判断
- 这一步没有改变默认渲染行为，只是把原来已经预留但未接线的 `m_validateGpuCompaction` 真正接上。
- 价值在于：为后续继续推进纯 GPU compaction / chunk-driven dispatch 提供正确性护栏，先把“GPU 生成的 active domain 是否与 CPU 参考一致”验证清楚。

### 下一步建议
- 优先继续推进：让后续 pass 更直接消费 chunk schedule / compacted domain，逐步减少当前每帧 CPU 侧可见域参与度。
- 具体可选方向：
	1. 增加 chunk schedule buffer 的调试/验证读取工具，确认 GPU producer 输出的 range 契约稳定
	2. 评估 `view_data.comp` 中按 splat 二分查 chunk 的额外成本，考虑改为直接消费 schedule/range 信息，避免重复 chunk 查找
	3. 在 smoke/self-test 中补一条 `GS_VALIDATE_GPU_COMPACTION=1` 的验证路径

## 后续架构决策（2026-03-25）

### 新决策：不再走渐进式迁移，直接切换到 UnityGaussianSplatting 风格架构
- 用户要求：不想一步步来，直接大改现有架构。
- 已新增项目级方案文件：`Plan.md`
- 新方案方向：
	- 离线导入：PLY/SPZ -> Cache Builder
	- 运行时加载：仅加载缓存，不再解析原始 PLY/SPZ
	- GPU 主流程：解码/排序/view-data/绘制/合成
	- Renderer 职责收缩为 GPU 资源与渲染调度，不再承担运行时打包

### 新增文档约束
- 每次会话结束前，必须把本次决策、修改、问题、下一步写入 `Contexts.md`
- 每次压缩上下文之前，必须先把当前上下文摘要写入 `Contexts.md`
- 若架构方向变化，先更新 `Plan.md`，再在 `Contexts.md` 记录变更原因与影响

### 当前计划文件
- `Plan.md`
	- 内容：直接切换到 UnityGaussianSplatting 架构的大改方案
	- 包含：目标架构、模块替换、新增目录、缓存格式、GPU pipeline、CMake 调整、里程碑、风险、验收标准、上下文保存规则

### 当前会话产出
- 产出一份 Big-Bang 架构切换方案并保存到 `Plan.md`
- 将上下文保存规则补充到 `Contexts.md`
- 在 `Plan.md` 中新增“便于改完自测”的大改实施步骤与固定自测顺序

### 下一步建议执行顺序
- 先按 `Plan.md` 落地新的目录骨架与 cache/runtime/render 模块边界
- 再切主程序入口为 cache-first
- 然后实现 builder、reader、runtime asset 与 GPU view pipeline

### 新增执行要求（2026-03-25）
- 用户要求：在方案中补充一套“大改步骤”，便于后续改完后自测
- 已落实：`Plan.md` 新增实施步骤、自测检查点、固定自测顺序

## 大改实施进展（Step 1，2026-03-25）

### 本次目标
- 用户要求“开始按步骤修改”，按 `Plan.md` 的 Step 1（先切数据契约，不切渲染行为）启动改造。

### 已完成改动
1. 提取共享 GPU 布局与打包模块
	- 新增 `src/render/GpuSplatLayout.h`
	- 新增 `src/render/GpuSplatPacking.h`
	- 新增 `src/render/GpuSplatPacking.cpp`
2. Renderer 使用共享模块
	- 修改 `src/render/GaussianRenderer.h`，移除内嵌 `GPUSplat` 定义，改为引用共享布局
	- 修改 `src/render/GaussianRenderer.cpp`，移除本地 half 打包实现，改为调用 `pack_gpu_splats` 等工具函数
3. 创建大改目录骨架（Step 1 要求）
	- `src/import/`：`GaussianScene.h`、`PlySceneImporter.h/.cpp`
	- `src/cache/`：`SplatCacheFormat.h`、`SplatCacheWriter.h/.cpp`、`SplatCacheReader.h/.cpp`
	- `src/runtime/`：`RuntimeSplatAsset.h`、`RuntimeSplatAssetLoader.h/.cpp`
	- `tools/gs_cache_builder/`：`main.cpp`
4. CMake 接入
	- `gaussian_splatting_gl` 新增 `PlySceneImporter.cpp`、`GpuSplatPacking.cpp`
	- 新增目标 `gs_cache_builder`

### 本次自测结果
1. 构建自测
	- `gaussian_splatting_gl` Release 构建通过
	- `gs_cache_builder` Release 构建通过
2. CLI 烟测
	- 以 `assets/configs/model_path.txt` 作为输入运行 `gs_cache_builder` 时失败（预期：该文件不是 PLY），报错正确：`Not a PLY file` / `Failed to parse PLY header`
3. 代码审查反馈
	- 已修复审查指出的关键问题：`SplatCacheWriter/Reader` 不再恒 false，改为最小可用读写实现
	- 已补回 `GaussianRenderer.cpp` 对 `<cmath>` 的直接依赖

### 当前状态判断
- Step 1 核心目标已达成：共享数据契约已抽离、目录骨架已落地、主渲染行为未被替换。
- Step 2 可开始：完善 Builder 输入路径策略，并补齐 cache section 的分段写入（当前为最小 packed-splat 单段实现）。

## 计划状态标记更新（2026-03-25）
- 用户要求：在 `Plan.md` 中把完成状态做上标记。
- 已落实：
	- 在里程碑区新增状态图例与“当前里程碑状态”。
	- 在 Step 1~9 前新增“状态”字段。
	- 当前标记为：
		- Step 1：✅ 已完成
		- Step 2：⏳ 进行中
		- Step 3~9：⬜ 未开始（Step 9 为流程定义待执行）

## 继续推进记录（2026-03-25，Step 2/3）

### 本次核心改动
1. 运行时切换为 cache-first：
	- `src/main.cpp` 移除主流程 `PlyLoader` 依赖。
	- 新增 `.gsplatcache` 强校验与明确报错信息。
	- 通过 `RuntimeSplatAssetLoader` 加载缓存并调用 `GaussianRenderer::uploadAsset` 上传。
2. Renderer 增加 runtime 资产上传路径：
	- `src/render/GaussianRenderer.h/.cpp` 新增 `uploadAsset` 与内部 `uploadPackedSplats`。
	- 旧 `uploadModel` 保留为兼容适配层（转调 shared packing + upload）。
3. Builder 健壮性增强：
	- `tools/gs_cache_builder/main.cpp` 新增 txt 输入的 trim/BOM 处理、注释行跳过、相对路径解析与输入扩展名校验。
	- 解决 UTF-8 BOM 配置导致路径读取失败问题。
4. 构建与文档：
	- `CMakeLists.txt` 为 `gaussian_splatting_gl` 接入 `SplatCacheReader.cpp` 与 `RuntimeSplatAssetLoader.cpp`。
	- `README.md` 更新为“先生成缓存再运行”的 cache-first 说明。
	- `assets/configs/model_path.txt` 更新为 `../../build/test.gsplatcache` 以匹配新主流程。

### 本次验证结果
1. 构建：
	- `cmake --build build --config Release` 通过（`gaussian_splatting_gl`、`gs_cache_builder`）。
	- 备注：`Build_CMakeTools` 在当前环境返回 configure 失败，改用终端构建完成验证。
2. Builder 烟测：
	- 命令：`./build/Release/gs_cache_builder.exe ./assets/configs/model_path.txt ./build/test.gsplatcache`
	- 结果：成功生成缓存。
3. 稳定性自测（Step 2 要求）：
	- 同一输入生成 `test_a.gsplatcache` 与 `test_b.gsplatcache`，SHA256 完全一致。
4. 运行时报错路径自测（Step 3 要求）：
	- 当配置为 `.ply` 时，程序明确提示必须使用 `.gsplatcache`，且提供 builder 命令。

### 状态更新
- `Plan.md` 已更新：
	- M1：✅ 已完成
	- M2：⏳ 进行中
	- Step 2：✅ 已完成
	- Step 3：✅ 已完成

### 下一步
- 进入 Step 4：拆分 `GpuUploadBuffers`，继续收敛 `GaussianRenderer` 职责，只保留渲染调度。

## Step 4 实施记录（2026-03-25）

### 本次核心改动
1. 新增上传模块：
	- `src/render/GpuUploadBuffers.h`
	- `src/render/GpuUploadBuffers.cpp`
	- 负责 SSBO 创建、释放、packed splat 上传、排序缓冲初始化与统计信息输出。
2. `GaussianRenderer` 职责收敛：
	- `GaussianRenderer` 中删除上传细节与 buffer 创建逻辑。
	- 通过 `GpuUploadBuffers::create/destroy/upload_packed_splats` 复用上传能力。
	- 新增 `m_initialized` 防御状态，避免初始化失败后误进入渲染。
3. 构建接入：
	- `CMakeLists.txt` 为运行时目标新增 `src/render/GpuUploadBuffers.cpp`。

### 本次验证结果
1. 构建验证：
	- `cmake --build build --config Release` 通过。
	- `gaussian_splatting_gl` 与 `gs_cache_builder` 均成功生成。
2. 代码审查：
	- 执行 `cpp-reviewer` 审查，唯一高优先级项为初始化守卫，已修复并重编译通过。

### 状态更新
- `Plan.md` 已更新：
	- M2：✅ 已完成
	- Step 4：✅ 已完成

### 下一步
- 进入 Step 5：实现 `view_data.comp` 与 `ViewDataPipeline`，将 view-dependent 计算从 draw shader 前移到 compute。

## Step 5 实施记录（2026-03-25）

### 本次核心改动
1. 新增 view-data 计算通道：
	- `assets/shaders/view_data.comp`
	- `src/render/ViewDataPipeline.h`
	- `src/render/ViewDataPipeline.cpp`
	- 每帧新增 pass：`distance -> sort -> view_data -> draw -> composite`
2. 渲染数据契约扩展：
	- `src/render/GpuSplatLayout.h` 新增 `GPUViewSplat`，作为 compute->vertex 的共享布局。
3. 上传模块扩展：
	- `src/render/GpuUploadBuffers.h/.cpp` 新增 `view_data_buffer` 生命周期与容量分配。
4. Renderer 接线：
	- `src/render/GaussianRenderer.h/.cpp` 接入 `ViewDataPipeline`，新增 `runViewDataPass`。
	- draw pass 改为消费 binding=2 的 view-data SSBO。
	- 增加 view pass 失败日志并避免提前返回导致状态泄漏。
	- 保存并恢复调用者 SSBO 绑定状态（0/1/2）。
5. Shader 调整：
	- `assets/shaders/gaussian.vert` 从“实时重计算”改为“读取 view-data buffer 并做 quad 展开”。
6. 构建接入：
	- `CMakeLists.txt` 新增 `src/render/ViewDataPipeline.cpp`。

### 本次验证结果
1. 编译：
	- `cmake --build build --config Release` 多轮通过。
	- `gaussian_splatting_gl` 与 `gs_cache_builder` 均成功生成。
2. 运行时烟测：
	- 启动日志可正常到达模型上传与运行参数打印，未出现 shader 编译失败提示。
3. 审查：
	- 执行 `code-reviewer` + `cpp-reviewer`。
	- 关键修复已完成：SSBO 状态恢复、上传后 generic SSBO 解绑定、view pass 失败路径日志与状态安全。

### 状态更新
- `Plan.md` 已更新：
	- Step 5：✅ 已完成

### 下一步
- 进入 Step 6：从单段 packed 缓存过渡到 Unity 风格多 section 缓存（pos/other/color/sh/chunk）。

## 用户要求补充（2026-03-25）

### 需求
- 用户要求：继续下一步时，缓存文件默认生成在原始 PLY 资源文件目录下。

### 已落实改动
1. `tools/gs_cache_builder/main.cpp`
	- 命令参数改为：`gs_cache_builder <input.ply|input.txt> [output.gsplatcache|output_directory]`
	- 当省略输出参数时，默认输出到源 PLY 同目录，命名 `<input_stem>.gsplatcache`。
	- 支持输出参数为目录（自动拼接 `<input_stem>.gsplatcache`）。
	- 增强健壮性：输入文件存在性检查、输出目录创建错误细化、覆盖已有缓存时告警。
	- 对非 `.gsplatcache` 输出扩展名做规范化（自动改为 `.gsplatcache` 并提示）。
2. `README.md`
	- 文档更新为默认输出到源 PLY 目录，并说明可显式传输出文件或目录。
3. `src/main.cpp`
	- 运行时报错提示改为与新 builder 用法一致（默认输出行为说明）。

### 验证结果
1. 构建：`cmake --build build --config Release` 通过。
2. 烟测：使用 `build/source_ply.txt` 作为输入且省略输出参数，成功在源 PLY 目录生成：
	- `.../iteration_30000/point_cloud.gsplatcache`
	- 若文件已存在，会输出覆盖告警。

## Step 6 进入记录（2026-03-25）

### 本次核心改动
1. 缓存格式升级为 v2 多 section：
	- 修改 `src/cache/SplatCacheFormat.h`
	- `kSplatCacheVersion` 升级为 `2`
	- 新增分段 payload 结构：`SplatCachePositionEntry`、`SplatCacheOtherEntry`、`SplatCacheColorEntry`、`SplatCacheShEntry`、`SplatCacheChunkEntry`
	- 增加二进制布局 `static_assert`，固定结构尺寸。
2. Builder 输出多 section 缓存：
	- 修改 `src/cache/SplatCacheWriter.cpp`
	- 输出 section 顺序：`chunk -> position -> other -> color -> sh`
	- 继续通过现有 `GPUSplat` 打包逻辑生成分段数据，确保与当前渲染路径一致。
3. Reader 分段读取与兼容：
	- 修改 `src/cache/SplatCacheReader.cpp`
	- 支持 v2 分段读取并重组为 `RuntimeSplatAsset::packed_splats`
	- 保留对旧版 `PackedSplat` section 的兼容读取（v1/v2 fallback）。
4. 文档同步：
	- 修改 `README.md`，声明当前缓存为 v2 分段布局并说明兼容旧 packed 缓存读取。

### 稳定性与安全性修复（审查反馈后）
1. 修复 size 计算溢出风险（乘法前置 64 位校验）。
2. 修复 `std::streamsize` 下转风险（写入/读取字节数上限校验）。
3. 增加 section offset 累加溢出校验。
4. 增加 `tellg()` 负值校验与 bounds 合法性校验。
5. 增加 chunk 区间相加溢出与越界校验。

### 本次验证结果
1. 编译验证：`cmake --build build --config Release` 通过（`gaussian_splatting_gl` 与 `gs_cache_builder`）。
2. 审查验证：
	- 已执行 `code-reviewer` 与 `cpp-reviewer`。
	- 最终 `cpp-reviewer` 结果：无 HIGH/CRITICAL 问题。

### 当前状态
- Step 6 已进入并处于进行中：Builder/Reader 的多 section 已落地。
- 尚未完成项：运行时 GPU 资源从“重组 packed 上传”切换为“按 section 直传 + shader 按分段解码”。

## Step 6/7 收尾记录（2026-03-26）

### 本次核心改动
1. 完成 Step 6 的运行时多 section 直通：
	- `src/runtime/RuntimeSplatAsset.h` 新增 `positions/others/colors/sh_data/chunks` 分段载荷。
	- `src/cache/SplatCacheReader.cpp` 读取 v2 缓存时不再强制重组回 `packed_splats`，而是保留分段数据；旧 packed 缓存继续 fallback 兼容。
	- `src/render/GpuUploadBuffers.h/.cpp` 新增 `position/other/color/sh/chunk` GPU buffer，并支持 split-section 上传。
	- `src/render/ViewDataPipeline.h/.cpp` 新增 `input_layout`，按 packed/split 两种输入绑定不同 SSBO。
	- `assets/shaders/depth_keys.comp` 与 `assets/shaders/view_data.comp` 已支持按 `u_inputLayout` 解码 packed 或 split 数据。
2. 完成 Step 7 的运行时旧路径清理：
	- `src/render/GaussianRenderer.h/.cpp` 删除旧 `uploadModel(const GaussianModel&)` 入口与对应 renderer 侧 pack 兼容逻辑。
	- `CMakeLists.txt` 从 `gaussian_splatting_gl` 目标中移除 `src/import/PlySceneImporter.cpp`、`src/io/PlyLoader.cpp`、`src/render/GpuSplatPacking.cpp`。
	- 运行时可执行现在仅保留 cache-first 路径；`PlyLoader/PlySceneImporter/GpuSplatPacking` 仅保留给 builder 侧使用。
3. 运行时残余依赖修复：
	- `src/render/GpuUploadBuffers.cpp` 内聚排序初始化 helper，避免运行时通过链接继续依赖旧 `GpuSplatPacking.cpp`。
4. shader 编译与审查修复：
	- 修复 `view_data.comp` 中 split SH buffer 字段命名导致的 GLSL 编译错误。
	- 修复 split 上传的计数一致性与 `GLsizeiptr` 溢出校验。
	- 修复 packed/split 模式下的条件 SSBO 绑定与 GL 错误日志。

### 本次验证结果
1. 编译：
	- `cmake --build build --config Release` 多轮通过。
	- `gaussian_splatting_gl` 与 `gs_cache_builder` 均成功生成。
2. 运行时烟测：
	- `./build/Release/gaussian_splatting_gl.exe` 可启动。
	- 启动过程中曾暴露 `view_data.comp` 的 GLSL 编译问题，已修复后成功启动。
	- 当前终端仅见 `libpng iCCP` 警告，无新的启动错误。
3. 审查：
	- 已执行 `planner`、`cpp-reviewer`、`code-reviewer`。
	- 最终 review 结果：无 CRITICAL/HIGH 问题。

### 状态更新
- `Plan.md` 已更新：
	- Step 6：✅ 已完成
	- Step 7：✅ 已完成

### 下一步
- 进入 Step 8：做总体验收清单，重点验证 split 缓存路径的实际出图、排序链路与合成链路。

## Step 8 验收启动记录（2026-03-26）

### 本次执行内容
1. Step 8 输入确认：
	- 运行时配置文件 `assets/configs/model_path.txt` 当前指向 `../../build/test.gsplatcache`。
	- builder 输入使用 `build/source_ply.txt`，对应 garden 场景的原始 PLY。
2. 构建验收：
	- `cmake --build build --config Release` 通过。
	- 备注：`Build_CMakeTools` 在当前环境仍然 configure 失败，继续使用既有 `build` 目录完成验收。
3. builder 验收：
	- 执行 `./build/Release/gs_cache_builder.exe ./build/source_ply.txt ./build/test.gsplatcache`
	- 成功重新生成缓存，日志：
		- `Loaded PLY splats: 5834784`
		- `Detected SH payload: degree-3`
		- `Cache written: ./build/test.gsplatcache`
4. runtime 烟测：
	- 启动 `./build/Release/gaussian_splatting_gl.exe`
	- 启动日志确认：
		- `Uploaded split-section asset: 5834784 splats to GPU (sort count: 8388608)`
		- `Loaded cache: build/test.gsplatcache`
		- `Model SH max degree: 3`
	- 未出现新的 shader 编译失败、缓存读取失败或 OpenGL 初始化错误。

### 当前结论
- Step 8 已开始并完成自动化烟测部分：
	1. 构建通过
	2. builder 重新生成缓存通过
	3. runtime 可加载最新缓存并启动 split-section 路径
- 尚待人工确认项：画面层面的深度排序、view-data、合成链路是否符合预期。

### 下一步
- 继续 Step 8 的人工视觉验收；若确认无误，可将 Step 8 标记为完成并进入 Step 9 固化自测流程。

## Step 9 固化记录（2026-03-26）

### 本次核心改动
1. 新增固定自测脚本：
	- `tools/run_step9_smoke.ps1`
	- 固定顺序为：编译 -> 生成缓存 -> 启动运行时 -> 输出视觉检查项 -> 记录结果
	- 支持 `-SkipVisualCheckPrompt` 便于自动化烟测
	- 支持 `-KeepViewerOpen` 便于人工观察窗口画面
2. 文档同步：
	- `README.md` 新增“固定自测顺序（Step 9）”章节
	- `Plan.md` 将 Step 9 标记为完成

### 预期使用方式
1. 自动烟测：`./tools/run_step9_smoke.ps1 -SkipVisualCheckPrompt`
2. 人工观察：`./tools/run_step9_smoke.ps1 -KeepViewerOpen`

### 本次验证结果
1. 首次实跑暴露脚本细节问题，已修复：
	- 严格模式下重定向输出的数组长度判断问题
	- runtime 进程异常时的清理保障
	- builder/runtime 可执行文件存在性检查
	- README 参数文档补齐
	- 运行前临时把 `assets/configs/model_path.txt` 切到新生成缓存，结束后恢复原值
2. 修复后执行：
	- `./tools/run_step9_smoke.ps1 -SkipVisualCheckPrompt`
	- 脚本已按顺序完成：构建 -> 生成缓存 -> 启动运行时 -> 输出视觉检查项
3. 配置恢复验证：
	- `assets/configs/model_path.txt` 已恢复为执行前的原始值。
4. 审查结果：
	- 最终 `code-reviewer` 结果：无 CRITICAL/HIGH/MEDIUM 问题。

### 下一步
- 继续 Step 8 的人工视觉验收；若画面确认无误，可把 Step 8 也标记为完成。

## 缓存输出策略调整（2026-03-26）

### 用户要求
- 生成的缓存文件应与原始 PLY 资源文件位于同一个文件夹。

### 已落实改动
1. `tools/run_step9_smoke.ps1`
	- `-OutputCachePath` 默认改为空。
	- 当未显式指定输出路径时，脚本会解析 `SourceConfigPath` 对应的 PLY 路径，并默认输出到该 PLY 同目录下的 `<stem>.gsplatcache`。
2. `README.md`
	- Step 9 示例命令改为省略输出路径，明确默认输出到源 PLY 同目录。
3. `assets/configs/model_path.txt`
	- 运行时默认模型路径改为同目录下的 `point_cloud.gsplatcache`，不再指向 `.ply`。

### 验证结果
1. 执行：`./build/Release/gs_cache_builder.exe ./build/source_ply.txt`
2. 输出：
	- `Cache written: I:/引擎工作组/模型/3DGS高斯泼溅/models/garden/point_cloud/iteration_30000/point_cloud.gsplatcache`
3. 结论：
	- builder 默认输出路径已符合“缓存与 PLY 同目录”的要求。

## Step 6 方案调整记录（2026-03-26）

### 用户要求
- Step 6 中的 `pos/other/color/sh/chunk` 数据组织方式，参考 Unity 插件改为生成单独的缓存文件，而不是继续以单文件多 section 作为最终目标。

### 架构决策
1. Step 6 的目标从“单个 `.gsplatcache` 内承载多 section”调整为“`.gsplatcache` 作为 manifest 入口，`pos/other/color/sh/chunk` 作为独立 payload 文件”。
2. 运行时配置仍保持指向 `.gsplatcache`，但该文件只负责描述元信息和 payload 文件表，不再承担全部二进制载荷。
3. 推荐命名保持统一前缀：
	- `<stem>.gsplatcache`
	- `<stem>.pos.byte`
	- `<stem>.other.byte`
	- `<stem>.color.byte`
	- `<stem>.sh.byte`
	- `<stem>.chunk.byte`（可选）
4. 当前代码中已落地的“单文件 v2 多 section”实现转为过渡兼容层：Reader 保留读取兼容，Builder 后续默认改为写 manifest + 多 payload。

### 影响说明
1. `Plan.md` 已同步调整：
	- 第 4 节缓存格式改为 manifest + payload 文件表
	- Step 6 从“已完成”回调为“进行中”
	- Step 6 目标、自测与兼容策略按新方案重写
2. Step 7 的运行时 cache-first 与离线路径剥离结论仍然成立，但后续需要在新分文件缓存落地后重新跑一次验收。
3. Step 8 的验收项后续需要补充 manifest/payload 一致性与缺失文件报错检查。

### 当前状态
1. 这是架构方向调整，当前会话仅更新计划与上下文，尚未开始把 Builder/Reader 从“单文件多 section”实现切换到“manifest + 多 payload”实现。
2. 现有单文件 v2 资产与流程暂时仍可继续使用，作为新格式切换前的兼容和回归基线。

## Step 6 完成记录（2026-03-26）

### 本次核心改动
1. 完成 manifest + payload 分文件缓存闭环：
	- `src/cache/SplatCacheWriter.h/.cpp` 增加 `SplatCacheWriteOptions`，builder 现在默认写 `manifest + pos/other/color/sh`，并可按选项决定是否写 `chunk`。
	- `tools/gs_cache_builder/main.cpp` 支持 `--chunk` / `--no-chunk`，便于覆盖 Step 6 的两种产物场景。
2. 收紧运行时契约并清理旧 runtime fallback：
	- `src/runtime/RuntimeSplatAsset.h` 删除 `packed_splats`，运行时资产只保留 split sections。
	- `src/render/GaussianRenderer.h/.cpp` 删除 runtime packed 上传 fallback，`uploadAsset` 只接受 split-section 资产。
	- `src/render/GpuUploadBuffers.h/.cpp` 删除 `upload_packed_splats` 运行时入口，保留 split 上传路径。
3. 增强 manifest 读取校验与错误信息：
	- `src/cache/SplatCacheReader.h/.cpp` 与 `src/runtime/RuntimeSplatAssetLoader.h/.cpp` 新增 `out_error` 传递链路。
	- 增加 manifest magic/version、重复 payload、chunk flag/table 不一致、相对路径非法、payload 缺失、大小不符、checksum 错误、bounds 非法等明确报错。
	- 路径校验改为基于目录约束的 fail-fast 检查，运行时启动时能直接打印具体失败原因。
4. 文档同步：
	- `README.md` 更新为 v3 manifest + payload 描述，并声明 runtime 不再兼容旧 single-file / packed cache。

### 本次验证结果
1. 构建验证：
	- `Build_CMakeTools` 仍因当前环境 configure 失败，继续使用既有 `build` 目录验证。
	- `cmake --build build --config Release` 通过，`gaussian_splatting_gl` 与 `gs_cache_builder` 均成功生成。
2. Builder 烟测：
	- 生成带 chunk 产物：`build/step6/with_chunk.gsplatcache` + `with_chunk.pos/other/color/sh/chunk.byte`。
	- 生成无 chunk 产物：`build/step6/no_chunk.gsplatcache` + `no_chunk.pos/other/color/sh.byte`，未生成 `no_chunk.chunk.byte`。
3. Runtime 烟测：
	- 使用 `../../build/step6/no_chunk.gsplatcache` 启动 runtime，日志确认：
		- `Uploaded split-section asset: 5834784 splats to GPU (sort count: 8388608)`
		- `Loaded cache: build\step6\no_chunk.gsplatcache`
		- `Cache splats: 5834784`
	- 结论：无 chunk 的 manifest/payload 组合可被运行时正常读取、上传并进入渲染主循环。
4. 审查结果：
	- 已执行 `code-reviewer` 与 `cpp-reviewer`。
	- `cpp-reviewer` 未发现 CRITICAL/HIGH 问题。
	- `code-reviewer` 输出以误报和非阻断性建议为主，本轮未发现需要继续修正的实际阻断问题。

### 状态更新
1. `Plan.md` 已更新：
	- Step 6：✅ 已完成
	- Step 8：仍为 ⏳ 进行中

### 下一步
1. 进入 Step 8 剩余验收：
	- 做人工视觉确认，重点检查排序稳定性、view-data 着色与最终合成是否符合预期。
2. 若需要，再补一个独立 cache validator / 负例测试入口，把缺失 payload、checksum 错误等场景自动化掉。

## 缓存输出目录约束更新（2026-03-26）

### 用户要求
- 缓存文件都生成在 `.ply` 同目录下。

### 已落实改动
1. `tools/gs_cache_builder/main.cpp`
	- 输出目录固定到源 `.ply` 所在目录。
	- 第二个参数如果提供，只再控制 manifest 文件名；其目录部分会被忽略。
	- `pos/other/color/sh/chunk` payload 与 `.gsplatcache` manifest 始终共同写入 `.ply` 同目录。
2. `tools/run_step9_smoke.ps1`
	- `-OutputCachePath` 现在只用于提供输出文件名；若传入其他目录，脚本会忽略目录部分，并仍指向 `.ply` 同目录中的实际产物。
3. `README.md`
	- 文档已同步为“所有缓存文件写入 `.ply` 同目录”的唯一行为。
4. `Plan.md`
	- 已同步更新 Step 6 动作与 builder 命令说明，避免继续出现“可指定输出目录”的旧描述。

## Step 8 验收推进记录（2026-03-26）

### 本次核心改动
1. `src/render/GaussianRenderer.h/.cpp`
	- 新增一次性 acceptance 日志：当首帧完整执行 depth sort、view-data、draw、composite 参考链路后，输出 `ACCEPT: pipeline frame completed (...)`。
	- 若 accumulation/composite 参考链路不可用，会输出 `ACCEPT_WARN: composite reference path unavailable ...`，避免静默 fallback。
2. `src/main.cpp`
	- 把关键 runtime 启动日志和 cache 加载错误同步写入可选的 `GS_RUNTIME_LOG_FILE`，便于脚本化验收和负例排查。
3. `tools/run_step9_smoke.ps1`
	- 增加 `-RequireAcceptanceMarker` 与 `-RunMissingPayloadCheck` 两个入口。
	- 默认观察窗口从 `5` 秒提升到 `10` 秒，降低大模型/冷启动情况下的误判概率。
	- 缺失 payload 的负例会校验 runtime 是否输出明确的 cache 读取错误。
4. `README.md`
	- 补充 Step 8 自动化验收命令示例，以及 acceptance marker / missing-payload 负例的用途说明。

### 本次验证结果
1. 构建验证：
	- `cmake --build build --config Release` 通过。
2. 代码审查：
	- 已执行 `cpp-reviewer` 与 `code-reviewer`。
	- 已按 review 修正环境变量读取的异常安全/跨平台分支，以及 smoke 脚本默认观察窗口过短的问题。
3. 当前限制：
	- 在当前 Copilot harness 下，GUI runtime 通过脚本进程包装时行为不稳定，尚未在本会话内完成“acceptance marker 自动判定成功”的最终闭环。
	- 因此 Step 8 仍保持 `⏳ 进行中`，剩余项仍是人工视觉验收，以及在真实本地终端环境里确认 acceptance marker / missing-payload 检查链路。

### 下一步
1. 在本地终端直接执行：`./tools/run_step9_smoke.ps1 -RequireAcceptanceMarker -RunMissingPayloadCheck -KeepViewerOpen`
2. 人工确认：
	- 模型正常出图
	- 相机移动时排序稳定
	- view-data 着色与最终 composite 正常
	- 日志中出现 acceptance marker，且 missing-payload 负例输出明确报错
3. 若以上确认通过，再将 Step 8 标记为完成。

## 运行时崩溃修复记录（2026-03-26）

### 问题现象
1. 用户反馈项目运行时直接崩溃。
2. 前台复现 `./build/Release/gaussian_splatting_gl.exe`，退出码为 `-1073740791`。

### 根因定位
1. `assets/configs/model_path.txt` 当前保存的是带中文目录的 Windows 本机路径，其字节内容不是 UTF-8，而是本机代码页文本。
2. 运行时与 builder 最近在多个边界点使用了 `std::filesystem::u8path(...)`：
	- `src/main.cpp`
	- `src/render/GaussianRenderer.cpp`
	- `tools/gs_cache_builder/main.cpp`
3. 在 Windows 下把非 UTF-8 窄字符串强行按 UTF-8 解码，导致运行时在启动阶段崩溃。

### 已落实修复
1. 新增“容错路径解析”策略：
	- Windows 下优先尝试 `std::filesystem::u8path(...)`
	- 若抛出 `std::filesystem::filesystem_error`，回退到 `std::filesystem::path(...)`
2. 修复范围：
	- `src/main.cpp`
	- `src/render/GaussianRenderer.cpp`
	- `tools/gs_cache_builder/main.cpp`
3. 这样既兼容 UTF-8 文本路径，也兼容当前 `model_path.txt` 里的本机代码页中文路径。

### 验证结果
1. 启动日志 `build/runtime-startup.log` 已成功写出：
	- `Uploaded split-section asset: 5834784 splats to GPU (sort count: 8388608)`
	- `Loaded cache: ...point_cloud.gsplatcache`
	- `ACCEPT: pipeline frame completed (depth_sort=1 view_data=1 draw=1 composite=1 reference_path=1)`
2. 结论：本次运行时“启动即崩溃”问题已修复，程序已至少成功完成一帧完整渲染链路。

### 备注
1. `get_errors` 中 `main.cpp` 的报错仍然是当前 VS Code IntelliSense 缺少系统头配置，与本次运行时崩溃修复无关。

## Chunk 链路检查记录（2026-03-26）

### 检查目标
1. 确认 builder 是否正常生成 chunk payload。
2. 确认 reader/runtime 是否正常读取并上传 chunk。
3. 确认当前渲染链路是否真的使用了 chunk 数据。

### 检查结果
1. 生成链路正常：
	- 使用 `./build/Release/gs_cache_builder.exe ./build/source_ply.txt` 重新生成 garden 场景缓存后，源 PLY 同目录下存在 `point_cloud.chunk.byte`。
	- 该文件大小为 `8` 字节，对应一个 `SplatCacheChunkEntry`。
	- 解析结果为：`start_index = 0`，`splat_count = 5834784`。
	- manifest 头部信息为：`payload_count = 5`、`flags = 0x00000001`，与“带 chunk”状态一致。
2. 读取链路正常：
	- `SplatCacheReader` 会校验 manifest 的 chunk flag 与 payload 表一致性，并在存在 chunk entry 时读取 `.chunk.byte` 到 `RuntimeSplatAsset::chunks`。
	- `GpuUploadBuffers` 会把 `RuntimeSplatAsset::chunks` 上传到 `chunk_buffer`。
3. 应用链路目前未真正使用：
	- 当前 `ViewDataPipeline` 只绑定 `indices/view_data` 和 `position/other/color/sh`，没有绑定 `chunk_buffer`。
	- 当前 shader 侧未声明任何 chunk SSBO，也没有按 chunk 做剔除、分桶、分批或索引重映射。
	- 实测运行时分别加载 `point_cloud.gsplatcache` 与 `no_chunk.gsplatcache`，两者都输出相同的 acceptance 日志，说明当前画面主链路不依赖 chunk。

### 当前结论
1. chunk 数据现在是“正常生成、正常读取、正常上传，但未参与实际渲染/调度逻辑”。
2. 现有 builder 生成的 chunk 还是最小占位实现：仅 1 个 chunk 覆盖全部 splats，而不是真正的空间分块或批次分块。

### 后续建议
1. 如果 Step 8/Step 9 只要求当前 split cache 主链路跑通，则 chunk 现状不构成阻塞。
2. 如果目标是接近 UnityGaussianSplatting 的 chunk-driven 路径，则后续至少需要补齐：
	- builder 侧真实 chunk 划分策略
	- runtime/shader 侧 chunk buffer 绑定与消费
	- 基于 chunk 的剔除、排序域缩减或分批调度

## Chunk 真正接入记录（2026-03-26）

### 本次核心改动
1. chunk 格式从“8 字节占位区间”升级为“连续 splat 区间 + 包围球”的 v4 布局：
	- `src/cache/SplatCacheFormat.h` 将 `kSplatCacheVersion` 升级为 `4`
	- `SplatCacheChunkEntry` 扩展为 `32` 字节，包含 `center/radius/start_index/splat_count`
2. builder 开始生成真实多 chunk payload：
	- `src/cache/SplatCacheWriter.h/.cpp` 新增 `chunk_size` 配置，默认 `2048`
	- 同一场景不再只写一个全量 chunk，而是按连续区间拆分并为每块计算包围球
	- garden 场景实测生成 `2850` 个 chunk，`point_cloud.chunk.byte` 大小为 `91200` 字节
3. reader 增强：
	- `src/cache/SplatCacheReader.cpp` 兼容读取旧 v3 chunk 载荷，并为旧格式合成包围球
	- 增加 chunk 连续覆盖、非空、半径非负等校验，避免 shader 二分查找建立在坏数据之上
4. runtime 真正消费 chunk：
	- `src/render/ViewDataPipeline.h/.cpp` 新增 `u_chunkCount` 与 binding `7` 的 `chunk_buffer` 绑定
	- `assets/shaders/view_data.comp` 新增对 chunk buffer 的二分查找与基于包围球的粗粒度视锥剔除
	- `src/render/GaussianRenderer.cpp` 保存/恢复新增 SSBO 绑定，并在上传日志中输出 chunk 数量
5. 运行时可观测性增强：
	- `src/main.cpp` 启动日志新增 `Cache chunks: ...`
	- `README.md` 已同步为 v4 chunk 说明，并记录默认 chunk 大小与实际 culling 用途

### 本次验证结果
1. 构建：
	- `cmake --build build --config Release` 多轮通过
2. builder：
	- `./build/Release/gs_cache_builder.exe ./build/source_ply.txt` 成功生成 v4 cache
	- 产物检查确认：`ChunkCount = 2850`
3. 兼容性：
	- `./build/Release/gs_cache_builder.exe --no-chunk ./build/source_ply.txt no_chunk_v4.gsplatcache` 成功生成无 chunk 变体
4. review：
	- 已执行 `cpp-reviewer` 与 `code-reviewer`
	- 最终无 CRITICAL/HIGH correctness blocker；仅剩测试覆盖与性能观测层面的后续项

### 当前结论
1. chunk 不再只是“生成/上传但未使用”的占位数据。
2. 当前运行时已在 `view_data.comp` 中真实消费 chunk 元数据，用于粗粒度视锥剔除。
3. 这是一条最小但真实生效的 chunk 路径；后续若继续对齐 UnityGaussianSplatting，可再引入更强的 chunk-driven 调度或排序域缩减。

## Chunk 可观测性补充（2026-03-26）

### 本次核心改动
1. 为 `view_data.comp` 增加 GPU 调试统计缓冲：
	- 统计项包括 `processed_splats`、`chunk_tests`、`chunk_culled_splats`
	- 通过 SSBO + `atomicAdd` 在 compute pass 内累计
2. `ViewDataPipeline` 支持在 dispatch 前清零 stats buffer，并在需要时回读统计结果。
3. `GaussianRenderer` 新增 `GS_DEBUG_CHUNKS` 调试开关：
	- 开启后按固定间隔输出 `CHUNK_DEBUG: ...`
	- 为避免刷屏，日志间隔固定且总输出次数做了上限限制
	- 若回读到 `chunk_culled_splats > chunk_tests` 的异常统计，会输出 `CHUNK_DEBUG_WARN`
4. `README.md` 已补充 `GS_DEBUG_CHUNKS` 环境变量说明。

### 验证情况
1. 构建验证：
	- `cmake --build build --config Release` 通过
2. 运行时验证限制：
	- 当前 Copilot harness 下，GUI 进程的 stdout / 日志回写仍不稳定
	- 因此本会话确认了代码路径、构建结果和文档落盘，但未在 harness 内稳定拿到 `CHUNK_DEBUG:` 运行日志
	- 后续应在本地终端直接以 `GS_DEBUG_CHUNKS=1` 启动查看器验证统计输出

## Chunk 排序域缩减接入记录（2026-03-26）

### 本次核心改动
1. `src/render/GaussianRenderer.h/.cpp`
	- 新增 total/active splat 计数与 sort capacity 区分，避免把缓冲容量和当前帧工作域混用。
	- 新增 `prepareVisibleSplatDomain(...)`：每帧按 chunk 包围球做 CPU 可见性测试，把可见 chunk 的连续 splat 区间展开成可见索引列表，并写回 `indices_buffer` 前缀。
	- 当存在 chunk 数据时，depth/sort/view-data/draw 现在只处理 `active_splats`，不再固定覆盖全量 splats。
	- 新增 upload 阶段的 chunk range/radius 校验，以及 `uint32` 可寻址 splat 上限保护。
2. `assets/shaders/depth_keys.comp`
	- 新增 `u_useSeedIndices`，允许 depth pass 在启用时读取 CPU 预写入的可见 splat 索引，而不是固定使用 `id`。
	- 这样 bitonic sort 的输入域就从“全量 splats”切换为“当前帧可见 splat 子集”。
3. 调试输出增强：
	- `CHUNK_DEBUG` 日志补充 `visible_chunks`、`active_splats` 与 `active_sort_count`，便于直接观察排序域是否缩小。

### 本次验证结果
1. 编译验证：
	- `cmake --build build --config Release` 通过。
2. review：
	- 已执行 `planner`、`tdd-guide`、`code-reviewer`。
	- review 指出的两个 correctness 问题已修复：
		- 恢复 `nextPow2` 的溢出保护。
		- 增加 `uint32` 索引上限与 chunk 元数据边界校验。
3. 运行时验证限制：
	- `./tools/run_step9_smoke.ps1 -RequireAcceptanceMarker` 在当前 harness 下依然拿不到 GUI runtime 的 acceptance marker。
	- `GS_RUNTIME_LOG_FILE` 在本 harness 下也未稳定写出新日志，因此本轮只能确认编译和代码路径，无法在 harness 内给出“排序域已缩小”的最终运行时日志证据。
	- 后续需要在本地终端直接运行 viewer，并观察 `GS_DEBUG_CHUNKS=1` 输出中的 `active_splats` / `active_sort_count` 是否随视角变化而明显下降。

### 当前结论
1. chunk 现在不只参与 coarse culling，也已经参与 depth/sort/view-data/draw 的工作域缩减。
2. 当前实现是最小侵入版本：
	- 不重写 bitonic sort。
	- 不改 view-data 输出布局。
	- 只通过“CPU 预种 visible indices + depth shader 消费 seeded indices”完成排序域缩减。
3. 下一阶段若继续对齐 UnityGaussianSplatting，可考虑把 per-frame 可见索引上传进一步替换为更纯 GPU 的 compaction / chunk-driven dispatch。

## Chunk 调度自适应 fallback 记录（2026-03-26）

### 本次核心改动
1. `src/render/GaussianRenderer.h/.cpp`
	- 将“资产支持 chunk scheduling”和“当前帧是否真的启用 seeded indices”拆成两个状态，避免 capability 与 policy 混用。
	- 引入可见比例驱动的自适应策略：
		- `< 0.80` 时启用 seeded indices 缩减排序域
		- `>= 0.90` 时退回 full-domain 路径
		- 中间区间维持上一帧路径，避免边界抖动
	- 新增 `visible_ratio` 调试字段，`CHUNK_DEBUG` 现在会输出 `path=seeded|full`。
2. 兜底与健壮性增强：
	- 若 visible index 累积会逼近 sort capacity，则直接回退 full-domain。
	- 若 `nextPow2(activeVisible)` 超过排序容量，直接回退 full-domain。
	- 若 `glBufferSubData` 上传 visible indices 失败，直接回退 full-domain，而不是跳过整帧。
	- 增加阈值顺序的 `static_assert`，并把 chunk radius 的 NaN/Infinity 检测改为显式 `std::isfinite(...)`。

### 本次验证结果
1. 编译：
	- `cmake --build build --config Release` 多轮通过。
2. review：
	- 已执行 `planner`、`tdd-guide`、`cpp-reviewer`、`code-reviewer`。
	- review 中提出的 capacity underflow、上传失败状态恢复、阈值可读性等问题均已修复。
3. 运行时限制：
	- 当前 harness 仍无法稳定抓取 GUI runtime 的 chunk debug / acceptance 日志，因此本轮主要完成了代码路径、build 与 review 级别的验证。
	- 后续应在本地终端以 `GS_DEBUG_CHUNKS=1` 运行，确认高可见比例场景会输出 `path=full`，低可见比例场景会输出 `path=seeded`。

### 当前结论
1. chunk 调度现在已经从“总是尝试缩减排序域”升级为“按可见比例自适应地缩减或回退”。
2. 这使当前 CPU-seeded 路径在低可见比例场景保留收益，同时避免高可见比例场景的无效索引上传成本。
3. 下一阶段若继续推进，最优先应落地 GPU-side compaction，用 GPU 替换当前每帧 CPU 构建并上传 visible indices 的热路径。
4. 在 GPU compaction 稳定后，再评估是否继续推进更彻底的 chunk-driven dispatch / multi-range scheduling，把 producer 端的 chunk range 调度直接延续到下游 dispatch/domain 契约。
5. 当前阈值策略外部化为可调试参数仍有价值，但它更偏调参与现场验证能力建设，优先级低于前两项。

## 运行时崩溃与验收闭环修复（2026-03-27）

### 问题现象
1. `./tools/run_step9_smoke.ps1 -RequireAcceptanceMarker -RequireGpuCompactionValidation -SkipVisualCheckPrompt` 在启动运行时阶段稳定失败。
2. 初始症状为运行时退出码 `-1073740791`；补上顶层异常捕获后，实际异常信息为：`No mapping for the Unicode character exists in the target multi-byte code page.`

### 根因定位
1. `assets/configs/model_path.txt` 中保存的是 Windows 本机代码页下的中文路径。
2. 运行时与工具链里多处把 `std::filesystem::path` 或原始路径文本直接做窄字符串转换：
	- 配置文本到 `std::filesystem::path`
	- `path.string()` 用于日志与错误信息
3. 在当前 Windows 环境下，这些转换会在启动期抛出未捕获 C++ 异常，导致查看器直接退出。

### 已落实修复
1. 新增共享路径工具：`src/core/PathUtils.h`
	- Windows 下显式按 `UTF-8 -> ACP -> OEMCP` 顺序解码文本路径，再构造 `std::filesystem::path`
	- 日志输出统一走 UTF-8 文本化，避免再次触发窄字符路径异常
2. 接入范围：
	- `src/main.cpp`
	- `src/cache/SplatCacheReader.cpp`
	- `src/io/PlyLoader.cpp`
	- `tools/gs_cache_builder/main.cpp`
	- `src/render/GaussianRenderer.cpp` 中的 runtime log 路径解析
3. `main.cpp` 保留了顶层异常捕获，后续若再出现启动期异常，可直接从日志拿到 `std::exception::what()`。

### GPU compaction 验收补充修复
1. `src/render/GaussianRenderer.cpp`
	- 修复 `prepareVisibleSplatDomainGpu()` 的校验门控：GPU scheduler 成功后，即便当前帧因高可见比例退回 full-domain，也会执行 CPU/GPU compaction 对照验证。
	- 修复 `validateGpuCompactionResult()` 的语义：当当前帧不使用 seeded domain 时，仅校验 counts/domain 决策，不再把 GPU compaction 的索引顺序当成硬错误。
	- 为 `glGetBufferSubData` 的回读路径补上显式 GL 错误检查。
2. 这使 `GS_VALIDATE_GPU_COMPACTION=1` 在当前 garden 场景的 full-domain fallback 场景下，也能稳定输出 `GPU_COMPACTION_VALIDATE_OK`。

### 本次验证结果
1. 构建：
	- `cmake --build build --config Release` 通过。
2. 直接运行：
	- 查看器不再启动即崩溃。
	- 日志确认首帧通过完整链路：`ACCEPT: pipeline frame completed (...)`
3. 完整 smoke：
	- `./tools/run_step9_smoke.ps1 -RequireAcceptanceMarker -RequireGpuCompactionValidation -SkipVisualCheckPrompt`
	- 结果：通过

## view-data 下游消费优化（2026-03-27）

### 本次改动
1. `src/render/GaussianRenderer.cpp/.h`
	- `runViewDataPass()` 新增 `activeDomainPreculled` 参数，把当前帧是否真正使用 seeded active domain 明确传给 view-data pipeline。
2. `src/render/ViewDataPipeline.cpp/.h`
	- 为 compute shader 新增 `u_activeDomainPreculled` uniform，并在 dispatch 时由 renderer 显式设置。
3. `assets/shaders/view_data.comp`
	- 当 `u_activeDomainPreculled=1` 时，跳过 `findChunkIndex()`、`isChunkVisible()` 以及对应的 `chunk_tests` / `chunk_culled_splats` 统计。
	- 当当前帧退回 full-domain 时，保留原有 chunk coarse culling 兜底逻辑不变。
4. `src/render/GaussianRenderer.cpp`
	- 顺手补强 CPU fallback 的防御式检查：忽略 `start_index` 越界的 chunk，并避免 `m_sortCapacity - m_visibleIndicesScratch.size()` 这类无符号下溢。

### 验证结论
1. `cmake --build build --config Release` 通过。
2. `./tools/run_step9_smoke.ps1 -ObserveSeconds 15 -RequireAcceptanceMarker -RequireGpuCompactionValidation -SkipVisualCheckPrompt` 通过。
3. 运行日志确认：
	- full-domain 路径未回归，仍能输出 `ACCEPT: pipeline frame completed (...)`
	- 同时稳定输出 `GPU_COMPACTION_VALIDATE_OK`
4. 当前默认 garden 首帧仍是 `path=full`、`visible_ratio=1.0`，因此这次自动验证确认了 full-domain 兼容性；seeded-path 的收益验证仍需在低可见比例视角下继续观测 `chunk_tests` 是否降为 `0`。
	- 日志同时包含：
		- `ACCEPT: pipeline frame completed (...)`
		- `GPU_COMPACTION_VALIDATE_OK: ...`

### 当前结论
1. 当前这轮优先级最高的问题已经收敛：运行时中文路径启动崩溃已修复。
2. Step 9 的 acceptance marker 与 GPU compaction validation 闭环现在都能在当前 harness 中稳定跑通。
3. 下一步可以回到原计划主线，继续推进真正的 GPU-side compaction / chunk-driven downstream consumption，而不是再花时间处理启动与验收基础设施问题。

## schedule-driven GPU compaction 下推（2026-03-27）

### 本次改动
1. `assets/shaders/chunk_schedule.comp`
	- GPU scheduler 现在只负责写 `scheduleEntries[]` 与 `ChunkSchedulerStats`，不再直接写 flat `indices[]`。
2. 新增 `assets/shaders/schedule_compact.comp`
	- 引入独立的 GPU compaction pass：按 `scheduleEntries[]` + `chunk_buffer` 统一展开到 `indices_buffer`。
3. 新增 `src/render/ScheduleCompactionPipeline.h/.cpp`
	- 封装 schedule -> indices 的 compute dispatch，供 CPU/GPU 两条 seeded path 复用。
4. `src/render/GaussianRenderer.h/.cpp`
	- GPU scheduler 成功后，若当前帧仍使用 seeded path，会显式运行 `ScheduleCompactionPipeline`。
	- CPU fallback 不再逐 splat 构建 `m_visibleIndicesScratch` 并上传 `indices_buffer`；改为只构建 `ChunkScheduleEntry` 列表，上传 `chunk_schedule_buffer` + `chunk_scheduler_stats_buffer`，再复用同一条 GPU compaction 链路。
5. `CMakeLists.txt`
	- 已把 `src/render/ScheduleCompactionPipeline.cpp` 接入运行时目标。

### 验证结果
1. `cmake --build build --config Release` 通过。
2. `./tools/run_step9_smoke.ps1 -ObserveSeconds 15 -RequireAcceptanceMarker -RequireGpuCompactionValidation -SkipVisualCheckPrompt` 通过。
3. 运行日志确认：
	- `ACCEPT: pipeline frame completed (...)` 仍稳定出现。
	- `GPU_COMPACTION_VALIDATE_OK` 仍稳定出现。
	- 说明新 schedule-driven compaction 没有破坏现有 depth/sort/view-data/draw 主链路。

### 当前结论
1. 主路径已经去掉“每帧 CPU 构建并上传整段 visible indices”这个热路径，改为“CPU/GPU 统一产出 schedule，GPU 统一展开”。
2. 这一步把 GPU scheduler 的结果继续往下游推进了一层，但下游 depth/sort/view-data/draw 仍然消费 flat `indices_buffer`，尚未直接改造成原生 schedule-aware dispatch。
3. 下一步若继续推进，应评估是否让 depth/view-data 等 pass 直接消费 schedule ranges，而不是先展开回 flat active domain。

## view-data schedule-aware 下推（2026-03-27）

### 本次改动
1. `src/render/ViewDataPipeline.h/.cpp`
	- 新增 `use_schedule_domain` 与 `schedule_entry_count` 参数。
	- 新增 binding `9` 的 `chunk_schedule_buffer` 绑定，并把 `u_useScheduleDomain` / `u_scheduleEntryCount` 传给 shader。
2. `assets/shaders/view_data.comp`
	- 新增 `ChunkScheduleEntry` 视图与 `ScheduleBuffer` 绑定。
	- 新增 `findScheduleIndex()`，在 seeded path 上可直接从 `gl_GlobalInvocationID.x` 反查所属 schedule entry。
	- 当 `u_useScheduleDomain=1` 时，view-data 不再通过 `sortedIndices[id] -> findChunkIndex(i)` 这条 flat active-domain 路径重新定位 chunk，而是直接从 schedule entry 推导 chunk 和 splat 源区间。
	- full-domain 路径仍保留原有 `sortedIndices + findChunkIndex()` 逻辑不变。
3. `src/render/GaussianRenderer.h/.cpp`
	- 新增 `m_viewDataUseScheduleDomainThisFrame`，仅在 seeded path 上打开 schedule-aware view-data 路径。
	- full-domain fallback 时仍关闭该路径，保持现有行为。

### 验证结果
1. `cmake --build build --config Release` 通过。
2. `./tools/run_step9_smoke.ps1 -ObserveSeconds 15 -RequireAcceptanceMarker -RequireGpuCompactionValidation -SkipVisualCheckPrompt` 通过。
3. 运行日志确认：
	- `ACCEPT: pipeline frame completed (...)` 稳定出现。
	- `GPU_COMPACTION_VALIDATE_OK` 稳定出现。

### 当前结论
1. schedule 不再只停留在 producer/compaction 边界，已经继续下推到了 view-data pass。
2. 当前真正仍然保留 flat `indices_buffer` 契约的下游主要是 depth/sort/draw，而 view-data 在 seeded path 上已经具备 schedule-aware 消费能力。

## 强制 seeded downstream 调试开关（2026-04-01）

### 本次改动
1. `src/render/GaussianRenderer.h/.cpp`
	- 新增环境变量 `GS_CHUNK_FORCE_SEEDED_PATH`，用于在当前帧已经成功生成合法 schedule/compacted domain 时，强制 depth/view-data 继续走 seeded schedule-domain 路径，而不是因高可见比例退回 full-domain。
	- 将该策略接到现有 `shouldUseSeededIndices()` 判定中，使 CPU path、GPU path 与 `validateGpuCompactionResult()` 共用同一套 seeded/full 决策语义。
	- `CHUNK_DEBUG_CONFIG` 与 `CHUNK_DEBUG` 日志新增 `force_seeded` 字段，便于区分“策略自然进入 seeded”与“调试强制进入 seeded”。
2. `README.md`
	- 文档补充 `GS_CHUNK_FORCE_SEEDED_PATH` 的用途与限制。

### 验证目标
1. 在当前默认 garden 场景 `visible_ratio=1.0` 的情况下，也能稳定把 downstream 切到 seeded path，真实观测 depth/view-data 对 schedule domain 的消费情况。
2. 同时保留 `GS_VALIDATE_GPU_COMPACTION=1` 的 CPU/GPU 对照验证，避免强制模式把错误隐藏掉。

### 继续修复
1. 强制 seeded 验证首次暴露了一个真实契约问题：
	- GPU scheduler 写出的 `scheduleEntries[]` 并不保证按 `outputOffset` 单调有序。
	- 但 `depth_keys.comp` / `view_data.comp` 里的 `findScheduleIndex()` 默认用二分查找，隐含要求 `scheduleEntries[]` 已按 `outputOffset` 排序。
	- 结果是在 GPU producer + seeded downstream 组合下，会出现 view-data 只处理部分 splat、或 GPU validation 因索引顺序假设过强而误报的情况。
2. 已落实修复：
	- `validateGpuCompactionResult()` 不再把 GPU compaction 的输出顺序当作硬约束，而是比较 source-index 集合是否一致。
	- `GaussianRenderer` 新增 `m_scheduleEntriesSortedThisFrame`，区分 CPU schedule（按 outputOffset 有序）与 GPU schedule（无序）。
	- `depth_keys.comp` 与 `view_data.comp` 仅在 `u_scheduleEntriesSorted=1` 时才走 schedule-entry 二分查找；否则回退为消费已经 compact 完成的 `indices_buffer` / `sortedIndices`，避免错误依赖“GPU schedule entry 数组天然有序”的假设。
	- `resetActiveDomainToFull()` 现在会同时清理 depth/view-data 的 schedule-domain 状态，避免 seeded path 失败后带着旧标志退回 full-domain。

### 本次验证结果
1. `cmake --build build --config Release --target gaussian_splatting_gl` 通过。
2. 基线日志（不强制 seeded）确认：
	- `producer=gpu`
	- `path=full`
	- `visible_ratio=1.0`
	- `chunk_tests=5834784`
3. 强制 seeded 日志确认：
	- `producer=gpu`
	- `path=seeded`
	- `force_seeded=1`
	- `visible_ratio=1.0`
	- `processed_splats=5834784`
	- `chunk_tests=0`
	- `GPU_COMPACTION_VALIDATE_OK` 稳定出现

### 当前结论
1. 当前默认 garden 场景下，已经可以在不改变可见比例的前提下，稳定把 GPU producer 的结果继续下推到 seeded downstream 并完成运行时验收。
2. 这次不是单纯新增一个调试开关，而是顺带修正了“GPU schedule entry 顺序天然可二分查找”这个错误前提。
3. 当前 GPU producer 场景下，下游验证到的是“compacted-index fallback downstream”而不是“直接消费无序 schedule entry 数组”；这条语义现在已经在文档中明确。
4. 现在可以更可信地继续推进真正的 schedule-aware downstream 优化，因为已经有办法在当前场景里稳定复现和验收 seeded path。
5. 下一步若继续推进，最自然的方向是评估 depth/sort 是否也能进一步减少对展开后 flat indices 的依赖，或继续把 draw 侧推进为更原生的 schedule/range 驱动契约。

## GPU sorted schedule lookup 下推（2026-04-01）

### 本次改动
1. `src/render/GpuUploadBuffers.h/.cpp`
	- 新增 `chunk_schedule_sort_keys_buffer` 与 `chunk_schedule_sort_indices_buffer`，为每帧 GPU schedule lookup 排序提供独立缓冲。
	- `GpuUploadStats` 新增 `chunk_schedule_sort_count`，把 schedule 排序容量回传给 renderer。
2. 新增 `assets/shaders/schedule_sort_init.comp`
	- 负责从 GPU scheduler 写出的 `scheduleEntries[]` 中提取 `outputOffset` 作为 sort key，并初始化 schedule 索引 payload。
3. 新增 `src/render/ScheduleSortInitPipeline.h/.cpp`
	- 封装 schedule sort key/index 初始化 dispatch。
4. `src/render/GaussianRenderer.h/.cpp`
	- 新增 `prepareSortedScheduleLookup()` 与通用 `runBitonicSort()`。
	- GPU seeded path 现在会先对 schedule 索引按 `outputOffset` 做 GPU bitonic sort，再把结果通过 `m_useSortedScheduleLookupThisFrame` 下发给 depth/view-data。
	- `CHUNK_DEBUG` 日志新增 `schedule_lookup` 字段，用于区分 `direct`、`indirect` 与 `fallback` 三种下游消费模式。
	- render state 保护现在补上了 SSBO binding `10` 的保存与恢复，避免把 sorted schedule lookup buffer 泄漏到调用方 GL 状态。
	- schedule 索引排序不再固定跑满 `nextPow2(total_chunk_count)`，而是按每帧 `visible schedule entry count` 的 padded 容量执行。
5. `assets/shaders/depth_keys.comp`
	- 当 schedule lookup 处于 direct/indirect 模式时，不再把 `indices[]` 当作 source index 载荷；而是把 compacted schedule index 作为排序 payload 保留下来，供 view-data 在排序后继续反查所属 schedule range。
6. `assets/shaders/view_data.comp`
	- 现在会在 GPU producer 场景下消费“按 `outputOffset` 排序后的 schedule 索引表”，通过 `sortedIndices[id] -> compactedIndex -> schedule range -> sourceIndex/chunkIndex` 这条链路恢复真正的 schedule-aware downstream。

### 本次验证结果
1. `cmake --build build --config Release --target gaussian_splatting_gl` 通过。
2. 以 detached GUI 方式运行：
	- `producer=gpu`
	- `path=seeded`
	- `schedule_lookup=indirect`
	- `processed_splats=5834784`
	- `chunk_tests=0`
	- `GPU_COMPACTION_VALIDATE_OK` 稳定出现
3. 在当前 harness 中，直接终端宿主启动查看器仍偶发 `Failed to upload split model buffers, GL error: 1285`；但 detached GUI 方式可稳定完成本轮验收。
4. 这说明当前 garden 场景下，GPU producer 的结果已经不再依赖 compacted-index fallback，而是真正进入了“sorted schedule 索引间接查表”的 schedule-aware downstream。

### 当前结论
1. CPU path 现在有两种合法的 schedule downstream 形式：
	- `schedule_lookup=direct`：CPU 直接生成有序 schedule entry，shader 可直接二分。
	- `schedule_lookup=indirect`：GPU producer 先生成无序 schedule entry，再用 GPU 排序出按 `outputOffset` 有序的索引表，shader 间接二分。
2. 只有当 schedule lookup 准备失败时，才需要回退到 compacted-index fallback。
3. depth/view-data 这条链路现在已经更接近真正的 chunk schedule-aware downstream；后续如果继续推进，重点应转向 draw/domain 契约是否还需要 flat active-domain 语义。

## draw 侧 compacted-domain 契约下推（2026-04-01）

### 本次改动
1. `assets/shaders/view_data.comp`
	- 在 seeded direct/indirect 模式下，不再把 `GPUViewSplat` 固定写到 `viewSplats[id]` 这个“最终 draw-order 已经打平”的隐式域中。
	- 改为先通过 `sortedIndices[id]` 取回 compacted schedule index，再把结果写到 `viewSplats[compactedIndex]`，使输出域与 schedule compact 后的逻辑域对齐。
2. `assets/shaders/gaussian.vert`
	- 新增对 `indices_buffer` 的显式读取；当 `u_useDrawIndirectLookup=1` 时，vertex shader 会通过 `sortedIndices[gl_InstanceID]` 找到当前实例对应的 compacted schedule index，再读取 `viewSplats[compactedIndex]`。
	- full/fallback 路径下仍保持原来的 `viewSplats[gl_InstanceID]` 读取方式，不改变默认主路径。
3. `src/render/ViewDataPipeline.h/.cpp`
	- 新增 `u_writeCompactedViewData` 契约开关，由 renderer 显式告诉 view-data pass 当前写出的到底是 flat draw-order 域还是 compacted schedule 域。
4. `src/render/GaussianRenderer.h/.cpp`
	- 新增 `m_drawUseIndirectLookupThisFrame`，把 draw 侧当前消费的是 `flat` 还是 `compacted` 变成显式状态，而不再由 shader 隐式假设。
	- `CHUNK_DEBUG` 日志新增 `draw_domain` 字段，用于区分 `flat` 与 `compacted`。

### 本次验证结果
1. `cmake --build build --config Release --target gaussian_splatting_gl` 通过。
2. 强制 seeded 验证日志确认：
	- `ACCEPT: pipeline frame completed`
	- `producer=gpu`
	- `path=seeded`
	- `schedule_lookup=indirect`
	- `draw_domain=compacted`
	- `GPU_COMPACTION_VALIDATE_OK`

### 当前结论
1. draw 侧现在已经开始显式消费 compacted schedule domain，而不是完全依赖“view_data 已经按最终 draw-order 打平”的隐式前提。
2. 当前真正仍保留的 flat active-domain 语义，更多是 draw submission 形式本身仍是单次 `glDrawArraysInstanced(active_splat_count)`，而不是 view-data / vertex fetch 之间的数据契约。
3. 后续如果继续推进，更自然的方向将是评估：
	- 是否要把 draw submission 本身也变成更显式的 range/indirection 驱动；
	- 以及是否有必要为 seeded path 引入更低开销的 draw-time indirection 形式，减少 vertex shader 对 `indices_buffer` 的重复读取。
