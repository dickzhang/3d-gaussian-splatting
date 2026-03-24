#include "scene/Camera.h"

#include <algorithm>
#include <cmath>

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace gs
{

	Camera::Camera(): 
		m_position(0,0, 0),
		m_yaw(0),
		m_pitch(0),
		m_fovDeg(60.0f),
		m_firstMouse(true),
		m_lastMouseX(0.0),
		m_lastMouseY(0.0)
	{
	}

	void Camera::update(GLFWwindow* window, float dt)
	{
		const float speed = 4.0f;
		const float sensitivity = 0.08f;

		const glm::vec3 forward = computeForward();

		const glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			m_position += forward * speed * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			m_position -= forward * speed * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			m_position -= right * speed * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			m_position += right * speed * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			m_position.y += speed * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			m_position.y -= speed * dt;
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			double mouseX = 0.0;
			double mouseY = 0.0;
			glfwGetCursorPos(window, &mouseX, &mouseY);

			if (m_firstMouse)
			{
				m_lastMouseX = mouseX;
				m_lastMouseY = mouseY;
				m_firstMouse = false;
			}

			const float offsetX = static_cast<float>(mouseX - m_lastMouseX);
			const float offsetY = static_cast<float>(m_lastMouseY - mouseY);
			m_lastMouseX = mouseX;
			m_lastMouseY = mouseY;

			m_yaw += offsetX * sensitivity;
			m_pitch += offsetY * sensitivity;
		}
		else
		{
			m_firstMouse = true;
		}
	}

	glm::mat4 Camera::viewMatrix() const
	{
		return glm::lookAt(m_position, m_position + computeForward(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::mat4 Camera::projectionMatrix(float aspect) const
	{
		return glm::perspective(glm::radians(m_fovDeg), aspect, 0.01f, 500.0f);
	}

	Camera::Pose Camera::pose() const
	{
		return Pose{ m_position, m_yaw, m_pitch };
	}

	void Camera::setPose(const Pose& value)
	{
		if (!std::isfinite(value.position.x) || !std::isfinite(value.position.y) || !std::isfinite(value.position.z))
		{
			return;
		}

		m_position = value.position;
		m_yaw = value.yaw;
		m_pitch = value.pitch;
		m_firstMouse = true;
	}

	glm::vec3 Camera::computeForward() const
	{
		glm::vec3 forward;
		forward.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		forward.y = sin(glm::radians(m_pitch));
		forward.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		return glm::normalize(forward);
	}

} // namespace gs
