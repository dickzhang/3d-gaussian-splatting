#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "render/GpuSplatLayout.h"
#include "scene/GaussianModel.h"

namespace gs
{

	std::vector<GPUSplat> pack_gpu_splats(const GaussianModel& model);
	std::vector<std::uint32_t> make_initial_sort_keys(std::size_t padded_count);
	std::vector<std::uint32_t> make_initial_sort_indices(std::size_t real_count, std::size_t padded_count);

} // namespace gs
