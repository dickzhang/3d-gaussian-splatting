#include "render/ChunkSchedulerPipeline.h"

#include <iostream>
#include <limits>

#include <glm/gtc/type_ptr.hpp>

namespace gs
{
	namespace
	{

		constexpr GLuint kComputeWorkGroupSize = 256u;

	} // namespace

	bool ChunkSchedulerPipeline::initialize()
	{
		if (!m_program.createComputeFromFile("assets/shaders/chunk_schedule.comp"))
		{
			std::cerr << "Failed to initialize chunk scheduler compute shader\n";
			return false;
		}

		m_view_loc = glGetUniformLocation(m_program.id(), "u_view");
		m_model_loc = glGetUniformLocation(m_program.id(), "u_model");
		m_proj_loc = glGetUniformLocation(m_program.id(), "u_proj");
		m_chunk_count_loc = glGetUniformLocation(m_program.id(), "u_chunkCount");
		m_sort_capacity_loc = glGetUniformLocation(m_program.id(), "u_sortCapacity");
		m_schedule_capacity_loc = glGetUniformLocation(m_program.id(), "u_scheduleCapacity");

		const bool uniforms_ok =
			m_view_loc >= 0 &&
			m_model_loc >= 0 &&
			m_proj_loc >= 0 &&
			m_chunk_count_loc >= 0 &&
			m_sort_capacity_loc >= 0 &&
			m_schedule_capacity_loc >= 0;
		if (!uniforms_ok)
		{
			std::cerr << "Failed to resolve one or more chunk scheduler uniform locations\n";
			return false;
		}

		return true;
	}

	bool ChunkSchedulerPipeline::dispatch(
		const GpuUploadBufferHandles& handles,
		const glm::mat4& view,
		const glm::mat4& model,
		const glm::mat4& projection,
		std::size_t chunk_count,
		std::size_t sort_capacity,
		DispatchStats* out_stats)
	{
		if (out_stats != nullptr)
		{
			*out_stats = DispatchStats{};
		}

		if (chunk_count == 0)
		{
			return true;
		}

		if (handles.chunk_buffer == 0 ||
			handles.chunk_scheduler_stats_buffer == 0 ||
			handles.chunk_schedule_buffer == 0)
		{
			std::cerr << "Chunk scheduler dispatch requires initialized GPU buffers\n";
			return false;
		}

		if (chunk_count > static_cast<std::size_t>(std::numeric_limits<GLuint>::max()) ||
			sort_capacity > static_cast<std::size_t>(std::numeric_limits<GLuint>::max()))
		{
			std::cerr << "Chunk scheduler dispatch exceeds GLuint address range\n";
			return false;
		}

		m_program.use();
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1ui(m_chunk_count_loc, static_cast<GLuint>(chunk_count));
		glUniform1ui(m_sort_capacity_loc, static_cast<GLuint>(sort_capacity));
		glUniform1ui(m_schedule_capacity_loc, static_cast<GLuint>(chunk_count));

		const ChunkSchedulerStats zero_stats{};
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.chunk_scheduler_stats_buffer);
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER,
			0,
			static_cast<GLsizeiptr>(sizeof(ChunkSchedulerStats)),
			&zero_stats);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, handles.chunk_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, handles.chunk_scheduler_stats_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, handles.chunk_schedule_buffer);

		const GLuint groups = static_cast<GLuint>((chunk_count + (kComputeWorkGroupSize - 1)) / kComputeWorkGroupSize);
		glDispatchCompute(groups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		if (out_stats != nullptr)
		{
			ChunkSchedulerStats raw_stats{};
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.chunk_scheduler_stats_buffer);
			glGetBufferSubData(
				GL_SHADER_STORAGE_BUFFER,
				0,
				static_cast<GLsizeiptr>(sizeof(ChunkSchedulerStats)),
				&raw_stats);
			out_stats->visible_chunk_count = raw_stats.visible_chunk_count;
			out_stats->active_splat_count = raw_stats.active_splat_count;
			out_stats->schedule_entry_count = raw_stats.schedule_entry_count;
			out_stats->overflowed = raw_stats.overflow_flag != 0;
		}

		const GLenum dispatch_error = glGetError();
		if (dispatch_error != GL_NO_ERROR)
		{
			std::cerr << "ChunkSchedulerPipeline dispatch GL error: " << dispatch_error << "\n";
			return false;
		}

		return true;
	}

} // namespace gs