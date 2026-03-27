#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <glad/glad.h>

#include "runtime/RuntimeSplatAsset.h"
#include "render/GpuSplatLayout.h"

namespace gs
{

	struct GpuUploadBufferHandles
	{
		GLuint splat_buffer{ 0 };
		GLuint position_buffer{ 0 };
		GLuint other_buffer{ 0 };
		GLuint color_buffer{ 0 };
		GLuint sh_buffer{ 0 };
		GLuint chunk_buffer{ 0 };
		GLuint chunk_schedule_buffer{ 0 };
		GLuint chunk_scheduler_stats_buffer{ 0 };
		GLuint view_stats_buffer{ 0 };
		GLuint keys_buffer{ 0 };
		GLuint indices_buffer{ 0 };
		GLuint view_data_buffer{ 0 };
	};

	struct ChunkScheduleEntry
	{
		std::uint32_t chunk_index{ 0 };
		std::uint32_t output_offset{ 0 };
		std::uint32_t splat_count{ 0 };
		std::uint32_t reserved{ 0 };
	};
	static_assert(sizeof(ChunkScheduleEntry) == 16, "Unexpected ChunkScheduleEntry size");

	struct ChunkSchedulerStats
	{
		std::uint32_t visible_chunk_count{ 0 };
		std::uint32_t active_splat_count{ 0 };
		std::uint32_t schedule_entry_count{ 0 };
		std::uint32_t overflow_flag{ 0 };
	};
	static_assert(sizeof(ChunkSchedulerStats) == 16, "Unexpected ChunkSchedulerStats size");

	struct ViewDataDebugStats
	{
		std::uint32_t processed_splats{ 0 };
		std::uint32_t chunk_tests{ 0 };
		std::uint32_t chunk_culled_splats{ 0 };
		std::uint32_t reserved{ 0 }; // Padding for 16-byte std430 alignment / future stats expansion.
	};
	static_assert(sizeof(ViewDataDebugStats) == 16, "Unexpected ViewDataDebugStats size");

	struct GpuUploadStats
	{
		std::size_t splat_count{ 0 };
		std::size_t sort_count{ 0 };
		std::size_t chunk_count{ 0 };
		int max_supported_sh_degree{ 0 };
	};

	class GpuUploadBuffers
	{
	public:
		static bool create(GpuUploadBufferHandles& out_handles);
		static void destroy(GpuUploadBufferHandles& handles);
		static bool upload_split_splats(
			const GpuUploadBufferHandles& handles,
			const RuntimeSplatAsset& asset,
			GpuUploadStats& out_stats);
	};

} // namespace gs
