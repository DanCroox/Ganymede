#pragma once
#include "Ganymede/Core/Core.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Ganymede
{
	struct GCTransform
	{
		const glm::mat4& GetMatrix() const
		{
			if (m_IsDirty)
			{
				m_CachedMatrix = glm::translate(glm::mat4(1.0f), m_Position)
					* glm::mat4_cast(m_Rotation)
					* glm::scale(glm::mat4(1.0f), m_Scale);
				m_IsDirty = false;
			}

			return m_CachedMatrix;
		}

		void SetMatrix(const glm::mat4& transform)
		{
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(transform, m_Scale, m_Rotation, m_Position, skew, perspective);
			m_CachedMatrix = transform;
			m_IsDirty = false;
		}

		void SetPosition(glm::vec3 position)
		{
			m_Position = position;
			m_IsDirty = true;
		}
		
		void SetPosition(float x, float y, float z)
		{
			m_Position = { x, y, z };
			m_IsDirty = true;
		}

		void SetScale(glm::vec3 scale)
		{
			m_Scale = scale;
			m_IsDirty = true;
		}
		
		void SetScale(float scale)
		{
			m_Scale = { scale, scale, scale };
			m_IsDirty = true;
		}

		void SetRotation(glm::quat rotation)
		{
			m_Rotation = rotation;
			m_IsDirty = true;
		}

		void SetEulerRotation(const glm::vec3& eulerRotation)
		{
			// GLM uses Euler ZYX so we swizzle
			m_Rotation = glm::quat(glm::radians(glm::vec3(eulerRotation.z, eulerRotation.y, eulerRotation.z )));
			m_IsDirty = true;
		}

		glm::vec3 GetPosition() const { return m_Position; }
		glm::vec3 GetScale() const { return m_Scale; }
		glm::quat GetRotation() const { return m_Rotation; }
		glm::vec3 getEulerRotation() const { return glm::degrees(glm::eulerAngles(m_Rotation));}

	private:
		glm::vec3 m_Position{ 0.0f };
		glm::vec3 m_Scale{ 1.0f };
		glm::quat m_Rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

		mutable glm::mat4 m_CachedMatrix{ 1.0f };
		mutable bool m_IsDirty{ true };
	};
}