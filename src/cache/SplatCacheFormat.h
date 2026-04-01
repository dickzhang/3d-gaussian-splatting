#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace gs
{

	// cache 文件魔数，用于识别自定义清单格式。
	inline constexpr std::uint32_t kSplatCacheMagic = 0x544C5053u;
	// 当前 cache 清单格式版本号。
	inline constexpr std::uint32_t kSplatCacheVersion = 4u;
	// 单个 manifest 中最多支持的 payload 段数量。
	inline constexpr std::size_t kSplatCacheMaxPayloadEntries = 8;
	// manifest 中相对 payload 路径字段的固定字符容量。
	inline constexpr std::size_t kSplatCachePayloadPathChars = 192;

	// cache payload 段类型枚举。
	enum class SplatCacheSectionType : std::uint32_t
	{
		Chunk = 1,
		Position = 2,
		Other = 3,
		Color = 4,
		Sh = 5,
	};

	// manifest 中描述单个 payload 文件的信息。
	struct SplatCachePayloadEntry
	{
		std::uint32_t type;
		std::uint32_t reserved;
		std::uint64_t size;
		std::uint64_t checksum;
		std::array<char, kSplatCachePayloadPathChars> relative_path;
	};

	// 位置段条目：位置与不透明度。
	struct SplatCachePositionEntry
	{
		float px;
		float py;
		float pz;
		float opacity;
	};
	static_assert(sizeof(SplatCachePositionEntry) == 16, "Unexpected SplatCachePositionEntry size");

	// 其他属性段条目：缩放与旋转。
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

	// 颜色段条目：颜色与基础半径。
	struct SplatCacheColorEntry
	{
		float cr;
		float cg;
		float cb;
		float radius;
	};
	static_assert(sizeof(SplatCacheColorEntry) == 16, "Unexpected SplatCacheColorEntry size");

	// SH 段条目：打包后的球谐系数。
	struct SplatCacheShEntry
	{
		std::uint32_t packed[24];
	};
	static_assert(sizeof(SplatCacheShEntry) == 96, "Unexpected SplatCacheShEntry size");

	// chunk 段条目：粗裁剪球与对应的 splat 连续区间。
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

	// cache manifest 头，描述整体 payload 布局和场景范围。
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

	// manifest 标志位：当前资产包含 chunk 元数据段。
	inline constexpr std::uint32_t kManifestFlagHasChunk = 1u << 0;

} // namespace gs
