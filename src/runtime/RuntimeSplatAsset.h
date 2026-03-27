#pragma once

#include <cstddef>
#include <array>
#include <vector>

#include "cache/SplatCacheFormat.h"

namespace gs
{

	struct RuntimeSplatAsset
	{
		std::vector<SplatCachePositionEntry> positions;
		std::vector<SplatCacheOtherEntry> others;
		std::vector<SplatCacheColorEntry> colors;
		std::vector<SplatCacheShEntry> sh_data;
		std::vector<SplatCacheChunkEntry> chunks;
		std::size_t splat_count{ 0 };
		int max_sh_degree{ 0 };
		std::array<float, 3> bounds_min{ 0.0f, 0.0f, 0.0f };
		std::array<float, 3> bounds_max{ 0.0f, 0.0f, 0.0f };

		bool has_split_sections() const noexcept
		{
			return !positions.empty() &&
				!others.empty() &&
				!colors.empty() &&
				!sh_data.empty() &&
				positions.size() == others.size() &&
				positions.size() == colors.size() &&
				positions.size() == sh_data.size();
		}
	};

} // namespace gs
