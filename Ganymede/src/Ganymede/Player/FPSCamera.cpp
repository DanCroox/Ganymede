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
		m_RenderView.SetNearClip(1000.0f);
		m_RenderView.SetNearClip(0.01f);

		m_MouseMoveEventCBHandle = std::make_unique<EventCallbackHandle>();

		EventSystem& eventSystem = Application::Get().GetEventSystem();
		eventSystem.SubscribeEvent<MouseMoveEvent>(*m_MouseMoveEventCBHandle, EVENT_BIND_TO_MEMBER(FPSCamera::OnMouseMoveEvent));
	}

	FPSCamera::~FPSCamera()
	{
		Application::Get().GetEventSystem().UnsubscribeEvent<MouseMoveEvent>(*m_MouseMoveEventCBHandle);
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
		m_RenderView.SetFrontVector(glm::normalize(direction));
	}
