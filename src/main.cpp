#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "io/PlyLoader.h"
#include "render/GaussianRenderer.h"
#include "scene/Camera.h"
#include "scene/GaussianModel.h"

namespace
{

	std::string readModelPathFromConfig()
	{
		std::ifstream ifs("assets/configs/model_path.txt");
		if (!ifs.is_open())
		{
			return {};
		}
		std::string line;
		std::getline(ifs, line);
		return line;
	}

	bool readEnvFlag(const char* name)
	{
		char* value = nullptr;
		std::size_t size = 0;
		if (_dupenv_s(&value, &size, name) != 0 || value == nullptr)
		{
			return false;
		}

		const bool enabled = value[0] != '\0' && value[0] != '0';
		std::free(value);
		return enabled;
	}

} // namespace

int main()
{
	if (!glfwInit())
	{
		std::cerr << "glfwInit failed\n";
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1600, 900, "3D Gaussian Splatting (OpenGL Compute)", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Failed to create window\n";
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "gladLoadGLLoader failed\n";
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
		std::cerr << "OpenGL 4.3+ is required, current: " << major << "." << minor << "\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	std::string modelPathText = readModelPathFromConfig();
	if (modelPathText.empty())
	{
		std::cerr << "Missing model path. Set assets/configs/model_path.txt\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	gs::GaussianModel model;
	if (!gs::PlyLoader::loadGaussianPly(std::filesystem::u8path(modelPathText), model))
	{
		std::cerr << "Failed to load model from: " << modelPathText << "\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	gs::GaussianRenderer renderer;
	if (!renderer.initialize() || !renderer.uploadModel(model))
	{
		std::cerr << "Failed to initialize renderer\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}
	renderer.setUseAnisotropic(true);
	renderer.setShDegree(renderer.maxSupportedShDegree());
	std::cout << "Model SH max degree: " << renderer.maxSupportedShDegree() << "\n";
	std::cout << "Default visual mode: anisotropic ON, SH degree " << renderer.shDegree() << "\n";
	std::cout << "Look mode: reference-like (press R to toggle original/reference)\n";

	gs::Camera camera;
	float prevTime = static_cast<float>(glfwGetTime());
	while (!glfwWindowShouldClose(window))
	{
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
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
