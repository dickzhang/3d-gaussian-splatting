#pragma once

#include <cstddef>

#include <glad/glad.h>

#include "core/ShaderProgram.h"
#include "render/GpuUploadBuffers.h"

namespace gs
{

	class ScheduleSortInitPipeline
	{
	public:
		bool initialize();
		bool dispatch(
			const GpuUploadBufferHandles& handles,
			std::size_t scheduleEntryCount,
			std::size_t sortCount);

	private:
		ShaderProgram m_program;
		GLint m_schedule_entry_count_loc{ -1 };
		GLint m_sort_count_loc{ -1 };
	};

} // namespace gs
