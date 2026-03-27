#include <glad/glad.h>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/PathUtils.h"
#include "render/GaussianRenderer.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace gs
{
	namespace
	{

		constexpr GLuint kComputeWorkGroupSize = 256u;
		constexpr std::uint32_t kInvalidSplatIndex = 0xFFFFFFFFu;
		constexpr std::uint64_t kChunkDebugLogInterval = 120u;
		constexpr std::uint32_t kMaxChunkDebugLogs = 10u;
		constexpr std::uint32_t kMaxGpuValidationLogs = 10u;
		constexpr float kDefaultChunkSchedulingDisableVisibleRatio = 0.90f;
		constexpr float kDefaultChunkSchedulingEnableVisibleRatio = 0.80f;

		std::size_t nextPow2(std::size_t value)
		{
			if (value <= 1)
			{
				return 1;
			}

			if (value > (std::numeric_limits<std::size_t>::max() >> 1))
			{
				return std::numeric_limits<std::size_t>::max() >> 1;
			}

			std::size_t padded = 1;
			while (padded < value && padded <= (std::numeric_limits<std::size_t>::max() >> 1))
			{
				padded <<= 1;
			}

			if (padded < value)
			{
				return std::numeric_limits<std::size_t>::max() >> 1;
			}

			return padded;
		}

		GLuint dispatchGroupCount(std::size_t elementCount)
		{
			if (elementCount == 0)
			{
				return 0;
			}

			return static_cast<GLuint>((elementCount + (kComputeWorkGroupSize - 1)) / kComputeWorkGroupSize);
		}

		float maxModelAxisScale(const glm::mat4& model)
		{
			const glm::vec3 c0(model[0].x, model[0].y, model[0].z);
			const glm::vec3 c1(model[1].x, model[1].y, model[1].z);
			const glm::vec3 c2(model[2].x, model[2].y, model[2].z);
			return std::max(glm::length(c0), std::max(glm::length(c1), glm::length(c2)));
		}

		bool isChunkVisible(
			const SplatCacheChunkEntry& chunk,
			const glm::mat4& model,
			const glm::mat4& view,
			const glm::mat4& projection)
		{
			const glm::vec4 localCenter(chunk.center_x, chunk.center_y, chunk.center_z, 1.0f);
			const glm::vec4 worldCenter = model * localCenter;
			const glm::vec4 viewCenter4 = view * worldCenter;
			const glm::vec3 viewCenter(viewCenter4.x, viewCenter4.y, viewCenter4.z);

			const float sphereRadius = chunk.radius * maxModelAxisScale(model);
			if (!std::isfinite(sphereRadius) || sphereRadius < 0.0f)
			{
				return true;
			}

			if (viewCenter.z >= sphereRadius)
			{
				return false;
			}

			const float depth = std::max(0.01f, -viewCenter.z);
			const float tanFovX = 1.0f / projection[0][0];
			const float tanFovY = 1.0f / projection[1][1];
			const float limitX = depth * tanFovX + sphereRadius;
			const float limitY = depth * tanFovY + sphereRadius;

			return std::abs(viewCenter.x) <= limitX && std::abs(viewCenter.y) <= limitY;
		}

		bool validateChunkRanges(
			const std::vector<SplatCacheChunkEntry>& chunks,
			std::size_t totalSplatCount)
		{
			std::size_t expectedStartIndex = 0;
			for (const SplatCacheChunkEntry& chunk : chunks)
			{
				if (chunk.splat_count == 0)
				{
					std::cerr << "Chunk metadata contains an empty range\n";
					return false;
				}

				if (!(chunk.radius >= 0.0f))
				{
					std::cerr << "Chunk metadata contains an invalid radius\n";
					return false;
				}

				const std::size_t startIndex = static_cast<std::size_t>(chunk.start_index);
				const std::size_t splatCount = static_cast<std::size_t>(chunk.splat_count);
				if (startIndex != expectedStartIndex)
				{
					std::cerr << "Chunk metadata must cover splats contiguously in ascending order\n";
					return false;
				}

				if (startIndex >= totalSplatCount)
				{
					std::cerr << "Chunk metadata start index exceeds uploaded splat count\n";
					return false;
				}

				if (splatCount > (totalSplatCount - startIndex))
				{
					std::cerr << "Chunk metadata range exceeds uploaded splat count\n";
					return false;
				}

				expectedStartIndex += splatCount;
			}

			if (!chunks.empty() && expectedStartIndex != totalSplatCount)
			{
				std::cerr << "Chunk metadata does not cover the full uploaded splat range\n";
				return false;
			}

			return true;
		}

		std::string readEnvironmentValue(const char* name)
		{
#ifdef _WIN32
			char* rawValue = nullptr;
			std::size_t size = 0;
			if (_dupenv_s(&rawValue, &size, name) != 0 || rawValue == nullptr)
			{
				return {};
			}

			std::unique_ptr<char, decltype(&std::free)> value(rawValue, &std::free);
			return std::string(value.get());
#else
			const char* value = std::getenv(name);
			return value != nullptr ? std::string(value) : std::string{};
#endif
		}

		float readEnvironmentFloat(const char* name, float fallback)
		{
			const std::string rawValue = readEnvironmentValue(name);
			if (rawValue.empty())
			{
				return fallback;
			}

			char* parseEnd = nullptr;
			const float parsed = std::strtof(rawValue.c_str(), &parseEnd);
			if (parseEnd == rawValue.c_str() || !std::isfinite(parsed))
			{
				return fallback;
			}

			return std::clamp(parsed, 0.0f, 1.0f);
		}

		bool readEnvironmentFlag(const char* name)
		{
			const std::string value = readEnvironmentValue(name);
			return !value.empty() && value[0] != '0';
		}

		void emitRuntimeLog(std::ostream& stream, const std::string& message)
		{
			stream << message << '\n';
			stream.flush();

			const std::string logPath = readEnvironmentValue("GS_RUNTIME_LOG_FILE");
			if (logPath.empty())
			{
				return;
			}

			std::ofstream logStream(gs::pathFromText(logPath), std::ios::app);
			if (logStream.is_open())
			{
				logStream << message << '\n';
			}
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
		GpuUploadBuffers::destroy(m_uploadBuffers);
		if (m_vao != 0)
		{
			glDeleteVertexArrays(1, &m_vao);
		}
	}

	bool GaussianRenderer::initialize()
	{
		m_initialized = false;
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		const bool drawOk = m_drawProgram.createFromFiles("assets/shaders/gaussian.vert", "assets/shaders/gaussian.frag");
		const bool depthOk = m_depthProgram.createComputeFromFile("assets/shaders/depth_keys.comp");
		const bool sortOk = m_sortProgram.createComputeFromFile("assets/shaders/bitonic_sort.comp");
		const bool compositeOk = m_compositeProgram.createFromFiles("assets/shaders/composite.vert", "assets/shaders/composite.frag");
		const bool schedulerOk = m_chunkSchedulerPipeline.initialize();
		const bool scheduleCompactionOk = m_scheduleCompactionPipeline.initialize();
		const bool viewDataOk = m_viewDataPipeline.initialize();

		if (!drawOk || !depthOk || !sortOk || !compositeOk || !schedulerOk || !scheduleCompactionOk || !viewDataOk)
		{
			std::cerr << "Failed to initialize shader programs\n";
			return false;
		}

		m_drawViewportSizeLoc = glGetUniformLocation(m_drawProgram.id(), "u_viewportSize");
		m_drawUseAnisotropicLoc = glGetUniformLocation(m_drawProgram.id(), "u_useAnisotropic");

		m_depthViewLoc = glGetUniformLocation(m_depthProgram.id(), "u_view");
		m_depthModelLoc = glGetUniformLocation(m_depthProgram.id(), "u_model");
		m_depthRealCountLoc = glGetUniformLocation(m_depthProgram.id(), "u_realCount");
		m_depthSortCountLoc = glGetUniformLocation(m_depthProgram.id(), "u_sortCount");
		m_depthChunkCountLoc = glGetUniformLocation(m_depthProgram.id(), "u_chunkCount");
		m_depthScheduleEntryCountLoc = glGetUniformLocation(m_depthProgram.id(), "u_scheduleEntryCount");
		m_depthInputLayoutLoc = glGetUniformLocation(m_depthProgram.id(), "u_inputLayout");
		m_depthUseSeedIndicesLoc = glGetUniformLocation(m_depthProgram.id(), "u_useSeedIndices");
		m_depthUseScheduleDomainLoc = glGetUniformLocation(m_depthProgram.id(), "u_useScheduleDomain");

		m_sortCountLoc = glGetUniformLocation(m_sortProgram.id(), "u_count");
		m_sortStageLoc = glGetUniformLocation(m_sortProgram.id(), "u_stage");
		m_sortPassLoc = glGetUniformLocation(m_sortProgram.id(), "u_pass");
		m_compositeTexLoc = glGetUniformLocation(m_compositeProgram.id(), "u_accumTex");

		const bool uniformsOk =
			m_drawViewportSizeLoc >= 0 &&
			m_drawUseAnisotropicLoc >= 0 &&
			m_depthViewLoc >= 0 &&
			m_depthModelLoc >= 0 &&
			m_depthRealCountLoc >= 0 &&
			m_depthSortCountLoc >= 0 &&
			m_depthChunkCountLoc >= 0 &&
			m_depthScheduleEntryCountLoc >= 0 &&
			m_depthInputLayoutLoc >= 0 &&
			m_depthUseSeedIndicesLoc >= 0 &&
			m_depthUseScheduleDomainLoc >= 0 &&
			m_sortCountLoc >= 0 &&
			m_sortStageLoc >= 0 &&
			m_sortPassLoc >= 0 &&
			m_compositeTexLoc >= 0;
		if (!uniformsOk)
		{
			std::cerr << "Failed to resolve one or more shader uniform locations\n";
			return false;
		}

		if (!GpuUploadBuffers::create(m_uploadBuffers))
		{
			std::cerr << "Failed to create GPU upload buffers\n";
			return false;
		}

		GLfloat pointRange[2] = { 1.0f, 128.0f };
		glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointRange);
		m_maxPointSize = std::max(1.0f, pointRange[1]);
		loadChunkSchedulingConfig();
		const std::string debugChunkEnv = readEnvironmentValue("GS_DEBUG_CHUNKS");
		m_debugChunkStats = !debugChunkEnv.empty() && debugChunkEnv[0] != '0';
		m_initialized = true;
		return true;
	}

	bool GaussianRenderer::uploadAsset(const RuntimeSplatAsset& asset)
	{
		if (!asset.has_split_sections())
		{
			std::cerr << "Runtime asset is missing split cache sections\n";
			return false;
		}

		GpuUploadStats stats{};
		if (!GpuUploadBuffers::upload_split_splats(m_uploadBuffers, asset, stats))
		{
			return false;
		}

		if (stats.splat_count > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()))
		{
			std::cerr << "Runtime splat count exceeds uint32 index addressable range\n";
			return false;
		}

		if (!asset.chunks.empty() && !validateChunkRanges(asset.chunks, stats.splat_count))
		{
			return false;
		}

		m_chunks = asset.chunks;
		m_totalSplatCount = stats.splat_count;
		m_activeSplatCount = stats.splat_count;
		m_sortCapacity = stats.sort_count;
		m_sortCount = stats.sort_count;
		m_chunkCount = stats.chunk_count;
		m_visibleChunkCount = stats.chunk_count;
		m_visibleScheduleEntryCount = stats.chunk_count;
		m_compactedSplatCount = stats.splat_count;
		m_lastVisibleRatio = stats.splat_count == 0 ? 0.0f : 1.0f;
		m_maxSupportedShDegree = stats.max_supported_sh_degree;
		m_shDegree = std::min(m_shDegree, m_maxSupportedShDegree);
		m_inputLayout = 1;
		m_hasChunkSchedulingSupport = !m_chunks.empty();
		m_useSeededIndicesThisFrame = m_hasChunkSchedulingSupport;
		m_usedGpuSchedulerThisFrame = false;
		m_visibleScheduleScratch.clear();
		m_visibleScheduleScratch.reserve(m_chunkCount);

		emitRuntimeLog(std::cout, "Uploaded split-section asset: " + std::to_string(m_totalSplatCount) +
			" splats to GPU (sort count: " + std::to_string(m_sortCount) +
			", chunks: " + std::to_string(stats.chunk_count) +
			", chunk scheduling: " + std::string(m_hasChunkSchedulingSupport ? "on" : "off") + ")");
		return true;
	}

	void GaussianRenderer::render(const glm::mat4& view, const glm::mat4& projection, float viewportWidth, float viewportHeight)
	{
		if (!m_initialized)
		{
			return;
		}

		if (m_totalSplatCount == 0)
		{
			return;
		}

		++m_renderFrameIndex;

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

		GLint prevSsbo0 = 0;
		GLint prevSsbo1 = 0;
		GLint prevSsbo2 = 0;
		GLint prevSsbo3 = 0;
		GLint prevSsbo4 = 0;
		GLint prevSsbo5 = 0;
		GLint prevSsbo6 = 0;
		GLint prevSsbo7 = 0;
		GLint prevSsbo8 = 0;
		glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 0, &prevSsbo0);
		glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 1, &prevSsbo1);
		glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 2, &prevSsbo2);
		glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 3, &prevSsbo3);
		glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 4, &prevSsbo4);
		glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 5, &prevSsbo5);
		glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 6, &prevSsbo6);
		glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 7, &prevSsbo7);
		glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 8, &prevSsbo8);

		const bool depthAndSortExecuted = prepareVisibleSplatDomain(view, projection);
		bool canDraw = depthAndSortExecuted && m_activeSplatCount > 0;
		if (depthAndSortExecuted && m_activeSplatCount > 0)
		{
			runDepthAndSort(view);
		}
		bool viewDataExecuted = false;
		ViewDataPipeline::DispatchStats viewDataStats{};
		if (canDraw && !runViewDataPass(
			view,
			projection,
			viewportWidth,
			viewportHeight,
			m_useSeededIndicesThisFrame,
			&viewDataStats))
		{
			std::cerr << "View-data compute pass failed, skipping draw for this frame\n";
			canDraw = false;
		}
		else if (canDraw)
		{
			viewDataExecuted = true;
		}

		const int renderWidth = std::max(1, static_cast<int>(viewportWidth));
		const int renderHeight = std::max(1, static_cast<int>(viewportHeight));
		bool useReferencePath = true;
		if (canDraw && useReferencePath)
		{
			if (!ensureAccumulationTarget(renderWidth, renderHeight))
			{
				// Fallback to original path if offscreen accumulation cannot be created.
				useReferencePath = false;
				if (!m_loggedCompositeFallback)
				{
					emitRuntimeLog(std::cerr, "ACCEPT_WARN: composite reference path unavailable; frame used direct blend fallback");
					m_loggedCompositeFallback = true;
				}
			}
		}

		if (canDraw && useReferencePath)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_accumFbo);
			glViewport(0, 0, renderWidth, renderHeight);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		bool drawExecuted = false;
		if (canDraw)
		{
			drawGaussianPass(viewportWidth, viewportHeight, useReferencePath);
			drawExecuted = true;
		}

		bool compositeExecuted = false;
		if (canDraw && useReferencePath)
		{
			compositeAccumulationPass(prevDrawFbo, prevReadFbo, prevViewport);
			compositeExecuted = true;
		}

		if (!m_loggedAcceptanceFrame && depthAndSortExecuted && viewDataExecuted && drawExecuted && compositeExecuted)
		{
			emitRuntimeLog(std::cout, "ACCEPT: pipeline frame completed (depth_sort=1 view_data=1 draw=1 composite=1 reference_path=1)");
			m_loggedAcceptanceFrame = true;
		}

		if (m_debugChunkStats && viewDataExecuted && viewDataStats.chunk_culled_splats > viewDataStats.chunk_tests)
		{
			emitRuntimeLog(
				std::cerr,
				"CHUNK_DEBUG_WARN: invalid stats detected (chunk_culled_splats > chunk_tests)");
		}

		if (m_debugChunkStats &&
			viewDataExecuted &&
			m_chunkDebugLogCount < kMaxChunkDebugLogs &&
			(m_renderFrameIndex == 1u || (m_renderFrameIndex % kChunkDebugLogInterval) == 0u))
		{
			if (!m_loggedChunkSchedulingConfig)
			{
				m_loggedChunkSchedulingConfig = true;
				emitRuntimeLog(
					std::cout,
					"CHUNK_DEBUG_CONFIG: scheduler_mode=" +
					std::string(
						m_chunkSchedulerMode == ChunkSchedulerMode::Cpu ? "cpu" :
						m_chunkSchedulerMode == ChunkSchedulerMode::Gpu ? "gpu" :
						m_chunkSchedulerMode == ChunkSchedulerMode::Full ? "full" : "auto") +
					" enable_ratio=" + std::to_string(m_chunkSchedulingEnableVisibleRatio) +
					" disable_ratio=" + std::to_string(m_chunkSchedulingDisableVisibleRatio));
			}

			++m_chunkDebugLogCount;
			emitRuntimeLog(
				std::cout,
				"CHUNK_DEBUG: frame=" + std::to_string(m_renderFrameIndex) +
				" chunks=" + std::to_string(m_chunkCount) +
				" visible_chunks=" + std::to_string(m_visibleChunkCount) +
				" scheduled_ranges=" + std::to_string(m_visibleScheduleEntryCount) +
				" compacted_splats=" + std::to_string(m_compactedSplatCount) +
				" producer=" + std::string(m_usedGpuSchedulerThisFrame ? "gpu" : "cpu") +
				" path=" + std::string(m_useSeededIndicesThisFrame ? "seeded" : "full") +
					" visible_ratio=" + std::to_string(m_lastVisibleRatio) +
				" active_splats=" + std::to_string(m_activeSplatCount) +
				" active_sort_count=" + std::to_string(m_sortCount) +
				" processed_splats=" + std::to_string(viewDataStats.processed_splats) +
				" chunk_tests=" + std::to_string(viewDataStats.chunk_tests) +
				" chunk_culled_splats=" + std::to_string(viewDataStats.chunk_culled_splats));
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(prevTexture2DOnUnit0));
		glActiveTexture(static_cast<GLenum>(prevActiveTexture));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, static_cast<GLuint>(prevSsbo0));
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, static_cast<GLuint>(prevSsbo1));
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, static_cast<GLuint>(prevSsbo2));
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, static_cast<GLuint>(prevSsbo3));
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, static_cast<GLuint>(prevSsbo4));
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, static_cast<GLuint>(prevSsbo5));
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, static_cast<GLuint>(prevSsbo6));
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, static_cast<GLuint>(prevSsbo7));
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, static_cast<GLuint>(prevSsbo8));

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

	void GaussianRenderer::drawGaussianPass(float viewportWidth, float viewportHeight, bool useReferencePath)
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
		glUniform2f(m_drawViewportSizeLoc, viewportWidth, viewportHeight);
		glUniform1i(m_drawUseAnisotropicLoc, m_useAnisotropic ? 1 : 0);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_uploadBuffers.view_data_buffer);

		glBindVertexArray(m_vao);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(m_activeSplatCount));
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

	void GaussianRenderer::loadChunkSchedulingConfig()
	{
		m_chunkSchedulingEnableVisibleRatio = readEnvironmentFloat(
			"GS_CHUNK_ENABLE_RATIO",
			kDefaultChunkSchedulingEnableVisibleRatio);
		m_chunkSchedulingDisableVisibleRatio = readEnvironmentFloat(
			"GS_CHUNK_DISABLE_RATIO",
			kDefaultChunkSchedulingDisableVisibleRatio);
		if (!(m_chunkSchedulingEnableVisibleRatio < m_chunkSchedulingDisableVisibleRatio))
		{
			m_chunkSchedulingEnableVisibleRatio = kDefaultChunkSchedulingEnableVisibleRatio;
			m_chunkSchedulingDisableVisibleRatio = kDefaultChunkSchedulingDisableVisibleRatio;
		}

		std::string schedulerMode = readEnvironmentValue("GS_CHUNK_SCHEDULER_MODE");
		std::transform(schedulerMode.begin(), schedulerMode.end(), schedulerMode.begin(), [](unsigned char ch)
			{
				return static_cast<char>(std::tolower(ch));
			});
		if (schedulerMode == "cpu")
		{
			m_chunkSchedulerMode = ChunkSchedulerMode::Cpu;
		}
		else if (schedulerMode == "gpu")
		{
			m_chunkSchedulerMode = ChunkSchedulerMode::Gpu;
		}
		else if (schedulerMode == "full")
		{
			m_chunkSchedulerMode = ChunkSchedulerMode::Full;
		}
		else
		{
			m_chunkSchedulerMode = ChunkSchedulerMode::Auto;
		}

		m_validateGpuCompaction = readEnvironmentFlag("GS_VALIDATE_GPU_COMPACTION");
	}

	void GaussianRenderer::resetActiveDomainToFull() noexcept
	{
		m_activeSplatCount = m_totalSplatCount;
		m_sortCount = m_sortCapacity;
		m_useSeededIndicesThisFrame = false;
	}

	bool GaussianRenderer::shouldUseSeededIndices(bool previousSeededPath, float visibleRatio) const noexcept
	{
		const double ratio = static_cast<double>(visibleRatio);
		return previousSeededPath
			? ratio < static_cast<double>(m_chunkSchedulingDisableVisibleRatio)
			: ratio < static_cast<double>(m_chunkSchedulingEnableVisibleRatio);
	}

	bool GaussianRenderer::runScheduleCompaction(std::size_t scheduleEntryCount)
	{
		return m_scheduleCompactionPipeline.dispatch(
			m_uploadBuffers,
			m_chunkCount,
			scheduleEntryCount,
			m_sortCount);
	}

	bool GaussianRenderer::prepareVisibleSplatDomain(const glm::mat4& view, const glm::mat4& projection)
	{
		const bool previousSeededPath = m_useSeededIndicesThisFrame;
		resetActiveDomainToFull();
		m_visibleChunkCount = m_chunkCount;
		m_visibleScheduleEntryCount = m_chunkCount;
		m_compactedSplatCount = m_totalSplatCount;
		m_lastVisibleRatio = m_totalSplatCount == 0 ? 0.0f : 1.0f;
		m_usedGpuSchedulerThisFrame = false;
		m_depthUseScheduleDomainThisFrame = false;
		m_viewDataUseScheduleDomainThisFrame = false;

		if (!m_hasChunkSchedulingSupport)
		{
			return true;
		}

		if (m_chunkSchedulerMode == ChunkSchedulerMode::Full)
		{
			return true;
		}

		if (m_chunkSchedulerMode == ChunkSchedulerMode::Cpu)
		{
			if (!prepareVisibleSplatDomainCpu(view, projection, previousSeededPath))
			{
				std::cerr << "CPU chunk scheduler exceeded capacity, falling back to full domain for this frame\n";
				resetActiveDomainToFull();
			}
			return true;
		}

		if (prepareVisibleSplatDomainGpu(view, projection, previousSeededPath))
		{
			return true;
		}

		std::cerr << "GPU chunk scheduler failed, falling back to CPU path for this frame\n";
		if (!prepareVisibleSplatDomainCpu(view, projection, previousSeededPath))
		{
			std::cerr << "CPU chunk scheduler exceeded capacity during fallback, using full domain for this frame\n";
			resetActiveDomainToFull();
		}
		return true;
	}

	bool GaussianRenderer::prepareVisibleSplatDomainCpu(const glm::mat4& view, const glm::mat4& projection, bool previousSeededPath)
	{

		m_visibleScheduleScratch.clear();
		m_visibleScheduleScratch.reserve(m_chunkCount);
		m_visibleChunkCount = 0;
		m_visibleScheduleEntryCount = 0;
		m_usedGpuSchedulerThisFrame = false;
		std::size_t scheduleOutputOffset = 0;

		for (std::size_t chunkIndex = 0; chunkIndex < m_chunks.size(); ++chunkIndex)
		{
			const SplatCacheChunkEntry& chunk = m_chunks[chunkIndex];
			if (!isChunkVisible(chunk, m_modelTransform, view, projection))
			{
				continue;
			}
			if (chunk.start_index >= m_totalSplatCount)
			{
				continue;
			}
			const std::size_t startIndex = chunk.start_index;
			const std::size_t endIndex = std::min(m_totalSplatCount, startIndex + static_cast<std::size_t>(chunk.splat_count));
			const std::size_t visibleChunkSplatCount = endIndex - startIndex;
			if (m_visibleScheduleScratch.size() >= m_chunkCount ||
				visibleChunkSplatCount > m_sortCapacity ||
				scheduleOutputOffset > (m_sortCapacity - visibleChunkSplatCount))
			{
				return false;
			}

			ChunkScheduleEntry scheduleEntry{};
			scheduleEntry.chunk_index = static_cast<std::uint32_t>(chunkIndex);
			scheduleEntry.output_offset = static_cast<std::uint32_t>(scheduleOutputOffset);
			scheduleEntry.splat_count = static_cast<std::uint32_t>(visibleChunkSplatCount);
			m_visibleScheduleScratch.push_back(scheduleEntry);
			scheduleOutputOffset += visibleChunkSplatCount;

			++m_visibleChunkCount;
			++m_visibleScheduleEntryCount;
		}

		m_compactedSplatCount = scheduleOutputOffset;
		m_lastVisibleRatio =
			m_totalSplatCount == 0 ? 0.0f : static_cast<float>(static_cast<double>(m_compactedSplatCount) / static_cast<double>(m_totalSplatCount));
		if (m_compactedSplatCount == 0)
		{
			m_activeSplatCount = 0;
			m_sortCount = 0;
			m_useSeededIndicesThisFrame = true;
			m_depthUseScheduleDomainThisFrame = true;
			m_viewDataUseScheduleDomainThisFrame = true;
			return true;
		}

		if (!shouldUseSeededIndices(previousSeededPath, m_lastVisibleRatio))
		{
			resetActiveDomainToFull();
			return true;
		}

		m_activeSplatCount = m_compactedSplatCount;
		m_useSeededIndicesThisFrame = true;
		m_depthUseScheduleDomainThisFrame = true;
		m_viewDataUseScheduleDomainThisFrame = true;

		m_sortCount = nextPow2(m_activeSplatCount);
		if (m_sortCount > m_sortCapacity)
		{
			resetActiveDomainToFull();
			return true;
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_uploadBuffers.chunk_schedule_buffer);
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER,
			0,
			static_cast<GLsizeiptr>(m_visibleScheduleScratch.size() * sizeof(ChunkScheduleEntry)),
			m_visibleScheduleScratch.data());

		const ChunkSchedulerStats cpuScheduleStats{
			static_cast<std::uint32_t>(m_visibleChunkCount),
			static_cast<std::uint32_t>(m_compactedSplatCount),
			static_cast<std::uint32_t>(m_visibleScheduleEntryCount),
			0u };
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_uploadBuffers.chunk_scheduler_stats_buffer);
		glBufferSubData(
			GL_SHADER_STORAGE_BUFFER,
			0,
			static_cast<GLsizeiptr>(sizeof(ChunkSchedulerStats)),
			&cpuScheduleStats);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		const GLenum uploadErr = glGetError();
		if (uploadErr != GL_NO_ERROR)
		{
			std::cerr << "Failed to upload CPU visible chunk schedule, GL error: " << uploadErr << "\n";
			resetActiveDomainToFull();
			return true;
		}

		if (!runScheduleCompaction(m_visibleScheduleEntryCount))
		{
			return false;
		}

		return true;
	}

	bool GaussianRenderer::prepareVisibleSplatDomainGpu(const glm::mat4& view, const glm::mat4& projection, bool previousSeededPath)
	{
		ChunkSchedulerPipeline::DispatchStats schedulerStats{};
		if (!m_chunkSchedulerPipeline.dispatch(
			m_uploadBuffers,
			view,
			m_modelTransform,
			projection,
			m_chunkCount,
			m_sortCapacity,
			&schedulerStats))
		{
			return false;
		}

		m_usedGpuSchedulerThisFrame = true;
		m_visibleChunkCount = schedulerStats.visible_chunk_count;
		m_visibleScheduleEntryCount = schedulerStats.schedule_entry_count;
		m_compactedSplatCount = schedulerStats.active_splat_count;
		m_lastVisibleRatio =
			m_totalSplatCount == 0 ? 0.0f : static_cast<float>(static_cast<double>(m_compactedSplatCount) / static_cast<double>(m_totalSplatCount));

		if (schedulerStats.overflowed || m_compactedSplatCount > m_sortCapacity)
		{
			return false;
		}

		if (m_compactedSplatCount == 0)
		{
			m_activeSplatCount = 0;
			m_sortCount = 0;
			m_useSeededIndicesThisFrame = true;
			m_depthUseScheduleDomainThisFrame = true;
			m_viewDataUseScheduleDomainThisFrame = true;
		}
		else if (!shouldUseSeededIndices(previousSeededPath, m_lastVisibleRatio))
		{
			resetActiveDomainToFull();
		}
		else
		{
			m_activeSplatCount = m_compactedSplatCount;
			m_useSeededIndicesThisFrame = true;
			m_depthUseScheduleDomainThisFrame = true;
			m_viewDataUseScheduleDomainThisFrame = true;
			m_sortCount = nextPow2(m_activeSplatCount);
			if (m_sortCount > m_sortCapacity)
			{
				resetActiveDomainToFull();
			}
		}

		if (m_useSeededIndicesThisFrame && m_activeSplatCount > 0 && !runScheduleCompaction(m_visibleScheduleEntryCount))
		{
			return false;
		}

		if (m_validateGpuCompaction && !validateGpuCompactionResult(view, projection, schedulerStats, previousSeededPath))
		{
			return false;
		}

		return true;
	}

	bool GaussianRenderer::validateGpuCompactionResult(
		const glm::mat4& view,
		const glm::mat4& projection,
		const ChunkSchedulerPipeline::DispatchStats& schedulerStats,
		bool previousSeededPath)
	{
		std::vector<std::uint32_t> expectedIndices;
		expectedIndices.reserve(m_sortCapacity);

		std::size_t expectedVisibleChunkCount = 0;
		std::size_t expectedScheduleEntryCount = 0;
		for (const SplatCacheChunkEntry& chunk : m_chunks)
		{
			if (!isChunkVisible(chunk, m_modelTransform, view, projection))
			{
				continue;
			}

			const std::size_t startIndex = chunk.start_index;
			const std::size_t endIndex = std::min(m_totalSplatCount, startIndex + static_cast<std::size_t>(chunk.splat_count));
			const std::size_t visibleChunkSplatCount = endIndex - startIndex;
			if (expectedIndices.size() > m_sortCapacity ||
				visibleChunkSplatCount > (m_sortCapacity - expectedIndices.size()))
			{
				if (m_gpuValidationLogCount < kMaxGpuValidationLogs)
				{
					++m_gpuValidationLogCount;
					emitRuntimeLog(std::cerr, "GPU_COMPACTION_VALIDATE: CPU reference exceeded sort capacity");
				}
				return false;
			}

			++expectedVisibleChunkCount;
			++expectedScheduleEntryCount;
			for (std::size_t splatIndex = startIndex; splatIndex < endIndex; ++splatIndex)
			{
				expectedIndices.push_back(static_cast<std::uint32_t>(splatIndex));
			}
		}

		const std::size_t expectedCompactedCount = expectedIndices.size();
		const float expectedVisibleRatio =
			m_totalSplatCount == 0 ? 0.0f : static_cast<float>(static_cast<double>(expectedCompactedCount) / static_cast<double>(m_totalSplatCount));

		std::size_t expectedActiveSplatCount = expectedCompactedCount;
		std::size_t expectedSortCount = 0;
		bool expectedUseSeededIndices = true;
		if (expectedCompactedCount == 0)
		{
			expectedSortCount = 0;
		}
		else if (!shouldUseSeededIndices(previousSeededPath, expectedVisibleRatio))
		{
			expectedActiveSplatCount = m_totalSplatCount;
			expectedSortCount = m_sortCapacity;
			expectedUseSeededIndices = false;
		}
		else
		{
			expectedSortCount = nextPow2(expectedActiveSplatCount);
			if (expectedSortCount > m_sortCapacity)
			{
				expectedActiveSplatCount = m_totalSplatCount;
				expectedSortCount = m_sortCapacity;
				expectedUseSeededIndices = false;
			}
		}

		const bool shouldCompareIndices = expectedUseSeededIndices;
		std::vector<std::uint32_t> gpuIndices;
		if (shouldCompareIndices && expectedCompactedCount > 0)
		{
			gpuIndices.resize(expectedCompactedCount, 0u);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_uploadBuffers.indices_buffer);
			glGetBufferSubData(
				GL_SHADER_STORAGE_BUFFER,
				0,
				static_cast<GLsizeiptr>(expectedCompactedCount * sizeof(std::uint32_t)),
				gpuIndices.data());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			const GLenum readbackErr = glGetError();
			if (readbackErr != GL_NO_ERROR)
			{
				if (m_gpuValidationLogCount < kMaxGpuValidationLogs)
				{
					++m_gpuValidationLogCount;
					emitRuntimeLog(
						std::cerr,
						"GPU_COMPACTION_VALIDATE: failed to read GPU indices, gl_error=" + std::to_string(readbackErr));
				}
				return false;
			}
		}

		const bool countsMatch =
			expectedVisibleChunkCount == schedulerStats.visible_chunk_count &&
			expectedScheduleEntryCount == schedulerStats.schedule_entry_count &&
			expectedCompactedCount == schedulerStats.active_splat_count &&
			expectedActiveSplatCount == m_activeSplatCount &&
			expectedSortCount == m_sortCount &&
			expectedUseSeededIndices == m_useSeededIndicesThisFrame;
		const bool indicesMatch = !shouldCompareIndices || gpuIndices == expectedIndices;

		if (!countsMatch || !indicesMatch)
		{
			if (m_gpuValidationLogCount < kMaxGpuValidationLogs)
			{
				++m_gpuValidationLogCount;
				emitRuntimeLog(
					std::cerr,
					"GPU_COMPACTION_VALIDATE_MISMATCH: visible_chunks gpu=" + std::to_string(schedulerStats.visible_chunk_count) +
					" cpu=" + std::to_string(expectedVisibleChunkCount) +
					" schedule_entries gpu=" + std::to_string(schedulerStats.schedule_entry_count) +
					" cpu=" + std::to_string(expectedScheduleEntryCount) +
					" compacted gpu=" + std::to_string(schedulerStats.active_splat_count) +
					" cpu=" + std::to_string(expectedCompactedCount) +
					" active gpu=" + std::to_string(m_activeSplatCount) +
					" cpu=" + std::to_string(expectedActiveSplatCount) +
					" sort gpu=" + std::to_string(m_sortCount) +
					" cpu=" + std::to_string(expectedSortCount) +
					" seeded gpu=" + std::string(m_useSeededIndicesThisFrame ? "1" : "0") +
					" cpu=" + std::string(expectedUseSeededIndices ? "1" : "0") +
					" indices_checked=" + std::string(shouldCompareIndices ? "1" : "0") +
					" indices_match=" + std::string(indicesMatch ? "1" : "0"));
			}
			return false;
		}

		if (m_gpuValidationLogCount < kMaxGpuValidationLogs)
		{
			++m_gpuValidationLogCount;
			emitRuntimeLog(
				std::cout,
				"GPU_COMPACTION_VALIDATE_OK: visible_chunks=" + std::to_string(expectedVisibleChunkCount) +
				" compacted_splats=" + std::to_string(expectedCompactedCount) +
				" sort_count=" + std::to_string(expectedSortCount));
		}

		return true;
	}

	bool GaussianRenderer::runViewDataPass(
		const glm::mat4& view,
		const glm::mat4& projection,
		float viewportWidth,
		float viewportHeight,
		bool activeDomainPreculled,
		ViewDataPipeline::DispatchStats* outStats)
	{
		const glm::mat4 invView = glm::inverse(view);
		const glm::vec3 cameraPos(invView[3].x, invView[3].y, invView[3].z);
		return m_viewDataPipeline.dispatch(
			m_uploadBuffers,
			m_inputLayout,
			view,
			m_modelTransform,
			projection,
			cameraPos,
			viewportWidth,
			viewportHeight,
			m_maxPointSize,
			activeDomainPreculled,
			m_viewDataUseScheduleDomainThisFrame,
			m_useAnisotropic,
			m_shDegree,
			m_chunkCount,
			m_visibleScheduleEntryCount,
			m_activeSplatCount,
			outStats);
	}

	void GaussianRenderer::runDepthAndSort(const glm::mat4& view)
	{
		m_depthProgram.use();
		glUniformMatrix4fv(m_depthViewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(m_depthModelLoc, 1, GL_FALSE, glm::value_ptr(m_modelTransform));
		glUniform1ui(m_depthRealCountLoc, static_cast<GLuint>(m_activeSplatCount));
		glUniform1ui(m_depthSortCountLoc, static_cast<GLuint>(m_sortCount));
		glUniform1ui(m_depthChunkCountLoc, static_cast<GLuint>(m_chunkCount));
		glUniform1ui(m_depthScheduleEntryCountLoc, static_cast<GLuint>(m_visibleScheduleEntryCount));
		glUniform1i(m_depthInputLayoutLoc, m_inputLayout);
		glUniform1i(m_depthUseSeedIndicesLoc, m_useSeededIndicesThisFrame ? 1 : 0);
		glUniform1i(m_depthUseScheduleDomainLoc, m_depthUseScheduleDomainThisFrame ? 1 : 0);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_uploadBuffers.keys_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_uploadBuffers.indices_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_uploadBuffers.chunk_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_uploadBuffers.chunk_schedule_buffer);
		if (m_inputLayout == 0)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_uploadBuffers.splat_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
		}
		else
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_uploadBuffers.position_buffer);
		}

		const GLuint groups = dispatchGroupCount(m_sortCount);
		glDispatchCompute(groups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		m_sortProgram.use();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_uploadBuffers.keys_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_uploadBuffers.indices_buffer);
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
