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

	// 视图数据预处理管线：在绘制前生成与当前相机相关的 GPUViewSplat 数据。
	class ViewDataPipeline
	{
	public:
		// 初始化 compute shader 与 uniform 位置。
		bool initialize();
		// 根据当前视图生成每个 splat 的屏幕空间表示。
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
			bool write_compacted_view_data,
			bool schedule_entries_sorted,
			bool use_sorted_schedule_lookup,
			bool use_anisotropic,
			int sh_degree,
			std::size_t chunk_count,
			std::size_t schedule_entry_count,
			std::size_t splat_count);

	private:
		ShaderProgram m_program; // view-data compute 程序
		GLint m_view_loc{ -1 }; // u_view 位置
		GLint m_model_loc{ -1 }; // u_model 位置
		GLint m_proj_loc{ -1 }; // u_proj 位置
		GLint m_viewport_size_loc{ -1 }; // u_viewportSize 位置
		GLint m_max_point_size_loc{ -1 }; // u_maxPointSize 位置
		GLint m_active_domain_preculled_loc{ -1 }; // u_activeDomainPreculled 位置
		GLint m_use_schedule_domain_loc{ -1 }; // u_useScheduleDomain 位置
		GLint m_write_compacted_view_data_loc{ -1 }; // u_writeCompactedViewData 位置
		GLint m_schedule_entries_sorted_loc{ -1 }; // u_scheduleEntriesSorted 位置
		GLint m_use_sorted_schedule_lookup_loc{ -1 }; // u_useSortedScheduleLookup 位置
		GLint m_use_anisotropic_loc{ -1 }; // u_useAnisotropic 位置
		GLint m_camera_pos_loc{ -1 }; // u_cameraPos 位置
		GLint m_sh_degree_loc{ -1 }; // u_shDegree 位置
		GLint m_chunk_count_loc{ -1 }; // u_chunkCount 位置
		GLint m_schedule_entry_count_loc{ -1 }; // u_scheduleEntryCount 位置
		GLint m_real_count_loc{ -1 }; // u_realCount 位置
		GLint m_input_layout_loc{ -1 }; // u_inputLayout 位置
	};

} // namespace gs
