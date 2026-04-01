#pragma once

#include <filesystem>
#include <string>

#include "runtime/RuntimeSplatAsset.h"

namespace gs
{

	// cache 读取器：从清单和 payload 文件恢复运行时资产。
	class SplatCacheReader
	{
	public:
		// 读取完整 cache bundle，并填充 RuntimeSplatAsset。
		static bool read_cache_bundle(const std::filesystem::path& input_path, RuntimeSplatAsset& out_asset, std::string* out_error = nullptr);
	};

} // namespace gs
