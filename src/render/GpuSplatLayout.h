#pragma once

#include <array>
#include <cstdint>

namespace gs
{

	// 打包后 SH 系数字数，和 GPU/缓存格式保持一致。
	inline constexpr int kPackedShWordCount = 24;
	// 当前运行时支持的最大 SH 阶数。
	inline constexpr int kMaxShDegree = 3;

	// GPU 侧标准 splat 布局，也是运行时上传的基础载荷格式。
	struct GPUSplat
	{
		float px;
		float py;
		float pz;
		float opacity;

		float sx;
		float sy;
		float sz;
		float pad0;

		float rx;
		float ry;
		float rz;
		float rw;

		float cr;
		float cg;
		float cb;
		float radius;

		std::uint32_t shPacked[kPackedShWordCount];
	};

	static_assert(sizeof(GPUSplat) == 160, "GPUSplat must match std430 array stride");

	// view_data.comp 生成、gaussian.vert 消费的视图空间中间数据布局。
	struct GPUViewSplat
	{
		float clip_x;
		float clip_y;
		float clip_z;
		float clip_w;

		float color_r;
		float color_g;
		float color_b;
		float opacity;

		float inv_cov_x;
		float inv_cov_y;
		float inv_cov_z;
		float half_extent_px;

		float basis_major_x;
		float basis_major_y;
		float basis_minor_x;
		float basis_minor_y;
	};

	static_assert(sizeof(GPUViewSplat) == 64, "GPUViewSplat must match std430 array stride");

} // namespace gs
