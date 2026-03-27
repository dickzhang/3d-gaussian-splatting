#include "cache/SplatCacheWriter.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <cstring>
#include <limits>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/common.hpp>

#include <glm/geometric.hpp>

#include "cache/SplatCacheFormat.h"
#include "render/GpuSplatPacking.h"

namespace gs
{
	namespace
	{
		constexpr std::uint64_t kFnvOffsetBasis = 1469598103934665603ull;
		constexpr std::uint64_t kFnvPrime = 1099511628211ull;

		template <typename T>
		bool byte_size_for_elements(std::size_t count, std::uint64_t& out_size)
		{
			constexpr std::uint64_t element_size = static_cast<std::uint64_t>(sizeof(T));
			if (static_cast<std::uint64_t>(count) > (std::numeric_limits<std::uint64_t>::max() / element_size))
			{
				return false;
			}

			out_size = static_cast<std::uint64_t>(count) * element_size;
			return true;
		}

		template <typename T>
		bool write_vector(std::ofstream& stream, const std::vector<T>& values)
		{
			if (values.empty())
			{
				return true;
			}

			std::uint64_t byte_count = 0;
			if (!byte_size_for_elements<T>(values.size(), byte_count))
			{
				return false;
			}

			if (byte_count > static_cast<std::uint64_t>(std::numeric_limits<std::streamsize>::max()))
			{
				return false;
			}

			stream.write(reinterpret_cast<const char*>(values.data()), static_cast<std::streamsize>(byte_count));
			return stream.good();
		}

		template <typename T>
		std::uint64_t fnv1a_vector_checksum(const std::vector<T>& values)
		{
			std::uint64_t hash = kFnvOffsetBasis;
			const auto* bytes = reinterpret_cast<const std::uint8_t*>(values.data());
			const std::size_t total_bytes = values.size() * sizeof(T);
			for (std::size_t i = 0; i < total_bytes; ++i)
			{
				hash ^= static_cast<std::uint64_t>(bytes[i]);
				hash *= kFnvPrime;
			}
			return hash;
		}

		std::filesystem::path payload_path_for(
			const std::filesystem::path& manifest_path,
			const std::string& suffix)
		{
			return manifest_path.parent_path() / (manifest_path.stem().string() + suffix);
		}

		bool fill_relative_path(
			const std::filesystem::path& manifest_path,
			const std::filesystem::path& payload_path,
			std::array<char, kSplatCachePayloadPathChars>& out_path)
		{
			out_path.fill('\0');
			std::error_code ec;
			const std::filesystem::path relative = std::filesystem::relative(payload_path, manifest_path.parent_path(), ec);
			const std::string text = ec ? payload_path.filename().string() : relative.generic_string();
			if (text.empty() || text.size() >= out_path.size())
			{
				return false;
			}

			std::memcpy(out_path.data(), text.data(), text.size());
			out_path[text.size()] = '\0';
			return true;
		}

		template <typename T>
		bool write_payload_file(const std::filesystem::path& payload_path, const std::vector<T>& values)
		{
			std::ofstream stream(payload_path, std::ios::binary);
			if (!stream.is_open())
			{
				return false;
			}

			if (!write_vector(stream, values))
			{
				return false;
			}

			return stream.good();
		}

		bool compute_bounds(const GaussianScene& scene, std::array<float, 3>& out_min, std::array<float, 3>& out_max)
		{
			if (scene.splats.empty())
			{
				return false;
			}

			glm::vec3 min_value(std::numeric_limits<float>::infinity());
			glm::vec3 max_value(-std::numeric_limits<float>::infinity());
			for (const auto& splat : scene.splats)
			{
				min_value = glm::min(min_value, splat.position);
				max_value = glm::max(max_value, splat.position);
			}

			out_min = { min_value.x, min_value.y, min_value.z };
			out_max = { max_value.x, max_value.y, max_value.z };
			return true;
		}

		bool build_chunk_entries(
			const GaussianScene& scene,
			std::uint32_t chunk_size,
			std::vector<SplatCacheChunkEntry>& out_chunks)
		{
			if (scene.splats.empty())
			{
				out_chunks.clear();
				return true;
			}

			if (chunk_size == 0)
			{
				return false;
			}

			const std::size_t splat_count = scene.splats.size();
			const std::size_t chunk_count = (splat_count + static_cast<std::size_t>(chunk_size) - 1u) / static_cast<std::size_t>(chunk_size);
			if (chunk_count > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()))
			{
				return false;
			}

			out_chunks.clear();
			out_chunks.reserve(chunk_count);

			for (std::size_t chunk_index = 0; chunk_index < chunk_count; ++chunk_index)
			{
				const std::size_t start_index = chunk_index * static_cast<std::size_t>(chunk_size);
				const std::size_t end_index = std::min(splat_count, start_index + static_cast<std::size_t>(chunk_size));
				if (start_index >= end_index)
				{
					return false;
				}

				glm::vec3 min_value(std::numeric_limits<float>::infinity());
				glm::vec3 max_value(-std::numeric_limits<float>::infinity());
				for (std::size_t splat_index = start_index; splat_index < end_index; ++splat_index)
				{
					const auto& splat = scene.splats[splat_index];
					min_value = glm::min(min_value, splat.position);
					max_value = glm::max(max_value, splat.position);
				}

				const glm::vec3 center = 0.5f * (min_value + max_value);
				float radius = 0.0f;
				for (std::size_t splat_index = start_index; splat_index < end_index; ++splat_index)
				{
					const auto& splat = scene.splats[splat_index];
					const float splat_extent = std::max({
						splat.scale.x,
						splat.scale.y,
						splat.scale.z,
						0.0f });
					const float candidate = glm::length(splat.position - center) + splat_extent;
					radius = std::max(radius, candidate);
				}

				const std::size_t count = end_index - start_index;
				if (start_index > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()) ||
					count > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()))
				{
					return false;
				}

				SplatCacheChunkEntry chunk{};
				chunk.center_x = center.x;
				chunk.center_y = center.y;
				chunk.center_z = center.z;
				chunk.radius = radius;
				chunk.start_index = static_cast<std::uint32_t>(start_index);
				chunk.splat_count = static_cast<std::uint32_t>(count);
				out_chunks.push_back(chunk);
			}

			return true;
		}

	} // namespace

	bool SplatCacheWriter::write_cache_bundle(
		const GaussianScene& scene,
		const std::filesystem::path& manifest_path,
		const SplatCacheWriteOptions& options)
	{
		if (scene.splats.empty())
		{
			return false;
		}

		std::error_code dir_ec;
		const std::filesystem::path manifest_parent = manifest_path.parent_path();
		if (!manifest_parent.empty())
		{
			std::filesystem::create_directories(manifest_parent, dir_ec);
			if (dir_ec)
			{
				return false;
			}
		}

		GaussianModel model;
		model.splats = scene.splats;
		model.setMaxShDegree(scene.max_sh_degree);
		const std::vector<GPUSplat> packed_splats = pack_gpu_splats(model);
		const std::size_t splat_count = packed_splats.size();
		if (splat_count > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()))
		{
			return false;
		}

		std::vector<SplatCachePositionEntry> positions;
		std::vector<SplatCacheOtherEntry> others;
		std::vector<SplatCacheColorEntry> colors;
		std::vector<SplatCacheShEntry> sh_data;
		positions.reserve(splat_count);
		others.reserve(splat_count);
		colors.reserve(splat_count);
		sh_data.reserve(splat_count);

		for (const auto& splat : packed_splats)
		{
			SplatCachePositionEntry position{};
			position.px = splat.px;
			position.py = splat.py;
			position.pz = splat.pz;
			position.opacity = splat.opacity;
			positions.push_back(position);

			SplatCacheOtherEntry other{};
			other.sx = splat.sx;
			other.sy = splat.sy;
			other.sz = splat.sz;
			other.pad0 = splat.pad0;
			other.rx = splat.rx;
			other.ry = splat.ry;
			other.rz = splat.rz;
			other.rw = splat.rw;
			others.push_back(other);

			SplatCacheColorEntry color{};
			color.cr = splat.cr;
			color.cg = splat.cg;
			color.cb = splat.cb;
			color.radius = splat.radius;
			colors.push_back(color);

			SplatCacheShEntry sh{};
			for (int word_index = 0; word_index < kPackedShWordCount; ++word_index)
			{
				sh.packed[word_index] = splat.shPacked[word_index];
			}
			sh_data.push_back(sh);
		}

		std::vector<SplatCacheChunkEntry> chunks;
		if (options.include_chunk && !build_chunk_entries(scene, options.chunk_size, chunks))
		{
			return false;
		}

		const std::filesystem::path pos_path = payload_path_for(manifest_path, ".pos.byte");
		const std::filesystem::path other_path = payload_path_for(manifest_path, ".other.byte");
		const std::filesystem::path color_path = payload_path_for(manifest_path, ".color.byte");
		const std::filesystem::path sh_path = payload_path_for(manifest_path, ".sh.byte");
		const std::filesystem::path chunk_path = payload_path_for(manifest_path, ".chunk.byte");

		if (!write_payload_file(pos_path, positions) ||
			!write_payload_file(other_path, others) ||
			!write_payload_file(color_path, colors) ||
			!write_payload_file(sh_path, sh_data))
		{
			return false;
		}

		if (options.include_chunk)
		{
			if (!write_payload_file(chunk_path, chunks))
			{
				return false;
			}
		}
		else
		{
			std::error_code remove_ec;
			std::filesystem::remove(chunk_path, remove_ec);
			if (remove_ec)
			{
				return false;
			}
		}

		SplatCacheManifestHeader manifest{};
		manifest.magic = kSplatCacheMagic;
		manifest.version = kSplatCacheVersion;
		manifest.payload_count = options.include_chunk ? 5u : 4u;
		manifest.splat_count = static_cast<std::uint32_t>(splat_count);
		manifest.max_sh_degree = static_cast<std::uint32_t>(std::clamp(scene.max_sh_degree, 0, kMaxShDegree));
		manifest.flags = options.include_chunk ? kManifestFlagHasChunk : 0u;
		if (!compute_bounds(scene, manifest.bounds_min, manifest.bounds_max))
		{
			return false;
		}

		std::array<SplatCachePayloadEntry, kSplatCacheMaxPayloadEntries> entries{};
		auto fill_entry = [&](std::size_t index, SplatCacheSectionType type, const std::filesystem::path& path, std::uint64_t size, std::uint64_t checksum) -> bool
		{
			entries[index].type = static_cast<std::uint32_t>(type);
			entries[index].size = size;
			entries[index].checksum = checksum;
			return fill_relative_path(manifest_path, path, entries[index].relative_path);
		};

		std::uint64_t pos_size = 0;
		std::uint64_t other_size = 0;
		std::uint64_t color_size = 0;
		std::uint64_t sh_size = 0;
		std::uint64_t chunk_size = 0;
		if (!byte_size_for_elements<SplatCachePositionEntry>(positions.size(), pos_size) ||
			!byte_size_for_elements<SplatCacheOtherEntry>(others.size(), other_size) ||
			!byte_size_for_elements<SplatCacheColorEntry>(colors.size(), color_size) ||
			!byte_size_for_elements<SplatCacheShEntry>(sh_data.size(), sh_size))
		{
			return false;
		}

		if (options.include_chunk && !byte_size_for_elements<SplatCacheChunkEntry>(chunks.size(), chunk_size))
		{
			return false;
		}

		if (!fill_entry(0, SplatCacheSectionType::Position, pos_path, pos_size, fnv1a_vector_checksum(positions)) ||
			!fill_entry(1, SplatCacheSectionType::Other, other_path, other_size, fnv1a_vector_checksum(others)) ||
			!fill_entry(2, SplatCacheSectionType::Color, color_path, color_size, fnv1a_vector_checksum(colors)) ||
			!fill_entry(3, SplatCacheSectionType::Sh, sh_path, sh_size, fnv1a_vector_checksum(sh_data)))
		{
			return false;
		}

		if (options.include_chunk &&
			!fill_entry(4, SplatCacheSectionType::Chunk, chunk_path, chunk_size, fnv1a_vector_checksum(chunks)))
		{
			return false;
		}

		std::ofstream manifest_stream(manifest_path, std::ios::binary);
		if (!manifest_stream.is_open())
		{
			return false;
		}

		manifest_stream.write(reinterpret_cast<const char*>(&manifest), sizeof(manifest));
		manifest_stream.write(reinterpret_cast<const char*>(entries.data()),
			static_cast<std::streamsize>(sizeof(SplatCachePayloadEntry) * entries.size()));
		return manifest_stream.good();
	}

} // namespace gs
