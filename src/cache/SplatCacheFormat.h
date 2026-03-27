#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace gs
{

	inline constexpr std::uint32_t kSplatCacheMagic = 0x544C5053u;
	inline constexpr std::uint32_t kSplatCacheVersion = 4u;
	inline constexpr std::size_t kSplatCacheMaxPayloadEntries = 8;
	inline constexpr std::size_t kSplatCachePayloadPathChars = 192;

	enum class SplatCacheSectionType : std::uint32_t
	{
		Chunk = 1,
		Position = 2,
		Other = 3,
		Color = 4,
		Sh = 5,
	};

	struct SplatCachePayloadEntry
	{
		std::uint32_t type;
		std::uint32_t reserved;
		std::uint64_t size;
		std::uint64_t checksum;
		std::array<char, kSplatCachePayloadPathChars> relative_path;
	};

		struct SplatCachePositionEntry
		{
			float px;
			float py;
			float pz;
			float opacity;
		};
		static_assert(sizeof(SplatCachePositionEntry) == 16, "Unexpected SplatCachePositionEntry size");

		struct SplatCacheOtherEntry
		{
			float sx;
			float sy;
			float sz;
			float pad0;
			float rx;
			float ry;
			float rz;
			float rw;
		};
		static_assert(sizeof(SplatCacheOtherEntry) == 32, "Unexpected SplatCacheOtherEntry size");

		struct SplatCacheColorEntry
		{
			float cr;
			float cg;
			float cb;
			float radius;
		};
		static_assert(sizeof(SplatCacheColorEntry) == 16, "Unexpected SplatCacheColorEntry size");

		struct SplatCacheShEntry
		{
			std::uint32_t packed[24];
		};
		static_assert(sizeof(SplatCacheShEntry) == 96, "Unexpected SplatCacheShEntry size");

		struct SplatCacheChunkEntry
		{
			float center_x;
			float center_y;
			float center_z;
			float radius;
			std::uint32_t start_index;
			std::uint32_t splat_count;
			std::uint32_t reserved0;
			std::uint32_t reserved1;
		};
		static_assert(sizeof(SplatCacheChunkEntry) == 32, "Unexpected SplatCacheChunkEntry size");

	struct SplatCacheManifestHeader
	{
		std::uint32_t magic;
		std::uint32_t version;
		std::uint32_t payload_count;
		std::uint32_t splat_count;
		std::uint32_t max_sh_degree;
		std::uint32_t reserved0;
		std::array<float, 3> bounds_min;
		float reserved1;
		std::array<float, 3> bounds_max;
		std::uint32_t flags;
	};

	inline constexpr std::uint32_t kManifestFlagHasChunk = 1u << 0;

} // namespace gs
