#pragma once

#include <cstdint>
#include <filesystem>

#include "import/GaussianScene.h"

namespace gs
{
	struct SplatCacheWriteOptions
	{
		bool include_chunk{ true };
		std::uint32_t chunk_size{ 2048u };
	};

	class SplatCacheWriter
	{
	public:
		static bool write_cache_bundle(
			const GaussianScene& scene,
			const std::filesystem::path& manifest_path,
			const SplatCacheWriteOptions& options = {});
	};

} // namespace gs
