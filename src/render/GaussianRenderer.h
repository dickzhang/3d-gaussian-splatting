#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <glm/mat4x4.hpp>

#include "core/ShaderProgram.h"
#include "scene/GaussianModel.h"

namespace gs
{

	// 高斯渲染器：管理 GPU 资源并执行高斯元绘制
	class GaussianRenderer
	{
	public:
		GaussianRenderer() = default;
		~GaussianRenderer();

		// 禁止拷贝，避免 OpenGL 资源重复释放
		GaussianRenderer(const GaussianRenderer&) = delete;
		GaussianRenderer& operator=(const GaussianRenderer&) = delete;

		// 初始化着色器与 GPU 侧资源
		bool initialize();
		// 上传模型数据到 GPU
		bool uploadModel(const GaussianModel& model);
		// 执行一帧渲染
		void render(const glm::mat4& view, const glm::mat4& projection, float viewportWidth, float viewportHeight);
		// 设置是否启用各向异性高斯
		void setUseAnisotropic(bool enabled);
		// 查询各向异性开关
		bool useAnisotropic() const noexcept;
		// 设置 SH 阶数，按模型能力自动裁剪
		int setShDegree(int degree);
		// 获取当前 SH 阶数
		int shDegree() const noexcept;
		// 获取模型支持的最大 SH 阶数
		int maxSupportedShDegree() const noexcept;
		// 设置模型到世界的变换矩阵
		void setModelTransform(const glm::mat4& model) noexcept;
		// 获取当前模型变换矩阵
		const glm::mat4& modelTransform() const noexcept;

	private:
		// GPU 高斯元布局，需与 shader 的 std430 结构严格一致
		struct GPUSplat
		{
			float px;      // 位置 x
			float py;      // 位置 y
			float pz;      // 位置 z
			float opacity; // 不透明度

			float sx;    // 尺度 x
			float sy;    // 尺度 y
			float sz;    // 尺度 z
			float _pad0; // 对齐填充

			float rx; // 旋转四元数 x
			float ry; // 旋转四元数 y
			float rz; // 旋转四元数 z
			float rw; // 旋转四元数 w

			float cr;     // 颜色 r
			float cg;     // 颜色 g
			float cb;     // 颜色 b
			float radius; // 半径估计值

			std::uint32_t shPacked[24]; // 压缩后 SH 系数（half2x16）
		};

		static_assert(sizeof(GPUSplat) == 160, "GPUSplat must match std430 array stride");

		// 计算不小于 value 的最小 2 的幂
		static std::size_t nextPow2(std::size_t value);
		// 确保颜色累积目标尺寸与资源有效
		bool ensureAccumulationTarget(int width, int height);
		// 执行深度键计算与 GPU 排序
		void runDepthAndSort(const glm::mat4& view);
		// 执行高斯累积绘制 pass
		void drawGaussianPass(const glm::mat4& view, const glm::mat4& projection, float viewportWidth, float viewportHeight, bool useReferencePath);
		// 执行累积结果合成 pass
		void compositeAccumulationPass(GLint prevDrawFbo, GLint prevReadFbo, const std::array<GLint, 4>& prevViewport);

		ShaderProgram m_drawProgram;      // 绘制程序
		ShaderProgram m_depthProgram;     // 深度键计算程序
		ShaderProgram m_sortProgram;      // bitonic 排序程序
		ShaderProgram m_compositeProgram; // 合成程序

		unsigned int m_vao{ 0 };          // 空 VAO
		unsigned int m_splatBuffer{ 0 };  // 高斯元 SSBO
		unsigned int m_keysBuffer{ 0 };   // 深度键 SSBO
		unsigned int m_indicesBuffer{ 0 };// 索引缓冲 SSBO
		unsigned int m_accumFbo{ 0 };     // 累积 FBO
		unsigned int m_accumColorTex{ 0 };// 累积颜色纹理

		int m_accumWidth{ 0 };  // 累积目标宽度
		int m_accumHeight{ 0 }; // 累积目标高度

		std::size_t m_splatCount{ 0 }; // 有效元数量
		std::size_t m_sortCount{ 0 };  // 排序数量（2 的幂）

		GLint m_drawViewLoc{ -1 };         // 绘制程序 u_view 位置
		GLint m_drawModelLoc{ -1 };        // 绘制程序 u_model 位置
		GLint m_drawProjLoc{ -1 };         // 绘制程序 u_proj 位置
		GLint m_drawViewportSizeLoc{ -1 }; // 绘制程序 u_viewportSize 位置
		GLint m_drawMaxPointSizeLoc{ -1 }; // 绘制程序 u_maxPointSize 位置
		GLint m_drawUseAnisotropicLoc{ -1 }; // 绘制程序 u_useAnisotropic 位置
		GLint m_drawCameraPosLoc{ -1 };    // 绘制程序 u_cameraPos 位置
		GLint m_drawShDegreeLoc{ -1 };     // 绘制程序 u_shDegree 位置

		GLint m_depthViewLoc{ -1 };         // 深度程序 u_view 位置
		GLint m_depthModelLoc{ -1 };        // 深度程序 u_model 位置
		GLint m_depthRealCountLoc{ -1 };    // 深度程序 u_realCount 位置
		GLint m_depthSortCountLoc{ -1 };    // 深度程序 u_sortCount 位置

		GLint m_sortCountLoc{ -1 };  // 排序程序 u_count 位置
		GLint m_sortStageLoc{ -1 };  // 排序程序 u_stage 位置
		GLint m_sortPassLoc{ -1 };   // 排序程序 u_pass 位置
		GLint m_compositeTexLoc{ -1 };// 合成程序 u_accumTex 位置

		float m_maxPointSize{ 128.0f }; // 设备支持的最大点尺寸
		bool m_useAnisotropic{ true };  // 各向异性开关
		int m_shDegree{ 1 };            // 当前 SH 阶数
		int m_maxSupportedShDegree{ 0 };// 模型支持的最大 SH 阶数
		glm::mat4 m_modelTransform{ 1.0f }; // 模型到世界变换
	};

} // namespace gs
