#pragma once

#include <filesystem>

#include "import/GaussianScene.h"

namespace gs
{

	class PlySceneImporter
	{
	public:
		static bool import_scene(const std::filesystem::path& path, GaussianScene& out_scene);
	};

} // namespace gs
