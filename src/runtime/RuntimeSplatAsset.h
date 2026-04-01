#pragma once

#include <cstddef>
#include <array>
#include <vector>

#include "cache/SplatCacheFormat.h"

namespace gs
{

	// 运行时 cache 资产：以 split-section 形式持有上传到 GPU 的各类数据块。
	struct RuntimeSplatAsset
	{
		std::vector<SplatCachePositionEntry> positions; // 位置与不透明度段
		std::vector<SplatCacheOtherEntry> others; // 缩放与旋转段
		std::vector<SplatCacheColorEntry> colors; // 颜色与半径段
		std::vector<SplatCacheShEntry> sh_data; // 球谐系数段
		std::vector<SplatCacheChunkEntry> chunks; // chunk 粗裁剪元数据段
		std::size_t splat_count{ 0 }; // 资产中的 splat 总数
		int max_sh_degree{ 0 }; // 资产支持的最大 SH 阶数
		std::array<float, 3> bounds_min{ 0.0f, 0.0f, 0.0f }; // 场景包围盒最小点
		std::array<float, 3> bounds_max{ 0.0f, 0.0f, 0.0f }; // 场景包围盒最大点

		// 判断 split-section 数据是否完整且长度一致。
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
