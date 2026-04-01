#pragma once

#include <cstddef>
#include <cstdint>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>

#include "core/ShaderProgram.h"
#include "render/GpuUploadBuffers.h"

namespace gs
{

	// chunk 调度管线：按视锥可见性筛选 chunk，并生成后续渲染使用的活动域。
	class ChunkSchedulerPipeline
	{
	public:
		// GPU chunk 调度结果统计。
		struct DispatchStats
		{
			std::uint32_t visible_chunk_count{ 0 }; // 当前帧可见 chunk 数量
			std::uint32_t active_splat_count{ 0 }; // 可见 chunk 覆盖的 splat 数量
			std::uint32_t schedule_entry_count{ 0 }; // 写入的调度项数量
			bool overflowed{ false }; // 调度输出是否超出缓冲容量
		};

		// 初始化 chunk scheduler compute shader 与 uniform 位置。
		bool initialize();
		// 执行 chunk 级可见性筛选，并输出调度表与统计信息。
		bool dispatch(
			const GpuUploadBufferHandles& handles,
			const glm::mat4& view,
			const glm::mat4& model,
			const glm::mat4& projection,
			std::size_t chunk_count,
			std::size_t sort_capacity,
			DispatchStats* out_stats = nullptr);

	private:
		ShaderProgram m_program; // chunk scheduler compute 程序
		GLint m_view_loc{ -1 }; // u_view 位置
		GLint m_model_loc{ -1 }; // u_model 位置
		GLint m_proj_loc{ -1 }; // u_proj 位置
		GLint m_chunk_count_loc{ -1 }; // u_chunkCount 位置
		GLint m_sort_capacity_loc{ -1 }; // u_sortCapacity 位置
		GLint m_schedule_capacity_loc{ -1 }; // u_scheduleCapacity 位置
	};

} // namespace gs