#pragma once

#include <cstddef>

#include <glad/glad.h>

#include "core/ShaderProgram.h"
#include "render/GpuUploadBuffers.h"

namespace gs
{

	// 调度压缩管线：将可见 chunk 调度表展开为连续的 splat 索引域。
	class ScheduleCompactionPipeline
	{
	public:
		// 初始化调度压缩 compute shader 与 uniform 位置。
		bool initialize();
		// 根据 chunk 调度表生成供排序和绘制消费的紧凑索引序列。
		bool dispatch(
			const GpuUploadBufferHandles& handles,
			std::size_t chunkCount,
			std::size_t scheduleEntryCount,
			std::size_t sortCapacity);

	private:
		ShaderProgram m_program; // schedule compaction compute 程序
		GLint m_chunk_count_loc{ -1 }; // u_chunkCount 位置
		GLint m_schedule_entry_count_loc{ -1 }; // u_scheduleEntryCount 位置
		GLint m_sort_capacity_loc{ -1 }; // u_sortCapacity 位置
	};

} // namespace gs