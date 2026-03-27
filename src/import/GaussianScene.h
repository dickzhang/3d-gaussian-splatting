#pragma once

#include <vector>

#include "scene/GaussianModel.h"

namespace gs
{

	// Import-stage canonical scene container. This remains high precision and is not tied to runtime GPU packing.
	struct GaussianScene
	{
		std::vector<GaussianSplat> splats;
		int max_sh_degree{ 0 };
	};

} // namespace gs
