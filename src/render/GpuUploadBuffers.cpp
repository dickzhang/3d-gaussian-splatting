#include "render/GpuUploadBuffers.h"

#include <algorithm>
#include <iostream>
#include <limits>

namespace gs
{
	namespace
	{

		std::size_t next_pow2(std::size_t value)
		{
			if (value <= 1)
			{
				return 1;
			}

			if (value > (std::numeric_limits<std::size_t>::max() >> 1))
			{
				return std::numeric_limits<std::size_t>::max() >> 1;
			}

			std::size_t p = 1;
			while (p < value && p <= (std::numeric_limits<std::size_t>::max() >> 1))
			{
				p <<= 1;
			}

			if (p < value)
			{
				return std::numeric_limits<std::size_t>::max() >> 1;
			}

			return p;
		}

		std::vector<std::uint32_t> make_initial_sort_keys(std::size_t padded_count)
		{
			return std::vector<std::uint32_t>(padded_count, 0xFFFFFFFFu);
		}

		std::vector<std::uint32_t> make_initial_sort_indices(std::size_t real_count, std::size_t padded_count)
		{
			std::vector<std::uint32_t> indices(padded_count, 0xFFFFFFFFu);
			for (std::size_t index = 0; index < real_count; ++index)
			{
				indices[index] = static_cast<std::uint32_t>(index);
			}
			return indices;
		}

	} // namespace

	bool GpuUploadBuffers::create(GpuUploadBufferHandles& out_handles)
	{
		out_handles = GpuUploadBufferHandles{};
		glGenBuffers(1, &out_handles.splat_buffer);
		glGenBuffers(1, &out_handles.position_buffer);
		glGenBuffers(1, &out_handles.other_buffer);
		glGenBuffers(1, &out_handles.color_buffer);
		glGenBuffers(1, &out_handles.sh_buffer);
		glGenBuffers(1, &out_handles.chunk_buffer);
		glGenBuffers(1, &out_handles.chunk_schedule_buffer);
		glGenBuffers(1, &out_handles.chunk_scheduler_stats_buffer);
		glGenBuffers(1, &out_handles.view_stats_buffer);
		glGenBuffers(1, &out_handles.keys_buffer);
		glGenBuffers(1, &out_handles.indices_buffer);
		glGenBuffers(1, &out_handles.view_data_buffer);

		if (out_handles.splat_buffer == 0 ||
			out_handles.position_buffer == 0 ||
			out_handles.other_buffer == 0 ||
			out_handles.color_buffer == 0 ||
			out_handles.sh_buffer == 0 ||
			out_handles.chunk_buffer == 0 ||
			out_handles.chunk_schedule_buffer == 0 ||
			out_handles.chunk_scheduler_stats_buffer == 0 ||
			out_handles.view_stats_buffer == 0 ||
			out_handles.keys_buffer == 0 ||
			out_handles.indices_buffer == 0 ||
			out_handles.view_data_buffer == 0)
		{
			destroy(out_handles);
			return false;
		}

		return true;
	}

	void GpuUploadBuffers::destroy(GpuUploadBufferHandles& handles)
	{
		if (handles.splat_buffer != 0)
		{
			glDeleteBuffers(1, &handles.splat_buffer);
			handles.splat_buffer = 0;
		}
		if (handles.position_buffer != 0)
		{
			glDeleteBuffers(1, &handles.position_buffer);
			handles.position_buffer = 0;
		}
		if (handles.other_buffer != 0)
		{
			glDeleteBuffers(1, &handles.other_buffer);
			handles.other_buffer = 0;
		}
		if (handles.color_buffer != 0)
		{
			glDeleteBuffers(1, &handles.color_buffer);
			handles.color_buffer = 0;
		}
		if (handles.sh_buffer != 0)
		{
			glDeleteBuffers(1, &handles.sh_buffer);
			handles.sh_buffer = 0;
		}
		if (handles.chunk_buffer != 0)
		{
			glDeleteBuffers(1, &handles.chunk_buffer);
			handles.chunk_buffer = 0;
		}
		if (handles.chunk_schedule_buffer != 0)
		{
			glDeleteBuffers(1, &handles.chunk_schedule_buffer);
			handles.chunk_schedule_buffer = 0;
		}
		if (handles.chunk_scheduler_stats_buffer != 0)
		{
			glDeleteBuffers(1, &handles.chunk_scheduler_stats_buffer);
			handles.chunk_scheduler_stats_buffer = 0;
		}
		if (handles.view_stats_buffer != 0)
		{
			glDeleteBuffers(1, &handles.view_stats_buffer);
			handles.view_stats_buffer = 0;
		}
		if (handles.keys_buffer != 0)
		{
			glDeleteBuffers(1, &handles.keys_buffer);
			handles.keys_buffer = 0;
		}
		if (handles.indices_buffer != 0)
		{
			glDeleteBuffers(1, &handles.indices_buffer);
			handles.indices_buffer = 0;
		}
		if (handles.view_data_buffer != 0)
		{
			glDeleteBuffers(1, &handles.view_data_buffer);
			handles.view_data_buffer = 0;
		}
	}

	bool GpuUploadBuffers::upload_split_splats(
		const GpuUploadBufferHandles& handles,
		const RuntimeSplatAsset& asset,
		GpuUploadStats& out_stats)
	{
		out_stats = GpuUploadStats{};
		if (!asset.has_split_sections())
		{
			std::cerr << "Split upload requires position/other/color/sh sections\n";
			return false;
		}

		if (handles.position_buffer == 0 ||
			handles.other_buffer == 0 ||
			handles.color_buffer == 0 ||
			handles.sh_buffer == 0 ||
			handles.chunk_buffer == 0 ||
			handles.chunk_schedule_buffer == 0 ||
			handles.chunk_scheduler_stats_buffer == 0 ||
			handles.view_stats_buffer == 0 ||
			handles.keys_buffer == 0 ||
			handles.indices_buffer == 0 ||
			handles.view_data_buffer == 0)
		{
			std::cerr << "GPU split upload buffers are not initialized\n";
			return false;
		}

		const std::size_t new_splat_count = asset.positions.size();
		if (new_splat_count == 0)
		{
			std::cerr << "Cannot upload empty split asset\n";
			return false;
		}
		if (asset.splat_count != new_splat_count)
		{
			std::cerr << "Split asset count mismatch: header " << asset.splat_count
				<< ", sections " << new_splat_count << "\n";
			return false;
		}
		if (asset.others.size() != new_splat_count || asset.colors.size() != new_splat_count || asset.sh_data.size() != new_splat_count)
		{
			std::cerr << "Split asset sections are inconsistent in length\n";
			return false;
		}

		const std::size_t new_sort_count = next_pow2(new_splat_count);
		if (new_sort_count > static_cast<std::size_t>(std::numeric_limits<GLuint>::max()))
		{
			std::cerr << "Model too large for current GPU sorting path\n";
			return false;
		}

		const std::vector<std::uint32_t> init_keys = make_initial_sort_keys(new_sort_count);
		const std::vector<std::uint32_t> init_indices = make_initial_sort_indices(new_splat_count, new_sort_count);

		if (asset.positions.size() > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) / sizeof(SplatCachePositionEntry) ||
			asset.others.size() > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) / sizeof(SplatCacheOtherEntry) ||
			asset.colors.size() > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) / sizeof(SplatCacheColorEntry) ||
			asset.sh_data.size() > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) / sizeof(SplatCacheShEntry) ||
			asset.chunks.size() > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) / sizeof(SplatCacheChunkEntry) ||
			asset.chunks.size() > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) / sizeof(ChunkScheduleEntry) ||
			new_splat_count > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) / sizeof(GPUViewSplat) ||
			init_keys.size() > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) / sizeof(std::uint32_t) ||
			init_indices.size() > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) / sizeof(std::uint32_t))
		{
			std::cerr << "Split upload buffer size exceeds GLsizeiptr range\n";
			return false;
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.position_buffer);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			static_cast<GLsizeiptr>(asset.positions.size() * sizeof(SplatCachePositionEntry)),
			asset.positions.data(),
			GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.other_buffer);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			static_cast<GLsizeiptr>(asset.others.size() * sizeof(SplatCacheOtherEntry)),
			asset.others.data(),
			GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.color_buffer);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			static_cast<GLsizeiptr>(asset.colors.size() * sizeof(SplatCacheColorEntry)),
			asset.colors.data(),
			GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.sh_buffer);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			static_cast<GLsizeiptr>(asset.sh_data.size() * sizeof(SplatCacheShEntry)),
			asset.sh_data.data(),
			GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.chunk_buffer);
		if (asset.chunks.empty())
		{
			glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STATIC_DRAW);
		}
		else
		{
			glBufferData(
				GL_SHADER_STORAGE_BUFFER,
				static_cast<GLsizeiptr>(asset.chunks.size() * sizeof(SplatCacheChunkEntry)),
				asset.chunks.data(),
				GL_STATIC_DRAW);
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.keys_buffer);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			static_cast<GLsizeiptr>(init_keys.size() * sizeof(std::uint32_t)),
			init_keys.data(),
			GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.indices_buffer);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			static_cast<GLsizeiptr>(init_indices.size() * sizeof(std::uint32_t)),
			init_indices.data(),
			GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.view_data_buffer);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			static_cast<GLsizeiptr>(new_splat_count * sizeof(GPUViewSplat)),
			nullptr,
			GL_DYNAMIC_DRAW);

		const GLsizeiptr scheduleBufferSize = asset.chunks.empty()
			? 0
			: static_cast<GLsizeiptr>(asset.chunks.size() * sizeof(ChunkScheduleEntry));
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.chunk_schedule_buffer);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			scheduleBufferSize,
			nullptr,
			GL_DYNAMIC_DRAW);

		const ChunkSchedulerStats emptySchedulerStats{};
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.chunk_scheduler_stats_buffer);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			static_cast<GLsizeiptr>(sizeof(ChunkSchedulerStats)),
			&emptySchedulerStats,
			GL_DYNAMIC_DRAW);

		const ViewDataDebugStats empty_stats{};
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handles.view_stats_buffer);
		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			static_cast<GLsizeiptr>(sizeof(ViewDataDebugStats)),
			&empty_stats,
			GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		const GLenum upload_err = glGetError();
		if (upload_err != GL_NO_ERROR)
		{
			std::cerr << "Failed to upload split model buffers, GL error: " << upload_err << "\n";
			return false;
		}

		out_stats.splat_count = new_splat_count;
		out_stats.sort_count = new_sort_count;
		out_stats.chunk_count = asset.chunks.size();
		out_stats.max_supported_sh_degree = std::max(0, std::min(kMaxShDegree, asset.max_sh_degree));
		return true;
	}

} // namespace gs
