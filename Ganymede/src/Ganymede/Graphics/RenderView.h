#pragma once

#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
namespace Ganymede
{
	struct RenderView
	{
		float m_RollInDegree = 0;
		glm::vec3 m_FrontVector = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 m_UpVector = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 m_RightVector = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 m_Position = glm::vec3(0.0f);

		glm::mat4 m_Perspective;
		float m_NearClip;
		float m_FarClip;
		glm::uint m_ViewID;
		glm::uint m_FaceIndex;
		glm::uint m_RenderViewGroup;

		glm::mat4 ToTransform() const
		{
			glm::mat4 mat = glm::lookAt(m_Position, m_Position + m_FrontVector, m_UpVector);
			mat = glm::translate(mat, m_Position);
			mat = glm::rotate(mat, glm::radians(m_RollInDegree), m_FrontVector);
			mat = glm::translate(mat, -m_Position);
			return mat;
		}
	};
}