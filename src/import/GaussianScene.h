#pragma once

#include <vector>

#include "scene/GaussianModel.h"

namespace gs
{

	// 导入阶段的标准场景容器，保留高精度数据，不绑定运行时 GPU 打包格式。
	struct GaussianScene
	{
		std::vector<GaussianSplat> splats; // 原始高精度 splat 列表
		int max_sh_degree{ 0 }; // 场景支持的最大 SH 阶数
	};

} // namespace gs
