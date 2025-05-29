#pragma once

#include "Ganymede/Core/Core.h"

#include "Ganymede/Graphics/RenderView.h"
#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Ganymede
{
	class EventCallbackHandle;
	class MouseMoveEvent;
	class RenderView;

	class GANYMEDE_API FPSCamera
	{
	public:
		FPSCamera() = delete;
		FPSCamera(RenderView& renderView);
		~FPSCamera();

		void Update(float deltaTime);

		std::unique_ptr<EventCallbackHandle> m_MouseMoveEventCBHandle;
		void OnMouseMoveEvent(MouseMoveEvent& event);

		inline void SetPosition(glm::vec3 position) { m_RenderView.m_Position = position; }
		inline glm::vec3 GetPosition() const { return m_RenderView.m_Position; }
		inline glm::vec3 GetFrontVector() const { return m_RenderView.m_FrontVector; }
		inline glm::vec3 GetUpVector() const { return m_RenderView.m_UpVector; }
		inline glm::vec3 GetRightVector() const { return glm::normalize(glm::cross(GetFrontVector(), GetUpVector())); }

		void SetRollInDegree(float roll) { m_RenderView.m_RollInDegree = roll; }
		float GetRollInDegree() const { return m_RenderView.m_RollInDegree; }

		const glm::mat4& GetProjection() const;

		void SetFOV(float fov) { m_FOV = fov; m_RecreateProjectionMatrix = true; }
		void SetNearClip(float nearclip) { m_RenderView.m_NearClip = nearclip; m_RecreateProjectionMatrix = true; }
		void SetFarClip(float farclip) { m_RenderView.m_FarClip = farclip; m_RecreateProjectionMatrix = true; }

		glm::u32vec2 GetScreenSize() const { return m_ScreenSize; }
		float GetFOV() const { return m_FOV; }
		float GetNearClip() const { return m_RenderView.m_NearClip; }
		float GetFarClip() const { return m_RenderView.m_FarClip; }

	private:
		void RecreateProjectionMatrix() const;

		RenderView& m_RenderView;

		double m_CursorPosX = 0;
		double m_CursorPosY = 0;

		mutable bool m_RecreateProjectionMatrix = true;

		glm::u32vec2 m_ScreenSize = glm::u32vec2(1920, 1080);
		float m_FOV = 65.f;

		float m_MouseSensitiviy = .015f;

		float m_Yaw = -90;
		float m_Pitch = 0;

		bool m_IsFirstExecution = true;

		double m_LastCursorX = 0;
		double m_LastCursorY = 0;
	};
}