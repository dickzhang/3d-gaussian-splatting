#pragma once

#include <filesystem>
#include <string>

#include "runtime/RuntimeSplatAsset.h"

namespace gs
{

	class SplatCacheReader
	{
	public:
		static bool read_cache_bundle(const std::filesystem::path& input_path, RuntimeSplatAsset& out_asset, std::string* out_error = nullptr);
	};

} // namespace gs
