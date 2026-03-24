#pragma once

#include <string>

#include <glad/glad.h>

namespace gs
{

	// Shader程序封装：负责着色器源码加载、编译、链接与绑定使用
	class ShaderProgram
	{
	public:
		// 默认构造
		ShaderProgram() = default;
		// 析构时释放OpenGL程序对象
		~ShaderProgram();

		// 禁止拷贝，避免OpenGL句柄被重复管理
		ShaderProgram(const ShaderProgram&) = delete;
		ShaderProgram& operator=(const ShaderProgram&) = delete;

		// 从顶点/片元着色器文件创建图形程序
		bool createFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
		// 从计算着色器文件创建计算程序
		bool createComputeFromFile(const std::string& computePath);
		// 显式销毁当前程序对象
		void destroy();
		// 绑定当前程序到OpenGL上下文
		void use() const;

		// 获取底层OpenGL程序ID
		GLuint id() const
		{
			return m_program;
		}

	private:
		// 链接顶点+片元着色器
		bool link(GLuint vs, GLuint fs);
		// 链接计算着色器
		bool linkCompute(GLuint cs);
		// 编译单个着色器源码
		static GLuint compileShader(GLenum type, const std::string& source, const std::string& debugPath);
		// 读取文本文件内容（着色器源码）
		static std::string readTextFile(const std::string& path);
		// OpenGL程序对象句柄（0表示无效）
		GLuint m_program{ 0 };
	};

} // namespace gs
