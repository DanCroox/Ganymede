#include "FPSCamera.h"
#include "Ganymede/Core/Application.h"
#include "Ganymede/World/World.h"
#include "Ganymede/Events/Event.h"
#include "Ganymede/Runtime/WindowEvents.h"


#include <GLFW/glfw3.h>
#include <iostream>

using namespace Ganymede;

	FPSCamera::FPSCamera()
	{
		m_MouseMoveEventCBHandle = std::make_unique<EventCallbackHandle>();

		EventSystem& eventSystem = Application::Get().GetEventSystem();
		eventSystem.SubscribeEvent<MouseMoveEvent>(*m_MouseMoveEventCBHandle, EVENT_BIND_TO_MEMBER(FPSCamera::OnMouseMoveEvent));
	}

	FPSCamera::~FPSCamera()
	{
		//EventSystem::GetInstance().UnsubscribeEvent<MouseMoveEvent>(m_MoveMoveEventCBHandle);
	}

	void FPSCamera::OnMouseMoveEvent(MouseMoveEvent& event)
	{
		m_CursorPosX = event.GetPosition().x;
		m_CursorPosY = event.GetPosition().y;
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




		const float speed = m_MouseSensitiviy;
		glm::vec2 mouseDelta = glm::vec2(m_CursorPosX - m_LastCursorX, m_LastCursorY - m_CursorPosY) * speed;
		m_LastCursorX = m_CursorPosX;
		m_LastCursorY = m_CursorPosY;

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
		//REWORK: Do proper aspect ratio creatíon! the cam needs the screen size. Maybe needs entire rework. Not sure if this shoudl be done here.
		m_Projection = glm::perspective(
			glm::radians(m_FOV),													// Vertical FOV angle
			static_cast<float>(1920) / static_cast<float>(1080),  // Screen Aspect Ratio
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
