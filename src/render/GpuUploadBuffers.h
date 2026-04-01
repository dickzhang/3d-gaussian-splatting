#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <glad/glad.h>

#include "runtime/RuntimeSplatAsset.h"
#include "render/GpuSplatLayout.h"

namespace gs
{

	// GPU 资源句柄集合：集中持有渲染与调度阶段使用的各类缓冲区。
	struct GpuUploadBufferHandles
	{
		GLuint splat_buffer{ 0 }; // 打包输入布局缓冲
		GLuint position_buffer{ 0 }; // 位置分段缓冲
		GLuint other_buffer{ 0 }; // 缩放与旋转分段缓冲
		GLuint color_buffer{ 0 }; // 颜色分段缓冲
		GLuint sh_buffer{ 0 }; // 球谐系数分段缓冲
		GLuint chunk_buffer{ 0 }; // chunk 元数据缓冲
		GLuint chunk_schedule_buffer{ 0 }; // 可见 chunk 调度表缓冲
		GLuint chunk_schedule_sort_keys_buffer{ 0 }; // chunk schedule 排序键缓冲
		GLuint chunk_schedule_sort_indices_buffer{ 0 }; // chunk schedule 排序索引缓冲
		GLuint chunk_scheduler_stats_buffer{ 0 }; // chunk scheduler 统计缓冲
		GLuint keys_buffer{ 0 }; // splat 深度键缓冲
		GLuint indices_buffer{ 0 }; // splat 排序索引缓冲
		GLuint view_data_buffer{ 0 }; // 视图相关 splat 数据缓冲
		GLuint draw_indirect_command_buffer{ 0 }; // 间接绘制命令缓冲
	};

	// DrawArraysIndirect 命令结构，对应 glDrawArraysIndirect 的参数布局。
	struct DrawArraysIndirectCommand
	{
		std::uint32_t count{ 0 }; // 每个实例绘制的顶点数
		std::uint32_t instance_count{ 0 }; // 提交的实例数量
		std::uint32_t first{ 0 }; // 起始顶点索引
		std::uint32_t base_instance{ 0 }; // 起始实例偏移
	};
	static_assert(sizeof(DrawArraysIndirectCommand) == 16, "Unexpected DrawArraysIndirectCommand size");

	// 可见 chunk 调度项，描述一个 chunk 在紧凑活动域中的写入区间。
	struct ChunkScheduleEntry
	{
		std::uint32_t chunk_index{ 0 }; // 对应的 chunk 编号
		std::uint32_t output_offset{ 0 }; // 该 chunk 在紧凑活动域中的起始偏移
		std::uint32_t splat_count{ 0 }; // 该 chunk 覆盖的 splat 数量
		std::uint32_t reserved{ 0 }; // 预留字段
	};
	static_assert(sizeof(ChunkScheduleEntry) == 16, "Unexpected ChunkScheduleEntry size");

	// GPU chunk 调度统计结构，与 shader 写回布局保持一致。
	struct ChunkSchedulerStats
	{
		std::uint32_t visible_chunk_count{ 0 }; // 可见 chunk 数量
		std::uint32_t active_splat_count{ 0 }; // 可见 chunk 覆盖的 splat 数量
		std::uint32_t schedule_entry_count{ 0 }; // 写出的调度项数量
		std::uint32_t overflow_flag{ 0 }; // 输出是否溢出
	};
	static_assert(sizeof(ChunkSchedulerStats) == 16, "Unexpected ChunkSchedulerStats size");

	// 上传完成后的统计信息，供渲染器记录容量和能力上限。
	struct GpuUploadStats
	{
		std::size_t splat_count{ 0 }; // 上传后的 splat 总数
		std::size_t sort_count{ 0 }; // splat 排序缓冲容量
		std::size_t chunk_count{ 0 }; // chunk 总数
		std::size_t chunk_schedule_sort_count{ 0 }; // schedule 排序缓冲容量
		int max_supported_sh_degree{ 0 }; // 资产支持的最大 SH 阶数
	};

	class GpuUploadBuffers
	{
	public:
		// 创建渲染与调度阶段需要的所有 GPU 缓冲。
		static bool create(GpuUploadBufferHandles& out_handles);
		// 销毁已创建的 GPU 缓冲。
		static void destroy(GpuUploadBufferHandles& handles);
		// 将 split-section 资产上传到 GPU，并初始化排序与绘制辅助缓冲。
		static bool upload_split_splats(
			const GpuUploadBufferHandles& handles,
			const RuntimeSplatAsset& asset,
			GpuUploadStats& out_stats);
	};

} // namespace gs
