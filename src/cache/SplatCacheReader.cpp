#include "cache/SplatCacheReader.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "cache/SplatCacheFormat.h"
#include "core/PathUtils.h"
#include "render/GpuSplatLayout.h"

namespace gs
{
	namespace
	{
		constexpr std::uint32_t kLegacyManifestVersion = 3u;
		constexpr std::uint64_t kFnvOffsetBasis = 1469598103934665603ull;
		constexpr std::uint64_t kFnvPrime = 1099511628211ull;

		struct LegacyChunkEntry
		{
			std::uint32_t start_index;
			std::uint32_t splat_count;
		};
		static_assert(sizeof(LegacyChunkEntry) == 8, "Unexpected legacy chunk size");

		bool fail(std::string* out_error, std::string message)
		{
			if (out_error != nullptr)
			{
				*out_error = std::move(message);
			}
			return false;
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
		bool read_payload_file(
			const std::filesystem::path& payload_path,
			std::size_t expected_count,
			std::vector<T>& out_values,
			std::string* out_error,
			const char* payload_label)
		{
			std::ifstream stream(payload_path, std::ios::binary);
			if (!stream.is_open())
			{
				return fail(out_error, std::string("Missing ") + payload_label + " payload file: " + pathToUtf8(payload_path));
			}

			stream.seekg(0, std::ios::end);
			const std::streamoff file_size_stream = stream.tellg();
			if (file_size_stream < 0)
			{
				return fail(out_error, std::string("Failed to read ") + payload_label + " payload size: " + pathToUtf8(payload_path));
			}

			std::uint64_t expected_size = 0;
			if (!byte_size_for_elements<T>(expected_count, expected_size))
			{
				return fail(out_error, std::string("Invalid expected ") + payload_label + " payload size");
			}

			if (static_cast<std::uint64_t>(file_size_stream) != expected_size)
			{
				return fail(
					out_error,
					std::string("Size mismatch for ") + payload_label + " payload: expected " +
					std::to_string(expected_size) + " bytes, got " +
					std::to_string(static_cast<std::uint64_t>(file_size_stream)) +
					" bytes at " + pathToUtf8(payload_path));
			}

			if (expected_size > static_cast<std::uint64_t>(std::numeric_limits<std::streamsize>::max()))
			{
				return fail(out_error, std::string("Payload too large to read for ") + payload_label + ": " + pathToUtf8(payload_path));
			}

			out_values.resize(expected_count);
			stream.seekg(0, std::ios::beg);
			stream.read(reinterpret_cast<char*>(out_values.data()), static_cast<std::streamsize>(expected_size));
			if (!stream.good())
			{
				return fail(out_error, std::string("Failed to read ") + payload_label + " payload bytes: " + pathToUtf8(payload_path));
			}

			return true;
		}

		bool read_relative_path_text(const SplatCachePayloadEntry& entry, std::string& out_text)
		{
			auto it = std::find(entry.relative_path.begin(), entry.relative_path.end(), '\0');
			if (it == entry.relative_path.end() || it == entry.relative_path.begin())
			{
				return false;
			}

			out_text.assign(entry.relative_path.begin(), it);
			return true;
		}

		bool is_path_under_directory(
			const std::filesystem::path& candidate,
			const std::filesystem::path& directory)
		{
			std::error_code ec_base;
			const std::filesystem::path canonical_base = std::filesystem::weakly_canonical(directory, ec_base);
			if (ec_base)
			{
				return false;
			}

			std::error_code ec_candidate;
			const std::filesystem::path canonical_candidate = std::filesystem::weakly_canonical(candidate, ec_candidate);
			if (ec_candidate)
			{
				return false;
			}

			auto base_it = canonical_base.begin();
			auto cand_it = canonical_candidate.begin();
			for (; base_it != canonical_base.end(); ++base_it, ++cand_it)
			{
				if (cand_it == canonical_candidate.end() || *base_it != *cand_it)
				{
					return false;
				}
			}

			return true;
		}

		const char* payload_label_for_type(SplatCacheSectionType type)
		{
			switch (type)
			{
			case SplatCacheSectionType::Chunk: return "chunk";
			case SplatCacheSectionType::Position: return "position";
			case SplatCacheSectionType::Other: return "other";
			case SplatCacheSectionType::Color: return "color";
			case SplatCacheSectionType::Sh: return "sh";
			default: return "unknown";
			}
		}

		template <typename T>
		bool validate_chunk_coverage(
			const std::vector<T>& chunks,
			std::size_t splat_count,
			std::string* out_error)
		{
			if (chunks.empty())
			{
				return true;
			}

			std::uint64_t covered = 0;
			for (std::size_t chunk_index = 0; chunk_index < chunks.size(); ++chunk_index)
			{
				const auto& chunk = chunks[chunk_index];
				if (chunk.splat_count == 0)
				{
					return fail(out_error, "Chunk payload contains an empty chunk range");
				}

				if (static_cast<std::uint64_t>(chunk.start_index) != covered)
				{
					return fail(out_error, "Chunk payload ranges are not contiguous from zero");
				}

				covered += static_cast<std::uint64_t>(chunk.splat_count);
				if (covered > static_cast<std::uint64_t>(splat_count))
				{
					return fail(out_error, "Chunk payload ranges exceed the cache splat count");
				}
			}

			if (covered != static_cast<std::uint64_t>(splat_count))
			{
				return fail(out_error, "Chunk payload ranges do not cover the full cache splat count");
			}

			return true;
		}

		bool validate_chunk_metadata(
			const std::vector<SplatCacheChunkEntry>& chunks,
			std::size_t splat_count,
			std::string* out_error)
		{
			if (!validate_chunk_coverage(chunks, splat_count, out_error))
			{
				return false;
			}

			for (const auto& chunk : chunks)
			{
				if (!(chunk.radius >= 0.0f))
				{
					return fail(out_error, "Chunk payload contains a negative radius");
				}
			}

			return true;
		}

		std::vector<SplatCacheChunkEntry> convert_legacy_chunks(
			const std::vector<LegacyChunkEntry>& legacy_chunks,
			const RuntimeSplatAsset& asset)
		{
			std::vector<SplatCacheChunkEntry> chunks;
			chunks.reserve(legacy_chunks.size());

			const float center_x = 0.5f * (asset.bounds_min[0] + asset.bounds_max[0]);
			const float center_y = 0.5f * (asset.bounds_min[1] + asset.bounds_max[1]);
			const float center_z = 0.5f * (asset.bounds_min[2] + asset.bounds_max[2]);
			const float extent_x = asset.bounds_max[0] - center_x;
			const float extent_y = asset.bounds_max[1] - center_y;
			const float extent_z = asset.bounds_max[2] - center_z;
			const float radius = std::sqrt(extent_x * extent_x + extent_y * extent_y + extent_z * extent_z);

			for (const auto& legacy_chunk : legacy_chunks)
			{
				SplatCacheChunkEntry chunk{};
				chunk.center_x = center_x;
				chunk.center_y = center_y;
				chunk.center_z = center_z;
				chunk.radius = radius;
				chunk.start_index = legacy_chunk.start_index;
				chunk.splat_count = legacy_chunk.splat_count;
				chunks.push_back(chunk);
			}

			return chunks;
		}

		bool read_manifest(const std::filesystem::path& input_path, RuntimeSplatAsset& out_asset, std::string* out_error)
		{
			std::ifstream stream(input_path, std::ios::binary);
			if (!stream.is_open())
			{
				return fail(out_error, "Failed to open cache manifest: " + pathToUtf8(input_path));
			}

			SplatCacheManifestHeader manifest{};
			stream.read(reinterpret_cast<char*>(&manifest), sizeof(manifest));
			if (!stream.good())
			{
				return fail(out_error, "Failed to read cache manifest header: " + pathToUtf8(input_path));
			}

			if (manifest.magic != kSplatCacheMagic)
			{
				return fail(out_error, "Invalid cache manifest magic in: " + pathToUtf8(input_path));
			}

			if (manifest.version != kLegacyManifestVersion && manifest.version != kSplatCacheVersion)
			{
				return fail(
					out_error,
					"Unsupported cache manifest version " + std::to_string(manifest.version) +
					", expected " + std::to_string(kLegacyManifestVersion) +
					" or " + std::to_string(kSplatCacheVersion));
			}

			if (manifest.payload_count == 0 || manifest.payload_count > kSplatCacheMaxPayloadEntries)
			{
				return fail(out_error, "Invalid payload_count in cache manifest: " + std::to_string(manifest.payload_count));
			}

			std::array<SplatCachePayloadEntry, kSplatCacheMaxPayloadEntries> entries{};
			stream.read(reinterpret_cast<char*>(entries.data()),
				static_cast<std::streamsize>(sizeof(SplatCachePayloadEntry) * entries.size()));
			if (!stream.good())
			{
				return fail(out_error, "Failed to read cache payload table: " + pathToUtf8(input_path));
			}

			const std::size_t splat_count = static_cast<std::size_t>(manifest.splat_count);
			const std::filesystem::path base_dir = input_path.parent_path();

			const SplatCachePayloadEntry* pos_entry = nullptr;
			const SplatCachePayloadEntry* other_entry = nullptr;
			const SplatCachePayloadEntry* color_entry = nullptr;
			const SplatCachePayloadEntry* sh_entry = nullptr;
			const SplatCachePayloadEntry* chunk_entry = nullptr;

			for (std::uint32_t i = 0; i < manifest.payload_count; ++i)
			{
				const auto& entry = entries[i];
				const auto type = static_cast<SplatCacheSectionType>(entry.type);
				switch (type)
				{
				case SplatCacheSectionType::Position:
					if (pos_entry != nullptr)
					{
						return fail(out_error, "Duplicate position payload entry in manifest");
					}
					pos_entry = &entry;
					break;
				case SplatCacheSectionType::Other:
					if (other_entry != nullptr)
					{
						return fail(out_error, "Duplicate other payload entry in manifest");
					}
					other_entry = &entry;
					break;
				case SplatCacheSectionType::Color:
					if (color_entry != nullptr)
					{
						return fail(out_error, "Duplicate color payload entry in manifest");
					}
					color_entry = &entry;
					break;
				case SplatCacheSectionType::Sh:
					if (sh_entry != nullptr)
					{
						return fail(out_error, "Duplicate sh payload entry in manifest");
					}
					sh_entry = &entry;
					break;
				case SplatCacheSectionType::Chunk:
					if (chunk_entry != nullptr)
					{
						return fail(out_error, "Duplicate chunk payload entry in manifest");
					}
					chunk_entry = &entry;
					break;
				default:
					return fail(out_error, "Unknown payload entry type in manifest: " + std::to_string(entry.type));
				}
			}

			if (pos_entry == nullptr || other_entry == nullptr || color_entry == nullptr || sh_entry == nullptr)
			{
				return fail(out_error, "Cache manifest is missing one or more required payload entries");
			}

			const bool manifest_has_chunk = (manifest.flags & kManifestFlagHasChunk) != 0;
			if (manifest_has_chunk != (chunk_entry != nullptr))
			{
				return fail(out_error, "Cache manifest chunk flag does not match chunk payload table");
			}

			auto entry_path = [&](const SplatCachePayloadEntry& entry, std::filesystem::path& out_path, const char* payload_label) -> bool
			{
				std::string rel_text;
				if (!read_relative_path_text(entry, rel_text))
				{
					return fail(out_error, std::string("Missing relative path for ") + payload_label + " payload entry");
				}

				const std::filesystem::path rel_path = std::filesystem::path(rel_text);
				if (rel_path.is_absolute())
				{
					return fail(out_error, std::string("Absolute path is not allowed for ") + payload_label + " payload: " + rel_text);
				}

				const std::filesystem::path resolved = (base_dir / rel_path).lexically_normal();
				if (!is_path_under_directory(resolved, base_dir))
				{
					return fail(out_error, std::string("Payload path escapes cache directory for ") + payload_label + ": " + rel_text);
				}

				out_path = resolved;
				return true;
			};

			std::filesystem::path pos_path;
			std::filesystem::path other_path;
			std::filesystem::path color_path;
			std::filesystem::path sh_path;
			if (!entry_path(*pos_entry, pos_path, "position") ||
				!entry_path(*other_entry, other_path, "other") ||
				!entry_path(*color_entry, color_path, "color") ||
				!entry_path(*sh_entry, sh_path, "sh"))
			{
				return false;
			}

			if (!read_payload_file(pos_path, splat_count, out_asset.positions, out_error, "position") ||
				!read_payload_file(other_path, splat_count, out_asset.others, out_error, "other") ||
				!read_payload_file(color_path, splat_count, out_asset.colors, out_error, "color") ||
				!read_payload_file(sh_path, splat_count, out_asset.sh_data, out_error, "sh"))
			{
				return false;
			}

			if (chunk_entry != nullptr)
			{
				std::filesystem::path chunk_path;
				if (!entry_path(*chunk_entry, chunk_path, "chunk"))
				{
					return false;
				}

				if (manifest.version == kSplatCacheVersion)
				{
					if (chunk_entry->size % sizeof(SplatCacheChunkEntry) != 0)
					{
						return fail(out_error, "Chunk payload size is not aligned to chunk entry size");
					}
					const std::size_t chunk_count = static_cast<std::size_t>(chunk_entry->size / sizeof(SplatCacheChunkEntry));
					if (!read_payload_file(chunk_path, chunk_count, out_asset.chunks, out_error, "chunk"))
					{
						return false;
					}
				}
				else
				{
					if (chunk_entry->size % sizeof(LegacyChunkEntry) != 0)
					{
						return fail(out_error, "Legacy chunk payload size is not aligned to legacy chunk entry size");
					}
					const std::size_t legacy_chunk_count = static_cast<std::size_t>(chunk_entry->size / sizeof(LegacyChunkEntry));
					std::vector<LegacyChunkEntry> legacy_chunks;
					if (!read_payload_file(chunk_path, legacy_chunk_count, legacy_chunks, out_error, "chunk"))
					{
						return false;
					}
					if (!validate_chunk_coverage(legacy_chunks, splat_count, out_error))
					{
						return false;
					}
					out_asset.chunks = convert_legacy_chunks(legacy_chunks, out_asset);
				}
			}

			std::uint64_t expected_pos = 0;
			std::uint64_t expected_other = 0;
			std::uint64_t expected_color = 0;
			std::uint64_t expected_sh = 0;
			if (!byte_size_for_elements<SplatCachePositionEntry>(out_asset.positions.size(), expected_pos) ||
				!byte_size_for_elements<SplatCacheOtherEntry>(out_asset.others.size(), expected_other) ||
				!byte_size_for_elements<SplatCacheColorEntry>(out_asset.colors.size(), expected_color) ||
				!byte_size_for_elements<SplatCacheShEntry>(out_asset.sh_data.size(), expected_sh))
			{
				return fail(out_error, "Failed to compute expected payload sizes");
			}

			if (pos_entry->size != expected_pos || other_entry->size != expected_other ||
				color_entry->size != expected_color || sh_entry->size != expected_sh)
			{
				return fail(out_error, "Manifest payload byte sizes do not match loaded payload data");
			}

			if (pos_entry->checksum != fnv1a_vector_checksum(out_asset.positions) ||
				other_entry->checksum != fnv1a_vector_checksum(out_asset.others) ||
				color_entry->checksum != fnv1a_vector_checksum(out_asset.colors) ||
				sh_entry->checksum != fnv1a_vector_checksum(out_asset.sh_data))
			{
				return fail(out_error, "Payload checksum mismatch detected in cache bundle");
			}

			if (chunk_entry != nullptr)
			{
				std::uint64_t expected_chunk = 0;
				if (manifest.version == kSplatCacheVersion)
				{
					if (!byte_size_for_elements<SplatCacheChunkEntry>(out_asset.chunks.size(), expected_chunk))
					{
						return fail(out_error, "Failed to compute expected chunk payload size");
					}
					if (chunk_entry->size != expected_chunk ||
						chunk_entry->checksum != fnv1a_vector_checksum(out_asset.chunks))
					{
						return fail(out_error, "Chunk payload metadata does not match loaded chunk payload data");
					}
				}
				else
				{
					std::vector<LegacyChunkEntry> legacy_chunks;
					legacy_chunks.reserve(out_asset.chunks.size());
					for (const auto& chunk : out_asset.chunks)
					{
						legacy_chunks.push_back(LegacyChunkEntry{ chunk.start_index, chunk.splat_count });
					}
					if (!byte_size_for_elements<LegacyChunkEntry>(legacy_chunks.size(), expected_chunk))
					{
						return fail(out_error, "Failed to compute expected legacy chunk payload size");
					}
					if (chunk_entry->size != expected_chunk ||
						chunk_entry->checksum != fnv1a_vector_checksum(legacy_chunks))
					{
						return fail(out_error, "Legacy chunk payload metadata does not match loaded chunk payload data");
					}
				}

				if (!validate_chunk_metadata(out_asset.chunks, splat_count, out_error))
				{
					return false;
				}
			}

			for (int axis = 0; axis < 3; ++axis)
			{
				if (manifest.bounds_min[axis] > manifest.bounds_max[axis])
				{
					return fail(out_error, "Invalid cache bounds in manifest");
				}
			}

			out_asset.splat_count = splat_count;
			out_asset.max_sh_degree = static_cast<int>(std::clamp(manifest.max_sh_degree, 0u, static_cast<std::uint32_t>(kMaxShDegree)));
			out_asset.bounds_min = manifest.bounds_min;
			out_asset.bounds_max = manifest.bounds_max;
			if (out_error != nullptr)
			{
				out_error->clear();
			}
			return true;
		}


	} // namespace

	bool SplatCacheReader::read_cache_bundle(const std::filesystem::path& input_path, RuntimeSplatAsset& out_asset, std::string* out_error)
	{
		out_asset = RuntimeSplatAsset{};
		if (out_error != nullptr)
		{
			out_error->clear();
		}
		return read_manifest(input_path, out_asset, out_error);
	}

} // namespace gs
