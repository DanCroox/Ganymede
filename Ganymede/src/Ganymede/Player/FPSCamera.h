#pragma once

#include "Ganymede/Core/Core.h"

#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Ganymede
{
	class EventCallbackHandle;
	class MouseMoveEvent;

	class GANYMEDE_API FPSCamera
	{
	public:
		struct Plane
		{
			glm::vec3 normal = { 0.f, 1.f, 0.f }; // unit vector
			float     distance = 0.f;        // Distance with origin

			Plane() = default;

			Plane(const glm::vec3& p1, const glm::vec3& norm)
				: normal(glm::normalize(norm)),
				distance(glm::dot(normal, p1))
			{}

			float getSignedDistanceToPlane(const glm::vec3& point) const
			{
				return glm::dot(normal, point) - distance;
			}
		};

		struct Frustum
		{
			Plane topFace;
			Plane bottomFace;

			Plane rightFace;
			Plane leftFace;

			Plane farFace;
			Plane nearFace;
		};


		FPSCamera();
		~FPSCamera();

		void Update(float deltaTime);

		std::unique_ptr<EventCallbackHandle> m_MouseMoveEventCBHandle;
		void OnMouseMoveEvent(MouseMoveEvent& event);

		const glm::mat4 GetTransform() const
		{
			// TODO: Cache transform is pos, and vectors didnt change
			glm::mat4 mat = glm::lookAt(m_Position, m_Position + m_FrontVector, m_UpVector);
			mat = glm::translate(mat, m_Position);
			mat = glm::rotate(mat, glm::radians(m_RollInDegree), m_FrontVector);
			mat = glm::translate(mat, -m_Position);

			return mat;
		}

		void SetPosition(glm::vec3 position) { m_Position = position; }
		glm::vec3 GetPosition() const { return m_Position; }
		glm::vec3 GetFrontVector() const { return m_FrontVector; }
		glm::vec3 GetUpVector() const { return m_UpVector; }

		void SetRollInDegree(float roll) { m_RollInDegree = roll; }
		float GetRollInDegree() const { return m_RollInDegree; }

		const glm::mat4& GetProjection() const;

		void SetFOV(float fov) { m_FOV = fov; m_RecreateProjectionMatrix = true; }
		void SetNearClip(float nearclip) { m_NearClip = nearclip; m_RecreateProjectionMatrix = true; }
		void SetFarClip(float farclip) { m_FarClip = farclip; m_RecreateProjectionMatrix = true; }

		glm::u32vec2 GetScreenSize() const { return m_ScreenSize; }
		float GetFOV() const { return m_FOV; }
		float GetNearClip() const { return m_NearClip; }
		float GetFarClip() const { return m_FarClip; }
		const Frustum& GetFrustum() const { return m_Frustum; }

	private:
		void RecreateProjectionMatrix() const;
		Frustum CreateFrustum();

		double m_CursorPosX = 0;
		double m_CursorPosY = 0;

		glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 m_FrontVector = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 m_UpVector = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 m_RightVector = glm::vec3(1.0f, 0.0f, 0.0f);

		glm::vec3 position = glm::vec3(0);
		float m_RollInDegree = 0.0f;

		mutable glm::mat4 m_Projection = glm::mat4(1.f);

		mutable bool m_RecreateProjectionMatrix = true;

		// 
		glm::u32vec2 m_ScreenSize = glm::u32vec2(1920, 1080);
		float m_FOV = 65.f;
		float m_NearClip = 0.01f;
		float m_FarClip = 1000.f;

		float m_MouseSensitiviy = .015f;

		float m_Yaw = -90;
		float m_Pitch = 0;

		bool m_IsFirstExecution = true;

		double m_LastCursorX = 0;
		double m_LastCursorY = 0;

		Frustum m_Frustum;
	};
}