#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "render/GpuSplatLayout.h"
#include "scene/GaussianModel.h"

namespace gs
{

	// 将高精度场景 splat 打包为运行时 GPU 布局。
	std::vector<GPUSplat> pack_gpu_splats(const GaussianModel& model);
	// 生成排序键缓冲的初始内容，空槽位写入无效键。
	std::vector<std::uint32_t> make_initial_sort_keys(std::size_t padded_count);
	// 生成排序索引缓冲的初始内容，真实 splat 顺序写入，剩余槽位填无效值。
	std::vector<std::uint32_t> make_initial_sort_indices(std::size_t real_count, std::size_t padded_count);

} // namespace gs
