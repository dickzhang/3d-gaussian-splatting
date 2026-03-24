#include "core/ShaderProgram.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace gs
{

	ShaderProgram::~ShaderProgram()
	{
		destroy();
	}

	bool ShaderProgram::createFromFiles(const std::string& vertexPath, const std::string& fragmentPath)
	{
		const std::string vsSource = readTextFile(vertexPath);
		const std::string fsSource = readTextFile(fragmentPath);
		if (vsSource.empty() || fsSource.empty())
		{
			return false;
		}

		const GLuint vs = compileShader(GL_VERTEX_SHADER, vsSource, vertexPath);
		const GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSource, fragmentPath);
		if (vs == 0 || fs == 0)
		{
			if (vs != 0)
			{
				glDeleteShader(vs);
			}
			if (fs != 0)
			{
				glDeleteShader(fs);
			}
			return false;
		}

		const bool ok = link(vs, fs);
		glDeleteShader(vs);
		glDeleteShader(fs);
		return ok;
	}

	bool ShaderProgram::createComputeFromFile(const std::string& computePath)
	{
		const std::string csSource = readTextFile(computePath);
		if (csSource.empty())
		{
			return false;
		}

		const GLuint cs = compileShader(GL_COMPUTE_SHADER, csSource, computePath);
		if (cs == 0)
		{
			return false;
		}

		const bool ok = linkCompute(cs);
		glDeleteShader(cs);
		return ok;
	}

	void ShaderProgram::destroy()
	{
		if (m_program != 0)
		{
			glDeleteProgram(m_program);
			m_program = 0;
		}
	}

	void ShaderProgram::use() const
	{
		glUseProgram(m_program);
	}

	bool ShaderProgram::link(GLuint vs, GLuint fs)
	{
		destroy();
		m_program = glCreateProgram();
		glAttachShader(m_program, vs);
		glAttachShader(m_program, fs);
		glLinkProgram(m_program);

		GLint status = 0;
		glGetProgramiv(m_program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint logLen = 0;
			glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
			std::vector<char> log(static_cast<std::size_t>(logLen) + 1U);
			glGetProgramInfoLog(m_program, logLen, nullptr, log.data());
			std::cerr << "Program link failed: " << log.data() << '\n';
			destroy();
			return false;
		}
		return true;
	}

	bool ShaderProgram::linkCompute(GLuint cs)
	{
		destroy();
		m_program = glCreateProgram();
		glAttachShader(m_program, cs);
		glLinkProgram(m_program);

		GLint status = 0;
		glGetProgramiv(m_program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint logLen = 0;
			glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
			std::vector<char> log(static_cast<std::size_t>(logLen) + 1U);
			glGetProgramInfoLog(m_program, logLen, nullptr, log.data());
			std::cerr << "Compute program link failed: " << log.data() << '\n';
			destroy();
			return false;
		}
		return true;
	}

	GLuint ShaderProgram::compileShader(GLenum type, const std::string& source, const std::string& debugPath)
	{
		const GLuint shader = glCreateShader(type);

		std::string effectiveSource = source;
		const std::size_t firstToken = effectiveSource.find_first_not_of(" \t\r\n");
		const bool hasVersion = firstToken != std::string::npos && effectiveSource.compare(firstToken, 8, "#version") == 0;
		if (!hasVersion)
		{
			effectiveSource = std::string("#version 430 core\n") + effectiveSource;
		}

		const char* src = effectiveSource.c_str();
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		GLint status = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint logLen = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
			std::vector<char> log(static_cast<std::size_t>(logLen) + 1U);
			glGetShaderInfoLog(shader, logLen, nullptr, log.data());
			std::cerr << "Shader compile failed [" << debugPath << "]: " << log.data() << '\n';
			glDeleteShader(shader);
			return 0;
		}

		return shader;
	}

	std::string ShaderProgram::readTextFile(const std::string& path)
	{
		std::ifstream ifs(path, std::ios::binary);
		if (!ifs.is_open())
		{
			std::cerr << "Failed to open shader file: " << path << '\n';
			return {};
		}
		std::stringstream ss;
		ss << ifs.rdbuf();
		return ss.str();
	}

} // namespace gs
