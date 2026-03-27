#pragma once

#include <filesystem>
#include <string>

#include "runtime/RuntimeSplatAsset.h"

namespace gs
{

	class RuntimeSplatAssetLoader
	{
	public:
		static bool load_from_cache(const std::filesystem::path& input_path, RuntimeSplatAsset& out_asset, std::string* out_error = nullptr);
	};

} // namespace gs
