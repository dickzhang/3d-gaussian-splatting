#pragma once

#include <filesystem>

#include "scene/GaussianModel.h"

namespace gs
{

	// PLY 加载器：从高斯点云 PLY 文件读取数据并填充 GaussianModel
	class PlyLoader
	{
	public:
		// 读取指定路径的 PLY，成功时写入 outModel 并返回 true
		static bool loadGaussianPly(const std::filesystem::path& path, GaussianModel& outModel);
	};

} // namespace gs
