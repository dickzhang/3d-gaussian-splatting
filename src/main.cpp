#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <cctype>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

#include "core/PathUtils.h"
#include "render/GaussianRenderer.h"
#include "runtime/RuntimeSplatAssetLoader.h"
#include "scene/Camera.h"

namespace
{

	std::string trim_copy(std::string value)
	{
		const auto is_space = [](unsigned char c)
		{
			return std::isspace(c) != 0;
		};

		auto first = value.begin();
		auto last = value.end();
		while (first != last && is_space(static_cast<unsigned char>(*first)))
		{
			++first;
		}
		while (first != last && is_space(static_cast<unsigned char>(*(last - 1))))
		{
			--last;
		}
		value = std::string(first, last);

		if (value.size() >= 3 &&
			static_cast<unsigned char>(value[0]) == 0xEF &&
			static_cast<unsigned char>(value[1]) == 0xBB &&
			static_cast<unsigned char>(value[2]) == 0xBF)
		{
			value.erase(0, 3);
		}

		return value;
	}

	std::string readModelPathFromConfig(const std::filesystem::path& config_path)
	{
		std::ifstream ifs(config_path);
		if (!ifs.is_open())
		{
			return {};
		}

		std::string line;
		while (std::getline(ifs, line))
		{
			line = trim_copy(line);
			if (!line.empty() && line[0] != '#')
			{
				return line;
			}
		}

		return {};
	}

	std::filesystem::path resolveConfiguredPath(const std::filesystem::path& config_path, const std::string& configured_value)
	{
		std::filesystem::path path = gs::pathFromText(configured_value);
		if (path.is_relative())
		{
			path = config_path.parent_path() / path;
		}
		return path.lexically_normal();
	}

	std::string readEnvValue(const char* name)
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

	bool readEnvFlag(const char* name)
	{
		const std::string value = readEnvValue(name);
		return !value.empty() && value[0] != '0';
	}

	glm::mat4 defaultReferenceModelTransform()
	{
		// Match the common Unity sample setup: rotate around X then mirror Z.
		glm::mat4 model(1.0f);
		model = glm::rotate(model, glm::radians(-160.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, -1.0f));
		return model;
	}

	void emitRuntimeLog(std::ostream& stream, const std::string& message)
	{
		stream << message << '\n';
		stream.flush();

		const std::string logPath = readEnvValue("GS_RUNTIME_LOG_FILE");
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

int main()
{
	try
	{
		if (!glfwInit())
		{
			emitRuntimeLog(std::cerr, "glfwInit failed");
			return 1;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWwindow* window = glfwCreateWindow(1600, 900, "3D Gaussian Splatting (OpenGL Compute)", nullptr, nullptr);
		if (!window)
		{
			emitRuntimeLog(std::cerr, "Failed to create window");
			glfwTerminate();
			return 1;
		}

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);

		if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
		{
			emitRuntimeLog(std::cerr, "gladLoadGLLoader failed");
			glfwDestroyWindow(window);
			glfwTerminate();
			return 1;
		}

		GLint major = 0;
		GLint minor = 0;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		if (major < 4 || (major == 4 && minor < 3))
		{
			emitRuntimeLog(std::cerr, "OpenGL 4.3+ is required, current: " + std::to_string(major) + "." + std::to_string(minor));
			glfwDestroyWindow(window);
			glfwTerminate();
			return 1;
		}

		const std::filesystem::path configPath = "assets/configs/model_path.txt";
		std::string modelPathText = readModelPathFromConfig(configPath);
		if (modelPathText.empty())
		{
			emitRuntimeLog(std::cerr, "Missing model path. Set assets/configs/model_path.txt");
			glfwDestroyWindow(window);
			glfwTerminate();
			return 1;
		}

		const std::filesystem::path cachePath = resolveConfiguredPath(configPath, modelPathText);
		if (cachePath.extension() != ".gsplatcache")
		{
			emitRuntimeLog(std::cerr, "Runtime now requires a .gsplatcache path in assets/configs/model_path.txt");
			emitRuntimeLog(std::cerr, "Current value: " + gs::pathToUtf8(cachePath));
			emitRuntimeLog(std::cerr, "Generate cache with: ./build/Release/gs_cache_builder.exe <input.ply>");
			emitRuntimeLog(std::cerr, "Default output is next to source PLY as <name>.gsplatcache");
			glfwDestroyWindow(window);
			glfwTerminate();
			return 1;
		}

		gs::RuntimeSplatAsset runtimeAsset;
		std::string cacheLoadError;
		if (!gs::RuntimeSplatAssetLoader::load_from_cache(cachePath, runtimeAsset, &cacheLoadError))
		{
			emitRuntimeLog(std::cerr, "Failed to load cache asset: " + gs::pathToUtf8(cachePath));
			if (!cacheLoadError.empty())
			{
				emitRuntimeLog(std::cerr, cacheLoadError);
			}
			emitRuntimeLog(std::cerr, "Ensure cache file exists and matches runtime cache format version");
			glfwDestroyWindow(window);
			glfwTerminate();
			return 1;
		}

		gs::GaussianRenderer renderer;
		if (!renderer.initialize() || !renderer.uploadAsset(runtimeAsset))
		{
			emitRuntimeLog(std::cerr, "Failed to initialize renderer");
			glfwDestroyWindow(window);
			glfwTerminate();
			return 1;
		}
		renderer.setUseAnisotropic(true);
		renderer.setShDegree(renderer.maxSupportedShDegree());
		renderer.setModelTransform(defaultReferenceModelTransform());
		emitRuntimeLog(std::cout, "Loaded cache: " + gs::pathToUtf8(cachePath));
		emitRuntimeLog(std::cout, "Cache splats: " + std::to_string(runtimeAsset.splat_count));
		emitRuntimeLog(std::cout, "Cache chunks: " + std::to_string(runtimeAsset.chunks.size()));
		emitRuntimeLog(std::cout, "Model SH max degree: " + std::to_string(renderer.maxSupportedShDegree()));
		emitRuntimeLog(std::cout, "Default visual mode: anisotropic ON, SH degree " + std::to_string(renderer.shDegree()));
		emitRuntimeLog(std::cout, "Object transform: rotate X -160 deg, mirror Z");

		gs::Camera camera;
		float prevTime = static_cast<float>(glfwGetTime());
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			if (glfwWindowShouldClose(window))
			{
				break;
			}

			const float now = static_cast<float>(glfwGetTime());
			const float dt = now - prevTime;
			prevTime = now;
			camera.update(window, dt);
			int width = 1;
			int height = 1;
			glfwGetFramebufferSize(window, &width, &height);
			glViewport(0, 0, width, height);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			const float aspect = (width > 0 && height > 0) ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
			renderer.render(camera.viewMatrix(), camera.projectionMatrix(aspect), static_cast<float>(width), static_cast<float>(height));

			glfwSwapBuffers(window);
		}

		glfwDestroyWindow(window);
		glfwTerminate();
		return 0;
	}
	catch (const std::exception& ex)
	{
		emitRuntimeLog(std::cerr, std::string("Unhandled exception: ") + ex.what());
		glfwTerminate();
		return 1;
	}
	catch (...)
	{
		emitRuntimeLog(std::cerr, "Unhandled non-standard exception");
		glfwTerminate();
		return 1;
	}
}
