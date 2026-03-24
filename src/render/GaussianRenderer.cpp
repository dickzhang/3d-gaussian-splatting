#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "render/GaussianRenderer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

namespace gs
{
	namespace
	{

		constexpr int kMaxShDegree = 3;

		std::uint16_t floatToHalfBits(float value)
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
			} v{ value };

			const std::uint32_t sign = (v.u >> 16) & 0x8000u;
			std::uint32_t mantissa = v.u & 0x007FFFFFu;
			int exponent = static_cast<int>((v.u >> 23) & 0xFFu) - 127 + 15;

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

		std::uint32_t packHalf2x16(float a, float b)
		{
			const std::uint16_t ha = floatToHalfBits(a);
			const std::uint16_t hb = floatToHalfBits(b);
			return static_cast<std::uint32_t>(ha) | (static_cast<std::uint32_t>(hb) << 16);
		}

	} // namespace

	GaussianRenderer::~GaussianRenderer()
	{
		if (m_accumColorTex != 0)
		{
			glDeleteTextures(1, &m_accumColorTex);
		}
		if (m_accumFbo != 0)
		{
			glDeleteFramebuffers(1, &m_accumFbo);
		}
		if (m_splatBuffer != 0)
		{
			glDeleteBuffers(1, &m_splatBuffer);
		}
		if (m_keysBuffer != 0)
		{
			glDeleteBuffers(1, &m_keysBuffer);
		}
		if (m_indicesBuffer != 0)
		{
			glDeleteBuffers(1, &m_indicesBuffer);
		}
		if (m_vao != 0)
		{
			glDeleteVertexArrays(1, &m_vao);
		}
	}

	bool GaussianRenderer::initialize()
	{
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		const bool drawOk = m_drawProgram.createFromFiles("assets/shaders/gaussian.vert", "assets/shaders/gaussian.frag");
		const bool depthOk = m_depthProgram.createComputeFromFile("assets/shaders/depth_keys.comp");
		const bool sortOk = m_sortProgram.createComputeFromFile("assets/shaders/bitonic_sort.comp");
		const bool compositeOk = m_compositeProgram.createFromFiles("assets/shaders/composite.vert", "assets/shaders/composite.frag");

		if (!drawOk || !depthOk || !sortOk || !compositeOk)
		{
			std::cerr << "Failed to initialize shader programs\n";
			return false;
		}

		m_drawViewLoc = glGetUniformLocation(m_drawProgram.id(), "u_view");
		m_drawModelLoc = glGetUniformLocation(m_drawProgram.id(), "u_model");
		m_drawProjLoc = glGetUniformLocation(m_drawProgram.id(), "u_proj");
		m_drawViewportSizeLoc = glGetUniformLocation(m_drawProgram.id(), "u_viewportSize");
		m_drawMaxPointSizeLoc = glGetUniformLocation(m_drawProgram.id(), "u_maxPointSize");
		m_drawUseAnisotropicLoc = glGetUniformLocation(m_drawProgram.id(), "u_useAnisotropic");
		m_drawCameraPosLoc = glGetUniformLocation(m_drawProgram.id(), "u_cameraPos");
		m_drawShDegreeLoc = glGetUniformLocation(m_drawProgram.id(), "u_shDegree");

		m_depthViewLoc = glGetUniformLocation(m_depthProgram.id(), "u_view");
		m_depthModelLoc = glGetUniformLocation(m_depthProgram.id(), "u_model");
		m_depthRealCountLoc = glGetUniformLocation(m_depthProgram.id(), "u_realCount");
		m_depthSortCountLoc = glGetUniformLocation(m_depthProgram.id(), "u_sortCount");

		m_sortCountLoc = glGetUniformLocation(m_sortProgram.id(), "u_count");
		m_sortStageLoc = glGetUniformLocation(m_sortProgram.id(), "u_stage");
		m_sortPassLoc = glGetUniformLocation(m_sortProgram.id(), "u_pass");
		m_compositeTexLoc = glGetUniformLocation(m_compositeProgram.id(), "u_accumTex");

		const bool uniformsOk =
			m_drawViewLoc >= 0 &&
			m_drawModelLoc >= 0 &&
			m_drawProjLoc >= 0 &&
			m_drawViewportSizeLoc >= 0 &&
			m_drawMaxPointSizeLoc >= 0 &&
			m_drawUseAnisotropicLoc >= 0 &&
			m_drawCameraPosLoc >= 0 &&
			m_drawShDegreeLoc >= 0 &&
			m_depthViewLoc >= 0 &&
			m_depthModelLoc >= 0 &&
			m_depthRealCountLoc >= 0 &&
			m_depthSortCountLoc >= 0 &&
			m_sortCountLoc >= 0 &&
			m_sortStageLoc >= 0 &&
			m_sortPassLoc >= 0 &&
			m_compositeTexLoc >= 0;
		if (!uniformsOk)
		{
			std::cerr << "Failed to resolve one or more shader uniform locations\n";
			return false;
		}

		glGenBuffers(1, &m_splatBuffer);
		glGenBuffers(1, &m_keysBuffer);
		glGenBuffers(1, &m_indicesBuffer);

		GLfloat pointRange[2] = { 1.0f, 128.0f };
		glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointRange);
		m_maxPointSize = std::max(1.0f, pointRange[1]);
		return true;
	}

	bool GaussianRenderer::uploadModel(const GaussianModel& model)
	{
		if (model.empty())
		{
			std::cerr << "Cannot upload empty model\n";
			return false;
		}

		const std::size_t newSplatCount = model.size();
		const std::size_t newSortCount = nextPow2(newSplatCount);
		const int newMaxSupportedShDegree = std::max(0, std::min(kMaxShDegree, model.maxShDegree()));
		if (newSortCount > static_cast<std::size_t>(std::numeric_limits<GLuint>::max()))
		{
			std::cerr << "Model too large for current GPU sorting path\n";
			return false;
		}

		std::vector<GPUSplat> gpuData;
		gpuData.reserve(newSplatCount);
		for (const auto& splat : model.splats)
		{
			GPUSplat item{};
			item.px = splat.position.x;
			item.py = splat.position.y;
			item.pz = splat.position.z;
			item.opacity = splat.opacity;

			item.sx = splat.scale.x;
			item.sy = splat.scale.y;
			item.sz = splat.scale.z;

			item.rx = splat.rotation.x;
			item.ry = splat.rotation.y;
			item.rz = splat.rotation.z;
			item.rw = splat.rotation.w;

			item.cr = splat.color.r;
			item.cg = splat.color.g;
			item.cb = splat.color.b;
			item.radius = std::max({ splat.scale.x, splat.scale.y, splat.scale.z });

			const std::array<float, 48> shValues{
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
			static_assert(std::tuple_size<decltype(shValues)>::value == 48, "Expected 48 SH float values");
			static_assert(std::tuple_size<decltype(shValues)>::value == 24 * 2, "Packed SH layout mismatch");

			for (std::size_t k = 0; k < std::size(item.shPacked); ++k)
			{
				const std::size_t i0 = k * 2;
				const std::size_t i1 = i0 + 1;
				item.shPacked[k] = packHalf2x16(shValues[i0], shValues[i1]);
			}

			gpuData.push_back(item);
		}

		std::vector<std::uint32_t> initKeys(newSortCount, 0xFFFFFFFFu);
		std::vector<std::uint32_t> initIndices(newSortCount, 0xFFFFFFFFu);
		for (std::size_t i = 0; i < newSplatCount; ++i)
		{
			initIndices[i] = static_cast<std::uint32_t>(i);
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_splatBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(gpuData.size() * sizeof(GPUSplat)), gpuData.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_keysBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(initKeys.size() * sizeof(std::uint32_t)), initKeys.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_indicesBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(initIndices.size() * sizeof(std::uint32_t)), initIndices.data(), GL_DYNAMIC_DRAW);

		const GLenum uploadErr = glGetError();
		if (uploadErr != GL_NO_ERROR)
		{
			std::cerr << "Failed to upload model buffers, GL error: " << uploadErr << "\n";
			return false;
		}

		m_splatCount = newSplatCount;
		m_sortCount = newSortCount;
		m_maxSupportedShDegree = newMaxSupportedShDegree;
		m_shDegree = std::min(m_shDegree, m_maxSupportedShDegree);

		std::cout << "Uploaded " << m_splatCount << " splats to GPU (sort count: " << m_sortCount << ")\n";
		return true;
	}

	void GaussianRenderer::render(const glm::mat4& view, const glm::mat4& projection, float viewportWidth, float viewportHeight)
	{
		if (m_splatCount == 0)
		{
			return;
		}

		const GLboolean wasBlendEnabled = glIsEnabled(GL_BLEND);
		GLint prevBlendSrcRgb = GL_ONE;
		GLint prevBlendDstRgb = GL_ZERO;
		GLint prevBlendSrcAlpha = GL_ONE;
		GLint prevBlendDstAlpha = GL_ZERO;
		GLint prevBlendEqRgb = GL_FUNC_ADD;
		GLint prevBlendEqAlpha = GL_FUNC_ADD;
		glGetIntegerv(GL_BLEND_SRC_RGB, &prevBlendSrcRgb);
		glGetIntegerv(GL_BLEND_DST_RGB, &prevBlendDstRgb);
		glGetIntegerv(GL_BLEND_SRC_ALPHA, &prevBlendSrcAlpha);
		glGetIntegerv(GL_BLEND_DST_ALPHA, &prevBlendDstAlpha);
		glGetIntegerv(GL_BLEND_EQUATION_RGB, &prevBlendEqRgb);
		glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &prevBlendEqAlpha);

		GLboolean wasDepthWrite = GL_TRUE;
		glGetBooleanv(GL_DEPTH_WRITEMASK, &wasDepthWrite);

		const GLboolean wasCullFaceEnabled = glIsEnabled(GL_CULL_FACE);

		GLint prevDrawFbo = 0;
		GLint prevReadFbo = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFbo);
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFbo);

		std::array<GLint, 4> prevViewport{ 0, 0, 0, 0 };
		glGetIntegerv(GL_VIEWPORT, prevViewport.data());

		GLint prevProgram = 0;
		GLint prevVao = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVao);

		GLint prevActiveTexture = GL_TEXTURE0;
		GLint prevTexture2DOnUnit0 = 0;
		glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActiveTexture);
		glActiveTexture(GL_TEXTURE0);
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTexture2DOnUnit0);
		glActiveTexture(static_cast<GLenum>(prevActiveTexture));

		runDepthAndSort(view);

		const int renderWidth = std::max(1, static_cast<int>(viewportWidth));
		const int renderHeight = std::max(1, static_cast<int>(viewportHeight));
		bool useReferencePath = true;
		if (useReferencePath)
		{
			if (!ensureAccumulationTarget(renderWidth, renderHeight))
			{
				// Fallback to original path if offscreen accumulation cannot be created.
				useReferencePath = false;
			}
		}

		if (useReferencePath)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_accumFbo);
			glViewport(0, 0, renderWidth, renderHeight);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		drawGaussianPass(view, projection, viewportWidth, viewportHeight, useReferencePath);

		if (useReferencePath)
		{
			compositeAccumulationPass(prevDrawFbo, prevReadFbo, prevViewport);
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(prevTexture2DOnUnit0));
		glActiveTexture(static_cast<GLenum>(prevActiveTexture));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);

		glDepthMask(wasDepthWrite);
		glBlendFuncSeparate(
			static_cast<GLenum>(prevBlendSrcRgb),
			static_cast<GLenum>(prevBlendDstRgb),
			static_cast<GLenum>(prevBlendSrcAlpha),
			static_cast<GLenum>(prevBlendDstAlpha));
		glBlendEquationSeparate(
			static_cast<GLenum>(prevBlendEqRgb),
			static_cast<GLenum>(prevBlendEqAlpha));
		if (!wasBlendEnabled)
		{
			glDisable(GL_BLEND);
		}

		if (wasCullFaceEnabled)
		{
			glEnable(GL_CULL_FACE);
		}

		glBindVertexArray(static_cast<GLuint>(prevVao));
		glUseProgram(static_cast<GLuint>(prevProgram));
	}

	void GaussianRenderer::drawGaussianPass(const glm::mat4& view, const glm::mat4& projection, float viewportWidth, float viewportHeight, bool useReferencePath)
	{
		glEnable(GL_BLEND);
		if (useReferencePath)
		{
			// Unity plugin pass: Blend OneMinusDstAlpha One
			glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);

		m_drawProgram.use();
		glUniformMatrix4fv(m_drawViewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(m_drawModelLoc, 1, GL_FALSE, glm::value_ptr(m_modelTransform));
		glUniformMatrix4fv(m_drawProjLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform2f(m_drawViewportSizeLoc, viewportWidth, viewportHeight);
		glUniform1f(m_drawMaxPointSizeLoc, m_maxPointSize);
		glUniform1i(m_drawUseAnisotropicLoc, m_useAnisotropic ? 1 : 0);
		const glm::mat4 invView = glm::inverse(view);
		const glm::vec3 cameraPos(invView[3].x, invView[3].y, invView[3].z);
		glUniform3f(m_drawCameraPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
		glUniform1i(m_drawShDegreeLoc, m_shDegree);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_splatBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_indicesBuffer);

		glBindVertexArray(m_vao);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(m_splatCount));
	}

	void GaussianRenderer::compositeAccumulationPass(GLint prevDrawFbo, GLint prevReadFbo, const std::array<GLint, 4>& prevViewport)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevDrawFbo));
		glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
		glBindVertexArray(m_vao);

		m_compositeProgram.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_accumColorTex);
		glUniform1i(m_compositeTexLoc, 0);

		const GLboolean wasDepthTest = glIsEnabled(GL_DEPTH_TEST);
		glDisable(GL_DEPTH_TEST);

		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		if (wasDepthTest)
		{
			glEnable(GL_DEPTH_TEST);
		}

		glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(prevReadFbo));
	}

	bool GaussianRenderer::ensureAccumulationTarget(int width, int height)
	{
		width = std::max(1, width);
		height = std::max(1, height);

		if (m_accumFbo == 0)
		{
			glGenFramebuffers(1, &m_accumFbo);
		}
		if (m_accumColorTex == 0)
		{
			glGenTextures(1, &m_accumColorTex);
		}

		if (m_accumWidth == width && m_accumHeight == height)
		{
			return true;
		}

		m_accumWidth = width;
		m_accumHeight = height;

		glBindTexture(GL_TEXTURE_2D, m_accumColorTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_accumWidth, m_accumHeight, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);

		glBindFramebuffer(GL_FRAMEBUFFER, m_accumFbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_accumColorTex, 0);

		const GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Accumulation framebuffer incomplete: " << fboStatus << "\n";
			return false;
		}

		return true;
	}

	void GaussianRenderer::setUseAnisotropic(bool enabled)
	{
		m_useAnisotropic = enabled;
	}

	bool GaussianRenderer::useAnisotropic() const noexcept
	{
		return m_useAnisotropic;
	}

	int GaussianRenderer::setShDegree(int degree)
	{
		const int requested = std::max(0, std::min(kMaxShDegree, degree));
		m_shDegree = std::min(requested, m_maxSupportedShDegree);
		return m_shDegree;
	}

	int GaussianRenderer::shDegree() const noexcept
	{
		return m_shDegree;
	}

	int GaussianRenderer::maxSupportedShDegree() const noexcept
	{
		return m_maxSupportedShDegree;
	}

	void GaussianRenderer::setModelTransform(const glm::mat4& model) noexcept
	{
		const glm::mat3 linear(model);
		const float det = glm::determinant(linear);
		if (std::abs(det) < 1e-6f)
		{
			std::cerr << "Warning: model transform is near-singular; splat footprint may become unstable\n";
		}
		m_modelTransform = model;
	}

	const glm::mat4& GaussianRenderer::modelTransform() const noexcept
	{
		return m_modelTransform;
	}

	std::size_t GaussianRenderer::nextPow2(std::size_t value)
	{
		if (value <= 1)
		{
			return 1;
		}

		if (value > (std::numeric_limits<std::size_t>::max() >> 1))
		{
			return std::numeric_limits<std::size_t>::max() >> 1;
		}

		std::size_t p = 1;
		while (p < value && p <= (std::numeric_limits<std::size_t>::max() >> 1))
		{
			p <<= 1;
		}

		if (p < value)
		{
			return std::numeric_limits<std::size_t>::max() >> 1;
		}

		return p;
	}

	void GaussianRenderer::runDepthAndSort(const glm::mat4& view)
	{
		m_depthProgram.use();
		glUniformMatrix4fv(m_depthViewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(m_depthModelLoc, 1, GL_FALSE, glm::value_ptr(m_modelTransform));
		glUniform1ui(m_depthRealCountLoc, static_cast<GLuint>(m_splatCount));
		glUniform1ui(m_depthSortCountLoc, static_cast<GLuint>(m_sortCount));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_splatBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_keysBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_indicesBuffer);

		const GLuint groups = static_cast<GLuint>((m_sortCount + 255) / 256);
		glDispatchCompute(groups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		m_sortProgram.use();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_keysBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_indicesBuffer);
		glUniform1ui(m_sortCountLoc, static_cast<GLuint>(m_sortCount));

		const GLuint sortCount = static_cast<GLuint>(m_sortCount);
		for (GLuint stage = 2; stage <= sortCount && stage != 0; stage <<= 1)
		{
			glUniform1ui(m_sortStageLoc, stage);
			for (GLuint pass = stage >> 1; pass > 0; pass >>= 1)
			{
				glUniform1ui(m_sortPassLoc, pass);
				glDispatchCompute(groups, 1, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			}
		}
	}

} // namespace gs
