#pragma once

#include <cstdint>
#include <filesystem>

#include "import/GaussianScene.h"

namespace gs
{
	// cache 写入选项：控制 chunk 构建和打包粒度。
	struct SplatCacheWriteOptions
	{
		bool include_chunk{ true }; // 是否写出 chunk 元数据
		std::uint32_t chunk_size{ 2048u }; // 每个 chunk 的目标 splat 数量
	};

	// cache 写入器：将导入场景序列化为 manifest + payload 文件集合。
	class SplatCacheWriter
	{
	public:
		// 将 GaussianScene 写出为可供运行时直接加载的 cache bundle。
		static bool write_cache_bundle(
			const GaussianScene& scene,
			const std::filesystem::path& manifest_path,
			const SplatCacheWriteOptions& options = {});
	};

} // namespace gs
