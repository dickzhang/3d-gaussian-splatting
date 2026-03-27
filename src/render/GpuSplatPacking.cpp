#include "render/GpuSplatPacking.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace gs
{
	namespace
	{

		std::uint16_t float_to_half_bits(float value)
		{
			if (std::isnan(value))
			{
				return static_cast<std::uint16_t>(0x7FFFu);
			}

			if (std::isinf(value))
			{
				return static_cast<std::uint16_t>(value < 0.0f ? 0xFC00u : 0x7C00u);
			}

			union
			{
				float f;
				std::uint32_t u;
			} value_bits{ value };

			const std::uint32_t sign = (value_bits.u >> 16) & 0x8000u;
			std::uint32_t mantissa = value_bits.u & 0x007FFFFFu;
			int exponent = static_cast<int>((value_bits.u >> 23) & 0xFFu) - 127 + 15;

			if (exponent <= 0)
			{
				if (exponent < -10)
				{
					return static_cast<std::uint16_t>(sign);
				}
				mantissa = (mantissa | 0x00800000u) >> (1 - exponent);
				std::uint32_t rounded = (mantissa + 0x00001000u) >> 13;
				rounded = std::min(rounded, 0x03FFu);
				return static_cast<std::uint16_t>(sign | rounded);
			}

			if (exponent >= 31)
			{
				return static_cast<std::uint16_t>(sign | 0x7C00u);
			}

			std::uint32_t rounded = (mantissa + 0x00001000u) >> 13;
			if (rounded > 0x03FFu)
			{
				rounded = 0;
				++exponent;
				if (exponent >= 31)
				{
					return static_cast<std::uint16_t>(sign | 0x7C00u);
				}
			}

			return static_cast<std::uint16_t>(sign | (static_cast<std::uint32_t>(exponent) << 10) | rounded);
		}

		std::uint32_t pack_half2x16(float first, float second)
		{
			const std::uint16_t half_first = float_to_half_bits(first);
			const std::uint16_t half_second = float_to_half_bits(second);
			return static_cast<std::uint32_t>(half_first) | (static_cast<std::uint32_t>(half_second) << 16);
		}

	} // namespace

	std::vector<GPUSplat> pack_gpu_splats(const GaussianModel& model)
	{
		std::vector<GPUSplat> packed_splats;
		packed_splats.reserve(model.size());

		for (const auto& splat : model.splats)
		{
			GPUSplat packed{};
			packed.px = splat.position.x;
			packed.py = splat.position.y;
			packed.pz = splat.position.z;
			packed.opacity = splat.opacity;

			packed.sx = splat.scale.x;
			packed.sy = splat.scale.y;
			packed.sz = splat.scale.z;
			packed.pad0 = 0.0f;

			packed.rx = splat.rotation.x;
			packed.ry = splat.rotation.y;
			packed.rz = splat.rotation.z;
			packed.rw = splat.rotation.w;

			packed.cr = splat.color.r;
			packed.cg = splat.color.g;
			packed.cb = splat.color.b;
			packed.radius = std::max({ splat.scale.x, splat.scale.y, splat.scale.z });

			const std::array<float, 48> sh_values{
				splat.sh1_0.x,
				splat.sh1_0.y,
				splat.sh1_0.z,
				splat.sh1_1.x,
				splat.sh1_1.y,
				splat.sh1_1.z,
				splat.sh1_2.x,
				splat.sh1_2.y,
				splat.sh1_2.z,
				splat.sh2_0.x,
				splat.sh2_0.y,
				splat.sh2_0.z,
				splat.sh2_1.x,
				splat.sh2_1.y,
				splat.sh2_1.z,
				splat.sh2_2.x,
				splat.sh2_2.y,
				splat.sh2_2.z,
				splat.sh2_3.x,
				splat.sh2_3.y,
				splat.sh2_3.z,
				splat.sh2_4.x,
				splat.sh2_4.y,
				splat.sh2_4.z,
				splat.sh3_0.x,
				splat.sh3_0.y,
				splat.sh3_0.z,
				splat.sh3_1.x,
				splat.sh3_1.y,
				splat.sh3_1.z,
				splat.sh3_2.x,
				splat.sh3_2.y,
				splat.sh3_2.z,
				splat.sh3_3.x,
				splat.sh3_3.y,
				splat.sh3_3.z,
				splat.sh3_4.x,
				splat.sh3_4.y,
				splat.sh3_4.z,
				splat.sh3_5.x,
				splat.sh3_5.y,
				splat.sh3_5.z,
				splat.sh3_6.x,
				splat.sh3_6.y,
				splat.sh3_6.z,
				0.0f,
				0.0f,
				0.0f,
			};
			static_assert(std::tuple_size<decltype(sh_values)>::value == 48, "Expected 48 SH float values");

			for (std::size_t index = 0; index < kPackedShWordCount; ++index)
			{
				const std::size_t first = index * 2;
				packed.shPacked[index] = pack_half2x16(sh_values[first], sh_values[first + 1]);
			}

			packed_splats.push_back(packed);
		}

		return packed_splats;
	}

	std::vector<std::uint32_t> make_initial_sort_keys(std::size_t padded_count)
	{
		return std::vector<std::uint32_t>(padded_count, 0xFFFFFFFFu);
	}

	std::vector<std::uint32_t> make_initial_sort_indices(std::size_t real_count, std::size_t padded_count)
	{
		std::vector<std::uint32_t> indices(padded_count, 0xFFFFFFFFu);
		for (std::size_t index = 0; index < real_count; ++index)
		{
			indices[index] = static_cast<std::uint32_t>(index);
		}
		return indices;
	}

} // namespace gs
