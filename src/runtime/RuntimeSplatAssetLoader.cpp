#include "runtime/RuntimeSplatAssetLoader.h"

#include "cache/SplatCacheReader.h"

namespace gs
{

	bool RuntimeSplatAssetLoader::load_from_cache(const std::filesystem::path& input_path, RuntimeSplatAsset& out_asset, std::string* out_error)
	{
		return SplatCacheReader::read_cache_bundle(input_path, out_asset, out_error);
	}

} // namespace gs
