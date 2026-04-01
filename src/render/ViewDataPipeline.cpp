#include "render/ViewDataPipeline.h"

#include <algorithm>
#include <iostream>
#include <limits>

#include <glm/gtc/type_ptr.hpp>

namespace gs
{
	namespace
	{

		constexpr GLuint kComputeWorkGroupSize = 256u;

	} // namespace

	bool ViewDataPipeline::initialize()
	{
		if (!m_program.createComputeFromFile("assets/shaders/view_data.comp"))
		{
			std::cerr << "Failed to initialize view-data compute shader\n";
			return false;
		}

		m_view_loc = glGetUniformLocation(m_program.id(), "u_view");
		m_model_loc = glGetUniformLocation(m_program.id(), "u_model");
		m_proj_loc = glGetUniformLocation(m_program.id(), "u_proj");
		m_viewport_size_loc = glGetUniformLocation(m_program.id(), "u_viewportSize");
		m_max_point_size_loc = glGetUniformLocation(m_program.id(), "u_maxPointSize");
		m_active_domain_preculled_loc = glGetUniformLocation(m_program.id(), "u_activeDomainPreculled");
		m_use_schedule_domain_loc = glGetUniformLocation(m_program.id(), "u_useScheduleDomain");
		m_write_compacted_view_data_loc = glGetUniformLocation(m_program.id(), "u_writeCompactedViewData");
		m_schedule_entries_sorted_loc = glGetUniformLocation(m_program.id(), "u_scheduleEntriesSorted");
		m_use_sorted_schedule_lookup_loc = glGetUniformLocation(m_program.id(), "u_useSortedScheduleLookup");
		m_use_anisotropic_loc = glGetUniformLocation(m_program.id(), "u_useAnisotropic");
		m_camera_pos_loc = glGetUniformLocation(m_program.id(), "u_cameraPos");
		m_sh_degree_loc = glGetUniformLocation(m_program.id(), "u_shDegree");
		m_chunk_count_loc = glGetUniformLocation(m_program.id(), "u_chunkCount");
		m_schedule_entry_count_loc = glGetUniformLocation(m_program.id(), "u_scheduleEntryCount");
		m_real_count_loc = glGetUniformLocation(m_program.id(), "u_realCount");
		m_input_layout_loc = glGetUniformLocation(m_program.id(), "u_inputLayout");

		const bool uniforms_ok =
			m_view_loc >= 0 &&
			m_model_loc >= 0 &&
			m_proj_loc >= 0 &&
			m_viewport_size_loc >= 0 &&
			m_max_point_size_loc >= 0 &&
			m_active_domain_preculled_loc >= 0 &&
			m_use_schedule_domain_loc >= 0 &&
			m_write_compacted_view_data_loc >= 0 &&
			m_schedule_entries_sorted_loc >= 0 &&
			m_use_sorted_schedule_lookup_loc >= 0 &&
			m_use_anisotropic_loc >= 0 &&
			m_camera_pos_loc >= 0 &&
			m_sh_degree_loc >= 0 &&
			m_chunk_count_loc >= 0 &&
			m_schedule_entry_count_loc >= 0 &&
			m_real_count_loc >= 0 &&
			m_input_layout_loc >= 0;
		if (!uniforms_ok)
		{
			std::cerr << "Failed to resolve one or more view-data uniform locations\n";
			return false;
		}

		return true;
	}

	bool ViewDataPipeline::dispatch(
		const GpuUploadBufferHandles& handles,
		int input_layout,
		const glm::mat4& view,
		const glm::mat4& model,
		const glm::mat4& projection,
		const glm::vec3& camera_pos,
		float viewport_width,
		float viewport_height,
		float max_point_size,
		bool active_domain_preculled,
		bool use_schedule_domain,
		bool write_compacted_view_data,
		bool schedule_entries_sorted,
		bool use_sorted_schedule_lookup,
		bool use_anisotropic,
		int sh_degree,
		std::size_t chunk_count,
		std::size_t schedule_entry_count,
		std::size_t splat_count,
		DispatchStats* out_stats)
	{
		if (out_stats != nullptr)
		{
			*out_stats = DispatchStats{};
		}

		if (handles.indices_buffer == 0 || handles.view_data_buffer == 0 || handles.view_stats_buffer == 0)
		{
			std::cerr << "View-data dispatch requires initialized GPU buffers\n";
			return false;
		}

		if (input_layout != 0 && input_layout != 1)
		{
			std::cerr << "Unsupported input layout for view-data dispatch: " << input_layout << "\n";
			return false;
		}

		if (input_layout == 0)
		{
			if (handles.splat_buffer == 0)
			{
				std::cerr << "Packed input layout requires packed splat buffer\n";
				return false;
			}
		}
		else
		{
			if (handles.position_buffer == 0 || handles.other_buffer == 0 || handles.color_buffer == 0 || handles.sh_buffer == 0)
			{
				std::cerr << "Split input layout requires split section buffers\n";
				return false;
			}
		}

		if (splat_count == 0)
		{
			return true;
		}

		if (splat_count > static_cast<std::size_t>(std::numeric_limits<GLuint>::max()))
		{
			std::cerr << "Splat count exceeds dispatch range\n";
			return false;
		}

		if (chunk_count > static_cast<std::size_t>(std::numeric_limits<GLuint>::max()))
		{
			std::cerr << "Chunk count exceeds dispatch range\n";
			return false;
		}

		m_program.use();
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform2f(m_viewport_size_loc, viewport_width, viewport_height);
		glUniform1f(m_max_point_size_loc, max_point_size);
		glUniform1i(m_active_domain_preculled_loc, active_domain_preculled ? 1 : 0);
		glUniform1i(m_use_schedule_domain_loc, use_schedule_domain ? 1 : 0);
		glUniform1i(m_write_compacted_view_data_loc, write_compacted_view_data ? 1 : 0);
		glUniform1i(m_schedule_entries_sorted_loc, schedule_entries_sorted ? 1 : 0);
		glUniform1i(m_use_sorted_schedule_lookup_loc, use_sorted_schedule_lookup ? 1 : 0);
		glUniform1i(m_use_anisotropic_loc, use_anisotropic ? 1 : 0);
		glUniform3f(m_camera_pos_loc, camera_pos.x, camera_pos.y, camera_pos.z);
		glUniform1i(m_sh_degree_loc, sh_degree);
		glUniform1ui(m_chunk_count_loc, static_cast<GLuint>(chunk_count));
		glUniform1ui(m_schedule_entry_count_loc, static_cast<GLuint>(schedule_entry_count));
		glUniform1ui(m_real_count_loc, static_cast<GLuint>(splat_count));
		glUniform1i(m_input_layout_loc, input_layout);

		const ViewDataDebugStats zero_stats{};
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.view_stats_buffer);
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER,
			0,
			static_cast<GLsizeiptr>(sizeof(ViewDataDebugStats)),
			&zero_stats);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, handles.indices_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, handles.view_data_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, handles.chunk_schedule_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, handles.chunk_schedule_sort_indices_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, handles.view_stats_buffer);
		if (input_layout == 0)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, handles.splat_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);
		}
		else
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, handles.position_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, handles.other_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, handles.color_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, handles.sh_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, handles.chunk_buffer);
		}

		const GLuint groups = static_cast<GLuint>((splat_count + (kComputeWorkGroupSize - 1)) / kComputeWorkGroupSize);
		glDispatchCompute(groups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		if (out_stats != nullptr)
		{
			ViewDataDebugStats raw_stats{};
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.view_stats_buffer);
			glGetBufferSubData(
				GL_SHADER_STORAGE_BUFFER,
				0,
				static_cast<GLsizeiptr>(sizeof(ViewDataDebugStats)),
				&raw_stats);
			out_stats->processed_splats = raw_stats.processed_splats;
			out_stats->chunk_tests = raw_stats.chunk_tests;
			out_stats->chunk_culled_splats = raw_stats.chunk_culled_splats;
		}

		const GLenum dispatch_error = glGetError();
		if (dispatch_error != GL_NO_ERROR)
		{
			std::cerr << "ViewDataPipeline dispatch GL error: " << dispatch_error << "\n";
			return false;
		}

		return true;
	}

} // namespace gs
