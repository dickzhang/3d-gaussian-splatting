#pragma once

#include <cstddef>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>

#include "core/ShaderProgram.h"
#include "render/GpuUploadBuffers.h"

namespace gs
{

	class ChunkSchedulerPipeline
	{
	public:
		struct DispatchStats
		{
			std::uint32_t visible_chunk_count{ 0 };
			std::uint32_t active_splat_count{ 0 };
			std::uint32_t schedule_entry_count{ 0 };
			bool overflowed{ false };
		};

		bool initialize();
		bool dispatch(
			const GpuUploadBufferHandles& handles,
			const glm::mat4& view,
			const glm::mat4& model,
			const glm::mat4& projection,
			std::size_t chunk_count,
			std::size_t sort_capacity,
			DispatchStats* out_stats = nullptr);

	private:
		ShaderProgram m_program;
		GLint m_view_loc{ -1 };
		GLint m_model_loc{ -1 };
		GLint m_proj_loc{ -1 };
		GLint m_chunk_count_loc{ -1 };
		GLint m_sort_capacity_loc{ -1 };
		GLint m_schedule_capacity_loc{ -1 };
	};

} // namespace gs