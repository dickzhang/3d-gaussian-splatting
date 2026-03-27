#include "import/PlySceneImporter.h"

#include "io/PlyLoader.h"

namespace gs
{

	bool PlySceneImporter::import_scene(const std::filesystem::path& path, GaussianScene& out_scene)
	{
		GaussianModel model;
		if (!PlyLoader::loadGaussianPly(path, model))
		{
			return false;
		}

		out_scene.splats = model.splats;
		out_scene.max_sh_degree = model.maxShDegree();
		return true;
	}

} // namespace gs
