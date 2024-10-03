#include "FPSCamera.h"
#include "Ganymede/World/World.h"
#include "Ganymede/World/PointlightWorldObjectInstance.h"
#include <GLFW/glfw3.h>
#include <iostream>


FPSCamera::FPSCamera(GLFWwindow* glfWwindow)
{
	m_GlfWwindow = glfWwindow;
	glfwSetInputMode(m_GlfWwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(m_GlfWwindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}

void FPSCamera::Update(float deltaTime)
{
	/*
	const float cameraSpeed = 10 * deltaTime;
	if (glfwGetKey(Globals::glfWindow, GLFW_KEY_W) == GLFW_PRESS)
		m_Position += cameraSpeed * m_FrontVector;
	if (glfwGetKey(Globals::glfWindow, GLFW_KEY_S) == GLFW_PRESS)
		m_Position -= cameraSpeed * m_FrontVector;
	if (glfwGetKey(Globals::glfWindow, GLFW_KEY_A) == GLFW_PRESS)
		m_Position -= glm::normalize(glm::cross(m_FrontVector, m_UpVector)) * cameraSpeed;
	if (glfwGetKey(Globals::glfWindow, GLFW_KEY_D) == GLFW_PRESS)
		m_Position += glm::normalize(glm::cross(m_FrontVector, m_UpVector)) * cameraSpeed;
		*/
		
	double cursorX;
	double cursorY;
	glfwGetCursorPos(m_GlfWwindow, &cursorX, &cursorY);

	const float speed = m_MouseSensitiviy;
	glm::vec2 mouseDelta = glm::vec2(cursorX - m_LastCursorX, m_LastCursorY - cursorY) * speed;
	m_LastCursorX = cursorX;
	m_LastCursorY = cursorY;

	if (m_IsFirstExecution)
	{
		m_IsFirstExecution = false;
		return;
	}

	m_Yaw += mouseDelta.x;
	m_Pitch += mouseDelta.y;

	if (m_Pitch > 89.0f)
		m_Pitch = 89.0f;
	if (m_Pitch < -89.0f)
		m_Pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	direction.y = sin(glm::radians(m_Pitch));
	direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	m_FrontVector = glm::normalize(direction);
	m_RightVector = glm::cross(m_FrontVector, m_UpVector);

	m_Frustum = CreateFrustum();
}

const glm::mat4& FPSCamera::GetProjection() const
{
	if (m_RecreateProjectionMatrix)
	{
		RecreateProjectionMatrix();
		m_RecreateProjectionMatrix = false;
	}

	return m_Projection;
}

void FPSCamera::RecreateProjectionMatrix() const
{
	m_Projection = glm::perspective(
		glm::radians(m_FOV),													// Vertical FOV angle
		static_cast<float>(m_ScreenSize.x) / static_cast<float>(m_ScreenSize.y),  // Screen Aspect Ratio
		m_NearClip,																		// Near clipping plane
		m_FarClip																		// Far clipping plane
	);
}

FPSCamera::Frustum FPSCamera::CreateFrustum()
{
	FPSCamera::Frustum frustum;
	
	const float aspect = m_ScreenSize.x / m_ScreenSize.y;
	const float halfVSide = m_FarClip * tanf(m_FOV * .5f);
	const float halfHSide = halfVSide * aspect;
	const glm::vec3 frontMultFar = m_FarClip * m_FrontVector;

	frustum.nearFace = { m_Position + m_NearClip * m_FrontVector, m_FrontVector };
	frustum.farFace = { m_Position + frontMultFar, -m_FrontVector };
	frustum.rightFace = { m_Position,
							glm::cross(frontMultFar - m_RightVector * halfHSide, m_UpVector) };
	frustum.leftFace = { m_Position,
							glm::cross(m_UpVector,frontMultFar + m_RightVector * halfHSide) };
	frustum.topFace = { m_Position,
							glm::cross(m_RightVector, frontMultFar - m_UpVector * halfVSide) };
	frustum.bottomFace = { m_Position,
							glm::cross(frontMultFar + m_UpVector * halfVSide, m_RightVector) };

	return frustum;
}