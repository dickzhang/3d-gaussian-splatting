#pragma once

#include <filesystem>

#include "import/GaussianScene.h"

namespace gs
{

	// PLY 场景导入器：将训练/导出阶段的 PLY 资产转换为 GaussianScene。
	class PlySceneImporter
	{
	public:
		// 从指定 PLY 文件导入高精度 GaussianScene。
		static bool import_scene(const std::filesystem::path& path, GaussianScene& out_scene);
	};

} // namespace gs
