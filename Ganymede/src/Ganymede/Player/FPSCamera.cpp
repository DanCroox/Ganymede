#include "FPSCamera.h"
#include "Ganymede/Core/Application.h"
#include "Ganymede/World/World.h"
#include "Ganymede/Events/Event.h"
#include "Ganymede/Runtime/WindowEvents.h"

#include <GLFW/glfw3.h>
#include <iostream>

using namespace Ganymede;

	FPSCamera::FPSCamera(RenderView& renderView) :
		m_RenderView(renderView)
	{
		m_RenderView.m_FarClip = 1000.0f;
		m_RenderView.m_NearClip = 0.1f;
		RecreateProjectionMatrix();

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
		m_RenderView.m_FrontVector = glm::normalize(direction);
		m_RenderView.m_RightVector = glm::cross(m_RenderView.m_FrontVector, m_RenderView.m_UpVector);
	}

	const glm::mat4& FPSCamera::GetProjection() const
	{
		if (m_RecreateProjectionMatrix)
		{
			RecreateProjectionMatrix();
			m_RecreateProjectionMatrix = false;
		}

		return m_RenderView.m_Perspective;
	}

	void FPSCamera::RecreateProjectionMatrix() const
	{	
		//REWORK: Do proper aspect ratio creatíon! the cam needs the screen size. Maybe needs entire rework. Not sure if this shoudl be done here.
		m_RenderView.m_Perspective = glm::perspective(
			glm::radians(m_FOV),													// Vertical FOV angle
			static_cast<float>(1920) / static_cast<float>(1080),  // Screen Aspect Ratio
			m_RenderView.m_NearClip,																		// Near clipping plane
			m_RenderView.m_FarClip																		// Far clipping plane
		);
	}
