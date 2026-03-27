#pragma once

#include <array>
#include <cstdint>

namespace gs
{

	inline constexpr int kPackedShWordCount = 24;
	inline constexpr int kMaxShDegree = 3;

	// Shared GPU-side splat layout. This is the canonical runtime payload format.
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

	// Per-instance view data produced by view_data.comp and consumed by gaussian.vert.
	// Keep field order and scalar widths aligned with GLSL GPUViewSplat (std430).
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
