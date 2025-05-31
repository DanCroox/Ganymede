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

		void OnMouseMoveEvent(MouseMoveEvent& event);

		inline void SetPosition(const glm::vec3& position) { m_RenderView.SetPosition(position); }
		inline const glm::vec3& GetPosition() const { return m_RenderView.GetPosition(); }
		inline const glm::vec3& GetFrontVector() const { return m_RenderView.GetFrontVector(); }
		inline const glm::vec3& GetUpVector() const { return m_RenderView.GetUpVector(); }
		inline const glm::vec3& GetRightVector() const { return glm::normalize(glm::cross(GetFrontVector(), GetUpVector())); }

		void SetRollInDegree(float roll) { m_RenderView.SetRollInDegree(roll); }
		float GetRollInDegree() const { return m_RenderView.GetRollInDegree(); }

	private:
		RenderView& m_RenderView;
		std::unique_ptr<EventCallbackHandle> m_MouseMoveEventCBHandle;

		float m_MouseSensitiviy = .015f;

		double m_CursorPosX = 0;
		double m_CursorPosY = 0;
		float m_Yaw = -90;
		float m_Pitch = 0;

		double m_LastCursorX = 0;
		double m_LastCursorY = 0;
	};
}