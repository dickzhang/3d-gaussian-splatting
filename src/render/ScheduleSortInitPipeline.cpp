#include "render/ScheduleSortInitPipeline.h"

#include <iostream>
#include <limits>

namespace gs
{
	namespace
	{

		constexpr GLuint kComputeWorkGroupSize = 256u;

	} // namespace

	bool ScheduleSortInitPipeline::initialize()
	{
		if (!m_program.createComputeFromFile("assets/shaders/schedule_sort_init.comp"))
		{
			std::cerr << "Failed to initialize schedule sort init compute shader\n";
			return false;
		}

		m_schedule_entry_count_loc = glGetUniformLocation(m_program.id(), "u_scheduleEntryCount");
		m_sort_count_loc = glGetUniformLocation(m_program.id(), "u_sortCount");
		if (m_schedule_entry_count_loc < 0 || m_sort_count_loc < 0)
		{
			std::cerr << "Failed to resolve one or more schedule sort init uniform locations\n";
			return false;
		}

		return true;
	}

	bool ScheduleSortInitPipeline::dispatch(
		const GpuUploadBufferHandles& handles,
		std::size_t scheduleEntryCount,
		std::size_t sortCount)
	{
		if (sortCount == 0)
		{
			return true;
		}

		if (handles.chunk_schedule_buffer == 0 ||
			handles.chunk_schedule_sort_keys_buffer == 0 ||
			handles.chunk_schedule_sort_indices_buffer == 0)
		{
			std::cerr << "Schedule sort init dispatch requires initialized GPU buffers\n";
			return false;
		}

		if (scheduleEntryCount > static_cast<std::size_t>(std::numeric_limits<GLuint>::max()) ||
			sortCount > static_cast<std::size_t>(std::numeric_limits<GLuint>::max()))
		{
			std::cerr << "Schedule sort init dispatch exceeds GLuint address range\n";
			return false;
		}

		m_program.use();
		glUniform1ui(m_schedule_entry_count_loc, static_cast<GLuint>(scheduleEntryCount));
		glUniform1ui(m_sort_count_loc, static_cast<GLuint>(sortCount));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, handles.chunk_schedule_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, handles.chunk_schedule_sort_keys_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, handles.chunk_schedule_sort_indices_buffer);

		const GLuint groups = static_cast<GLuint>((sortCount + (kComputeWorkGroupSize - 1)) / kComputeWorkGroupSize);
		glDispatchCompute(groups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		const GLenum dispatch_error = glGetError();
		if (dispatch_error != GL_NO_ERROR)
		{
			std::cerr << "ScheduleSortInitPipeline dispatch GL error: " << dispatch_error << "\n";
			return false;
		}

		return true;
	}

} // namespace gs
