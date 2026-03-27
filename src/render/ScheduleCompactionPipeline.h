#pragma once

#include <cstddef>

#include <glad/glad.h>

#include "core/ShaderProgram.h"
#include "render/GpuUploadBuffers.h"

namespace gs
{

	class ScheduleCompactionPipeline
	{
	public:
		bool initialize();
		bool dispatch(
			const GpuUploadBufferHandles& handles,
			std::size_t chunkCount,
			std::size_t scheduleEntryCount,
			std::size_t sortCapacity);

	private:
		ShaderProgram m_program;
		GLint m_chunk_count_loc{ -1 };
		GLint m_schedule_entry_count_loc{ -1 };
		GLint m_sort_capacity_loc{ -1 };
	};

} // namespace gs