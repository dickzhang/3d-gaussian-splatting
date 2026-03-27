#include "render/ScheduleCompactionPipeline.h"

#include <iostream>
#include <limits>

namespace gs
{
	namespace
	{

		constexpr GLuint kComputeWorkGroupSize = 256u;

	} // namespace

	bool ScheduleCompactionPipeline::initialize()
	{
		if (!m_program.createComputeFromFile("assets/shaders/schedule_compact.comp"))
		{
			std::cerr << "Failed to initialize schedule compaction compute shader\n";
			return false;
		}

		m_chunk_count_loc = glGetUniformLocation(m_program.id(), "u_chunkCount");
		m_schedule_entry_count_loc = glGetUniformLocation(m_program.id(), "u_scheduleEntryCount");
		m_sort_capacity_loc = glGetUniformLocation(m_program.id(), "u_sortCapacity");

		const bool uniforms_ok =
			m_chunk_count_loc >= 0 &&
			m_schedule_entry_count_loc >= 0 &&
			m_sort_capacity_loc >= 0;
		if (!uniforms_ok)
		{
			std::cerr << "Failed to resolve one or more schedule compaction uniform locations\n";
			return false;
		}

		return true;
	}

	bool ScheduleCompactionPipeline::dispatch(
		const GpuUploadBufferHandles& handles,
		std::size_t chunkCount,
		std::size_t scheduleEntryCount,
		std::size_t sortCapacity)
	{
		if (scheduleEntryCount == 0)
		{
			return true;
		}

		if (handles.chunk_buffer == 0 ||
			handles.chunk_schedule_buffer == 0 ||
			handles.indices_buffer == 0)
		{
			std::cerr << "Schedule compaction dispatch requires initialized GPU buffers\n";
			return false;
		}

		if (chunkCount > static_cast<std::size_t>(std::numeric_limits<GLuint>::max()) ||
			scheduleEntryCount > static_cast<std::size_t>(std::numeric_limits<GLuint>::max()) ||
			sortCapacity > static_cast<std::size_t>(std::numeric_limits<GLuint>::max()))
		{
			std::cerr << "Schedule compaction dispatch exceeds GLuint address range\n";
			return false;
		}

		m_program.use();
		glUniform1ui(m_chunk_count_loc, static_cast<GLuint>(chunkCount));
		glUniform1ui(m_schedule_entry_count_loc, static_cast<GLuint>(scheduleEntryCount));
		glUniform1ui(m_sort_capacity_loc, static_cast<GLuint>(sortCapacity));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, handles.chunk_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, handles.chunk_schedule_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, handles.indices_buffer);

		const GLuint groups = static_cast<GLuint>((scheduleEntryCount + (kComputeWorkGroupSize - 1)) / kComputeWorkGroupSize);
		glDispatchCompute(groups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		const GLenum dispatch_error = glGetError();
		if (dispatch_error != GL_NO_ERROR)
		{
			std::cerr << "ScheduleCompactionPipeline dispatch GL error: " << dispatch_error << "\n";
			return false;
		}

		return true;
	}

} // namespace gs