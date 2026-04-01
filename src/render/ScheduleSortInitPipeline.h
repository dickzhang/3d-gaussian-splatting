#pragma once

#include <cstddef>

#include <glad/glad.h>

#include "core/ShaderProgram.h"
#include "render/GpuUploadBuffers.h"

namespace gs
{

	// 调度排序初始化管线：为 chunk schedule 构建 bitonic sort 所需的初始键和值。
	class ScheduleSortInitPipeline
	{
	public:
		// 初始化排序键生成 compute shader 与 uniform 位置。
		bool initialize();
		// 为指定数量的 schedule 项写入初始排序键和索引。
		bool dispatch(
			const GpuUploadBufferHandles& handles,
			std::size_t scheduleEntryCount,
			std::size_t sortCount);

	private:
		ShaderProgram m_program; // schedule sort init compute 程序
		GLint m_schedule_entry_count_loc{ -1 }; // u_scheduleEntryCount 位置
		GLint m_sort_count_loc{ -1 }; // u_sortCount 位置
	};

} // namespace gs
