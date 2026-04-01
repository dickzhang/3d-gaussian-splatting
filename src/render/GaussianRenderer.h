#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <glm/mat4x4.hpp>

#include "core/ShaderProgram.h"
#include "render/ChunkSchedulerPipeline.h"
#include "render/GpuSplatLayout.h"
#include "render/GpuUploadBuffers.h"
#include "render/ScheduleCompactionPipeline.h"
#include "render/ScheduleSortInitPipeline.h"
#include "render/ViewDataPipeline.h"
#include "runtime/RuntimeSplatAsset.h"

namespace gs
{

	// 高斯渲染器：管理 GPU 资源并执行高斯元绘制
	class GaussianRenderer
	{
	public:
		GaussianRenderer() = default;
		~GaussianRenderer();

		// 禁止拷贝，避免 OpenGL 资源重复释放
		GaussianRenderer(const GaussianRenderer&) = delete;
		GaussianRenderer& operator=(const GaussianRenderer&) = delete;

		// 初始化着色器与 GPU 侧资源
		bool initialize();
		// 上传运行时缓存资产到 GPU（cache-first 路径）
		bool uploadAsset(const RuntimeSplatAsset& asset);
		// 执行一帧渲染
		void render(const glm::mat4& view, const glm::mat4& projection, float viewportWidth, float viewportHeight);
		// 设置是否启用各向异性高斯
		void setUseAnisotropic(bool enabled);
		// 查询各向异性开关
		bool useAnisotropic() const noexcept;
		// 设置 SH 阶数，按模型能力自动裁剪
		int setShDegree(int degree);
		// 获取当前 SH 阶数
		int shDegree() const noexcept;
		// 获取模型支持的最大 SH 阶数
		int maxSupportedShDegree() const noexcept;
		// 设置模型到世界的变换矩阵
		void setModelTransform(const glm::mat4& model) noexcept;
		// 获取当前模型变换矩阵
		const glm::mat4& modelTransform() const noexcept;
	private:
		// 确保颜色累积目标尺寸与资源有效
		bool ensureAccumulationTarget(int width, int height);
		// 基于 chunk 可见性准备当前帧的排序/绘制域
		bool prepareVisibleSplatDomain(const glm::mat4& view, const glm::mat4& projection);
		bool prepareVisibleSplatDomainCpu(const glm::mat4& view, const glm::mat4& projection, bool previousSeededPath);
		bool prepareVisibleSplatDomainGpu(const glm::mat4& view, const glm::mat4& projection, bool previousSeededPath);
		bool prepareSortedScheduleLookup(std::size_t scheduleEntryCount);
		bool runScheduleCompaction(std::size_t scheduleEntryCount);
		bool runBitonicSort(GLuint keyBuffer, GLuint indexBuffer, std::size_t count);
		bool validateGpuCompactionResult(
			const glm::mat4& view,
			const glm::mat4& projection,
			const ChunkSchedulerPipeline::DispatchStats& schedulerStats,
			bool previousSeededPath);
		void resetActiveDomainToFull() noexcept;
		bool shouldUseSeededIndices(bool previousSeededPath, float visibleRatio) const noexcept;
		void loadChunkSchedulingConfig();
		// 执行深度键计算与 GPU 排序
		void runDepthAndSort(const glm::mat4& view);
		// 执行 view-dependent 数据预计算
		bool runViewDataPass(
			const glm::mat4& view,
			const glm::mat4& projection,
			float viewportWidth,
			float viewportHeight,
			bool activeDomainPreculled,
			ViewDataPipeline::DispatchStats* outStats);
		// 执行高斯累积绘制 pass
		void drawGaussianPass(float viewportWidth, float viewportHeight, bool useReferencePath);
		// 执行累积结果合成 pass
		void compositeAccumulationPass(GLint prevDrawFbo, GLint prevReadFbo, const std::array<GLint, 4>& prevViewport);

		ShaderProgram m_drawProgram;      // 绘制程序
		ShaderProgram m_depthProgram;     // 深度键计算程序
		ShaderProgram m_sortProgram;      // bitonic 排序程序
		ShaderProgram m_compositeProgram; // 合成程序
		ChunkSchedulerPipeline m_chunkSchedulerPipeline; // chunk 调度计算管线
		ScheduleCompactionPipeline m_scheduleCompactionPipeline; // schedule 展开到 indices 的压缩管线
		ScheduleSortInitPipeline m_scheduleSortInitPipeline; // schedule 排序键初始化管线
		ViewDataPipeline m_viewDataPipeline; // 视图预计算管线

		bool m_initialized{ false };    // 初始化状态
		unsigned int m_vao{ 0 };          // 空 VAO
		GpuUploadBufferHandles m_uploadBuffers{}; // 上传相关 SSBO
		unsigned int m_accumFbo{ 0 };     // 累积 FBO
		unsigned int m_accumColorTex{ 0 };// 累积颜色纹理

		int m_accumWidth{ 0 };  // 累积目标宽度
		int m_accumHeight{ 0 }; // 累积目标高度

		std::vector<SplatCacheChunkEntry> m_chunks; // CPU 侧 chunk 元数据
		std::vector<ChunkScheduleEntry> m_visibleScheduleScratch; // CPU fallback 生成的可见 chunk schedule

		std::size_t m_totalSplatCount{ 0 }; // 上传资产的总 splat 数量
		std::size_t m_activeSplatCount{ 0 }; // 当前帧参与排序/绘制的 splat 数量
		std::size_t m_sortCapacity{ 0 }; // indices/keys buffer 的容量（2 的幂）
		std::size_t m_scheduleSortCapacity{ 0 }; // schedule 排序键/索引缓冲容量（2 的幂）
		std::size_t m_sortCount{ 0 };  // 当前帧排序数量（2 的幂）
		std::size_t m_chunkCount{ 0 }; // chunk 总数量
		std::size_t m_visibleChunkCount{ 0 }; // 当前帧可见 chunk 数量
		std::size_t m_visibleScheduleEntryCount{ 0 }; // 当前帧可见 chunk schedule 数量
		std::size_t m_compactedSplatCount{ 0 }; // 当前帧 compacted splat 数量（策略前）
		float m_lastVisibleRatio{ 1.0f }; // 当前帧可见 splat 占总量的比例
		bool m_depthUseScheduleDomainThisFrame{ false }; // 当前帧 depth pass 是否直接消费 schedule 域
		bool m_scheduleEntriesSortedThisFrame{ false }; // 当前帧 schedule entries 是否按 outputOffset 有序
		bool m_useSortedScheduleLookupThisFrame{ false }; // 当前帧是否使用 sorted schedule index 间接查表
		bool m_viewDataUseScheduleDomainThisFrame{ false }; // 当前帧 view-data 是否直接消费 schedule 域

		GLint m_drawViewportSizeLoc{ -1 }; // 绘制程序 u_viewportSize 位置
		GLint m_drawUseAnisotropicLoc{ -1 }; // 绘制程序 u_useAnisotropic 位置

		GLint m_depthViewLoc{ -1 };         // 深度程序 u_view 位置
		GLint m_depthModelLoc{ -1 };        // 深度程序 u_model 位置
		GLint m_depthRealCountLoc{ -1 };    // 深度程序 u_realCount 位置
		GLint m_depthSortCountLoc{ -1 };    // 深度程序 u_sortCount 位置
		GLint m_depthChunkCountLoc{ -1 };   // 深度程序 u_chunkCount 位置
		GLint m_depthScheduleEntryCountLoc{ -1 }; // 深度程序 u_scheduleEntryCount 位置
		GLint m_depthInputLayoutLoc{ -1 };  // 深度程序 u_inputLayout 位置
		GLint m_depthUseSeedIndicesLoc{ -1 }; // 深度程序 u_useSeedIndices 位置
		GLint m_depthUseScheduleDomainLoc{ -1 }; // 深度程序 u_useScheduleDomain 位置
		GLint m_depthScheduleEntriesSortedLoc{ -1 }; // 深度程序 u_scheduleEntriesSorted 位置
		GLint m_depthUseSortedScheduleLookupLoc{ -1 }; // 深度程序 u_useSortedScheduleLookup 位置

		GLint m_sortCountLoc{ -1 };  // 排序程序 u_count 位置
		GLint m_sortStageLoc{ -1 };  // 排序程序 u_stage 位置
		GLint m_sortPassLoc{ -1 };   // 排序程序 u_pass 位置
		GLint m_compositeTexLoc{ -1 };// 合成程序 u_accumTex 位置

		float m_maxPointSize{ 128.0f }; // 设备支持的最大点尺寸
		bool m_useAnisotropic{ true };  // 各向异性开关
		bool m_debugChunkStats{ false }; // 是否输出 chunk 调试统计
		bool m_loggedAcceptanceFrame{ false }; // 首帧完整链路验收标记是否已输出
		bool m_loggedCompositeFallback{ false }; // 是否已输出 composite fallback 警告
		bool m_loggedChunkSchedulingConfig{ false }; // 是否已输出 chunk 调度配置
		bool m_hasChunkSchedulingSupport{ false }; // 当前资产是否支持 chunk 驱动调度
		bool m_useSeededIndicesThisFrame{ false }; // 当前帧是否启用 seeded indices 缩减排序域
		bool m_forceSeededPath{ false }; // 是否强制走 seeded/schedule downstream 调试路径
		bool m_usedGpuSchedulerThisFrame{ false }; // 当前帧是否使用 GPU scheduler 生成 active domain
		bool m_validateGpuCompaction{ false }; // 是否启用 GPU compaction CPU/GPU 对照验证
		std::uint64_t m_renderFrameIndex{ 0 }; // 渲染帧序号
		std::uint32_t m_chunkDebugLogCount{ 0 }; // 已输出的 chunk 调试日志次数
		std::uint32_t m_gpuValidationLogCount{ 0 }; // 已输出 GPU compaction 验证日志次数
		int m_shDegree{ 1 };            // 当前 SH 阶数
		int m_maxSupportedShDegree{ 0 };// 模型支持的最大 SH 阶数
		int m_inputLayout{ 1 };         // 1: split sections
		float m_chunkSchedulingEnableVisibleRatio{ 0.80f }; // seeded path 启用阈值
		float m_chunkSchedulingDisableVisibleRatio{ 0.90f }; // seeded path 退出阈值
		enum class ChunkSchedulerMode
		{
			Auto,
			Cpu,
			Gpu,
			Full,
		};
		ChunkSchedulerMode m_chunkSchedulerMode{ ChunkSchedulerMode::Auto }; // chunk 调度策略模式
		glm::mat4 m_modelTransform{ 1.0f }; // 模型到世界变换
	};

} // namespace gs
