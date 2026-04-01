#pragma once

#include <filesystem>
#include <string>

#include "runtime/RuntimeSplatAsset.h"

namespace gs
{

	// 运行时资产加载器：从磁盘 cache 恢复可直接上传到 GPU 的 split-section 数据。
	class RuntimeSplatAssetLoader
	{
	public:
		// 从 .gsplatcache 清单读取运行时资产。
		static bool load_from_cache(const std::filesystem::path& input_path, RuntimeSplatAsset& out_asset, std::string* out_error = nullptr);
	};

} // namespace gs
