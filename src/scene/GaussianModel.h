#pragma once

#include <cstddef>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace gs
{

	// 单个高斯元数据：位置、形状、颜色与 SH 系数
	struct GaussianSplat
	{
		glm::vec3 position{ 0.0f };              // 世界位置
		glm::vec3 scale{ 0.01f };                // 各向尺度
		glm::vec4 rotation{ 0.0f, 0.0f, 0.0f, 1.0f }; // 旋转四元数
		float opacity{ 1.0f };                   // 不透明度
		glm::vec3 color{ 1.0f };                 // 基础颜色（DC）
		glm::vec3 sh1_0{ 0.0f };                 // 一阶 SH 系数 0
		glm::vec3 sh1_1{ 0.0f };                 // 一阶 SH 系数 1
		glm::vec3 sh1_2{ 0.0f };                 // 一阶 SH 系数 2
		glm::vec3 sh2_0{ 0.0f };                 // 二阶 SH 系数 0
		glm::vec3 sh2_1{ 0.0f };                 // 二阶 SH 系数 1
		glm::vec3 sh2_2{ 0.0f };                 // 二阶 SH 系数 2
		glm::vec3 sh2_3{ 0.0f };                 // 二阶 SH 系数 3
		glm::vec3 sh2_4{ 0.0f };                 // 二阶 SH 系数 4
		glm::vec3 sh3_0{ 0.0f };                 // 三阶 SH 系数 0
		glm::vec3 sh3_1{ 0.0f };                 // 三阶 SH 系数 1
		glm::vec3 sh3_2{ 0.0f };                 // 三阶 SH 系数 2
		glm::vec3 sh3_3{ 0.0f };                 // 三阶 SH 系数 3
		glm::vec3 sh3_4{ 0.0f };                 // 三阶 SH 系数 4
		glm::vec3 sh3_5{ 0.0f };                 // 三阶 SH 系数 5
		glm::vec3 sh3_6{ 0.0f };                 // 三阶 SH 系数 6
	};

	// 高斯模型：保存所有高斯元，并记录模型支持的最大 SH 阶数
	class GaussianModel
	{
	public:
		std::vector<GaussianSplat> splats; // 全部高斯元

		// 返回元素数量
		std::size_t size() const noexcept
		{
			return splats.size();
		}
		// 是否为空模型
		bool empty() const noexcept
		{
			return splats.empty();
		}
		// 设置模型支持的最大 SH 阶，范围限制在 0~3
		void setMaxShDegree(int degree) noexcept
		{
			if (degree < 0)
			{
				m_maxShDegree = 0;
			}
			else if (degree > 3)
			{
				m_maxShDegree = 3;
			}
			else
			{
				m_maxShDegree = degree;
			}
		}
		// 获取模型支持的最大 SH 阶
		int maxShDegree() const noexcept
		{
			return m_maxShDegree;
		}
	private:
		int m_maxShDegree{ 0 }; // 模型最大 SH 阶数
	};
} // namespace gs
