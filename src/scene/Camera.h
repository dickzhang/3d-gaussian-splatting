#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct GLFWwindow;

namespace gs
{

	// 相机类：管理相机姿态、输入更新与视图/投影矩阵计算
	class Camera
	{
	public:
		// 相机姿态：位置 + 欧拉角
		struct Pose
		{
			glm::vec3 position{ 0.0f, 0.0f, 0.0f }; // 相机世界坐标
			float yaw{ 0.0f };                      // 偏航角（绕 Y 轴）
			float pitch{ 0.0f };                    // 俯仰角（绕 X 轴）
		};

		// 构造并初始化默认相机参数
		Camera();

		// 根据输入和时间步更新相机状态
		void update(GLFWwindow* window, float dt);
		// 获取视图矩阵
		glm::mat4 viewMatrix() const;
		// 获取投影矩阵（需要传入宽高比）
		glm::mat4 projectionMatrix(float aspect) const;
		// 获取当前姿态
		Pose pose() const;
		// 设置姿态
		void setPose(const Pose& value);

		// 获取当前位置
		glm::vec3 position() const
		{
			return m_position;
		}
	private:
		// 根据 yaw/pitch 计算前向单位向量
		glm::vec3 computeForward() const;
		glm::vec3 m_position; // 相机位置
		float m_yaw;          // 偏航角
		float m_pitch;        // 俯仰角
		float m_fovDeg;       // 垂直视场角（度）
		bool m_firstMouse;    // 右键旋转时是否为首帧
		bool m_mouseCaptured; // 右键旋转期间是否已捕获鼠标
		double m_lastMouseX;  // 上一帧鼠标 X
		double m_lastMouseY;  // 上一帧鼠标 Y
	};

} // namespace gs
