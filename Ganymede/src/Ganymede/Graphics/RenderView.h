#pragma once

#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
namespace Ganymede
{
	struct RenderView
	{
		RenderView() = delete;
		RenderView(const glm::u32vec2& renderResolution, float fov, float nearClip, float farClip, glm::uint viewGroupID, glm::uint viewID) :
			m_RenderResolution(renderResolution),
			m_FOV(fov),
			m_NearClip(nearClip),
			m_FarClip(farClip),
			m_RenderViewGroup(viewGroupID),
			m_ViewID(viewID),
			m_FaceIndex(0),  // TODO Remove once we have a proper association between Pointlights, related Views and Cubemap
			m_FrontVector({ 0.0f, 0.0f, -1.0f }),
			m_UpVector({ 0.0f, 1.0f, 0.0f }),
			m_Position({ 0.0f }),
			m_RollInDegree(0),
			m_ProjectionMatrix(glm::mat4(1)),
			m_IsProjectionMatrixDirty(true),
			m_ViewMatrix(glm::mat4(1)),
			m_IsViewMatrixDirty(true)
		{}

		glm::uint m_ViewID;
		glm::uint m_FaceIndex;
		glm::uint m_RenderViewGroup;

		glm::mat4 GetViewMatrix() const
		{
			if (m_IsViewMatrixDirty)
			{
				m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_FrontVector, m_UpVector);
				m_ViewMatrix = glm::translate(m_ViewMatrix, m_Position);
				m_ViewMatrix = glm::rotate(m_ViewMatrix, glm::radians(m_RollInDegree), m_FrontVector);
				m_ViewMatrix = glm::translate(m_ViewMatrix, -m_Position);
				m_IsViewMatrixDirty = false;
			}

			return m_ViewMatrix;
		}

		glm::mat4 GetProjectionMatrix() const
		{
			if (m_IsProjectionMatrixDirty)
			{
				m_ProjectionMatrix = glm::perspective(
					glm::radians(m_FOV),
					static_cast<float>(m_RenderResolution.x) / static_cast<float>(m_RenderResolution.y),
					m_NearClip,
					m_FarClip
				);
				m_IsProjectionMatrixDirty = false;
			}

			return m_ProjectionMatrix;
		}

		float GetNearClip() const { return m_NearClip; }
		void SetNearClip(float nearClip)
		{
			m_NearClip = nearClip;
			m_IsProjectionMatrixDirty = true;
		}

		float GetFarClip() const { return m_FarClip; }
		void SetFarClip(float farClip)
		{
			m_FarClip = farClip;
			m_IsProjectionMatrixDirty = true;
		}

		float GetFOV() const { return m_FOV; }
		void SetFOV(float fov)
		{
			m_FOV = fov;
			m_IsProjectionMatrixDirty = true;
		}

		const glm::u32vec2& GetRenderResolution() const { return m_RenderResolution; }
		void SetRenderResolution(const glm::u32vec2& resolution)
		{
			m_RenderResolution = resolution;
			m_IsProjectionMatrixDirty = true;
		}

		void SetRenderResolution(uint32_t width, uint32_t height)
		{
			m_RenderResolution.x = width;
			m_RenderResolution.y = height;
			m_IsProjectionMatrixDirty = true;
		}

		void SetFrontVector(glm::vec3 vector)
		{
			m_FrontVector = vector;
			m_IsViewMatrixDirty = true;
		}

		const glm::vec3& GetFrontVector() const
		{
			return m_FrontVector;
		}

		void SetUpVector(glm::vec3 vector)
		{
			m_UpVector = vector;
			m_IsViewMatrixDirty = true;
		}

		const glm::vec3& GetUpVector() const
		{
			return m_UpVector;
		}
		
		void SetPosition(glm::vec3 vector)
		{
			m_Position = vector;
			m_IsViewMatrixDirty = true;
		}

		const glm::vec3& GetPosition() const
		{
			return m_Position;
		}

		void SetRollInDegree(float roll)
		{
			m_RollInDegree = roll;
			m_IsViewMatrixDirty = true;
		}

		float GetRollInDegree() const
		{
			return m_RollInDegree;
		}

	private:
		float m_NearClip;
		float m_FarClip;
		float m_FOV;
		glm::u32vec2 m_RenderResolution;

		glm::vec3 m_FrontVector;
		glm::vec3 m_UpVector;
		glm::vec3 m_Position;
		float m_RollInDegree;

		mutable glm::mat4 m_ProjectionMatrix;
		mutable bool m_IsProjectionMatrixDirty;

		mutable glm::mat4 m_ViewMatrix;
		mutable bool m_IsViewMatrixDirty;
	};
}