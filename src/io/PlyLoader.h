#pragma once

#include <filesystem>

#include "scene/GaussianModel.h"

namespace gs
{

	// PLY加载器：从高斯点云PLY文件解析并填充GaussianModel
	class PlyLoader
	{
	public:
		// 读取指定路径PLY并输出到outModel，成功返回true
		static bool loadGaussianPly(const std::filesystem::path& path, GaussianModel& outModel);
	};

} // namespace gs
