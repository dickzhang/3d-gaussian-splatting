#pragma once

#include <cstddef>
#include <cstdint>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "core/ShaderProgram.h"
#include "render/GpuUploadBuffers.h"

namespace gs
{

	// Pre-compute per-frame view-dependent splat data before the draw pass.
	// This pass transforms splats into clip/view space, evaluates screen-space
	// footprint data, performs chunk-based coarse culling, and writes compact
	// GPUViewSplat records for the vertex shader to consume.
	class ViewDataPipeline
	{
	public:
		struct DispatchStats
		{
			std::uint32_t processed_splats{ 0 }; // Valid sorted splats processed by view_data.comp.
			std::uint32_t chunk_tests{ 0 }; // Splats whose owning chunk was tested for coarse culling.
			std::uint32_t chunk_culled_splats{ 0 }; // Splats rejected because their chunk failed the coarse frustum test.
		};

		bool initialize();
		bool dispatch(
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
			bool use_anisotropic,
			int sh_degree,
			std::size_t chunk_count,
			std::size_t schedule_entry_count,
			std::size_t splat_count,
			DispatchStats* out_stats = nullptr);

	private:
		ShaderProgram m_program;
		GLint m_view_loc{ -1 };
		GLint m_model_loc{ -1 };
		GLint m_proj_loc{ -1 };
		GLint m_viewport_size_loc{ -1 };
		GLint m_max_point_size_loc{ -1 };
		GLint m_active_domain_preculled_loc{ -1 };
		GLint m_use_schedule_domain_loc{ -1 };
		GLint m_use_anisotropic_loc{ -1 };
		GLint m_camera_pos_loc{ -1 };
		GLint m_sh_degree_loc{ -1 };
		GLint m_chunk_count_loc{ -1 };
		GLint m_schedule_entry_count_loc{ -1 };
		GLint m_real_count_loc{ -1 };
		GLint m_input_layout_loc{ -1 };
	};

} // namespace gs
