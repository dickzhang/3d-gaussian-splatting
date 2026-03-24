#pragma once

#include <cstddef>
#include <cstdint>

#include <glm/mat4x4.hpp>

#include "core/ShaderProgram.h"
#include "scene/GaussianModel.h"

namespace gs
{

	// 高斯渲染器：负责GPU资源管理、排序与高斯点元绘制
	class GaussianRenderer
	{
	public:
		GaussianRenderer() = default;
		~GaussianRenderer();

		// 禁止拷贝，避免OpenGL资源重复释放
		GaussianRenderer(const GaussianRenderer&) = delete;
		GaussianRenderer& operator=(const GaussianRenderer&) = delete;

		// 初始化着色器与GPU缓冲资源
		bool initialize();
		// 上传模型数据到GPU
		bool uploadModel(const GaussianModel& model);
		// 执行一帧渲染
		void render(const glm::mat4& view, const glm::mat4& projection, float viewportWidth, float viewportHeight);
		// 设置是否启用各向异性高斯
		void setUseAnisotropic(bool enabled);
		// 查询各向异性开关
		bool useAnisotropic() const noexcept;
		// 设置SH阶数（会按模型能力自动裁剪）
		int setShDegree(int degree);
		// 获取当前SH阶数
		int shDegree() const noexcept;
		// 获取模型支持的最大SH阶数
		int maxSupportedShDegree() const noexcept;
		// 设置参考观感路径开关
		void setReferenceLook(bool enabled);
		// 查询参考观感路径开关
		bool referenceLook() const noexcept;

	private:
		// GPU侧点元布局（需与shader中的std430结构严格一致）
		struct GPUSplat
		{
			float px;      // 位置x
			float py;      // 位置y
			float pz;      // 位置z
			float opacity; // 不透明度

			float sx;    // 尺度x
			float sy;    // 尺度y
			float sz;    // 尺度z
			float _pad0; // 对齐填充

			float rx; // 旋转四元数x
			float ry; // 旋转四元数y
			float rz; // 旋转四元数z
			float rw; // 旋转四元数w

			float cr;     // 颜色r
			float cg;     // 颜色g
			float cb;     // 颜色b
			float radius; // 半径近似值

			std::uint32_t shPacked[24]; // 打包后的SH系数（half2x16）
		};

		static_assert(sizeof(GPUSplat) == 160, "GPUSplat must match std430 array stride");

		// 计算不小于value的最小2次幂
		static std::size_t nextPow2(std::size_t value);
		// 确保离屏累积目标尺寸与资源有效
		bool ensureAccumulationTarget(int width, int height);
		// 执行深度键生成与GPU排序
		void runDepthAndSort(const glm::mat4& view);

		ShaderProgram m_drawProgram;      // 主绘制程序
		ShaderProgram m_depthProgram;     // 深度键计算程序
		ShaderProgram m_sortProgram;      // bitonic排序程序
		ShaderProgram m_compositeProgram; // 合成程序

		unsigned int m_vao{ 0 };          // 绘制用VAO
		unsigned int m_splatBuffer{ 0 };  // 点元SSBO
		unsigned int m_keysBuffer{ 0 };   // 排序键SSBO
		unsigned int m_indicesBuffer{ 0 };// 排序索引SSBO
		unsigned int m_accumFbo{ 0 };     // 累积FBO
		unsigned int m_accumColorTex{ 0 };// 累积颜色纹理

		int m_accumWidth{ 0 };  // 累积纹理宽度
		int m_accumHeight{ 0 }; // 累积纹理高度

		std::size_t m_splatCount{ 0 }; // 有效点元数量
		std::size_t m_sortCount{ 0 };  // 排序容量（2次幂

		GLint m_drawViewLoc{ -1 };         // 主绘制u_view位置
		GLint m_drawProjLoc{ -1 };         // 主绘制u_proj位置
		GLint m_drawViewportSizeLoc{ -1 }; // 主绘制u_viewportSize位置
		GLint m_drawMaxPointSizeLoc{ -1 }; // 主绘制u_maxPointSize位置
		GLint m_drawUseAnisotropicLoc{ -1 }; // 主绘制u_useAnisotropic位置
		GLint m_drawCameraPosLoc{ -1 };    // 主绘制u_cameraPos位置
		GLint m_drawShDegreeLoc{ -1 };     // 主绘制u_shDegree位置
		GLint m_drawReferenceLookLoc{ -1 };// 主绘制u_referenceLook位置

		GLint m_depthViewLoc{ -1 };         // 深度程序u_view位置
		GLint m_depthRealCountLoc{ -1 };    // 深度程序u_realCount位置
		GLint m_depthSortCountLoc{ -1 };    // 深度程序u_sortCount位置
		GLint m_depthFrontToBackLoc{ -1 };  // 深度程序u_frontToBack位置

		GLint m_sortCountLoc{ -1 };  // 排序程序u_count位置
		GLint m_sortStageLoc{ -1 };  // 排序程序u_stage位置
		GLint m_sortPassLoc{ -1 };   // 排序程序u_pass位置
		GLint m_compositeTexLoc{ -1 };// 合成程序u_accumTex位置

		float m_maxPointSize{ 128.0f }; // 设备支持的最大点尺寸
		bool m_useAnisotropic{ true };  // 各向异性开关
		int m_shDegree{ 1 };            // 当前SH阶数
		int m_maxSupportedShDegree{ 0 };// 模型支持的最大SH阶数
		bool m_referenceLook{ true };   // 参考观感渲染路径开关
	};

} // namespace gs
