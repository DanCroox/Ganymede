#include "MeshWorldObjectInstanceDoor.h"

namespace Ganymede
{
	void MeshWorldObjectInstanceDoor::OnCreate()
	{
		m_DoorCloseRotation = GetEulerAngle().y;
		m_DoorOpenRotation = m_DoorCloseRotation + 1.57079633f;
	}

	void MeshWorldObjectInstanceDoor::Tick(float deltaTime)
	{
		if (m_IsDoorOpen)
		{
			m_DoorOpenInterpolator += (1.f - m_DoorOpenInterpolator) * 0.5f * (deltaTime * 5.f);
		}
		else
		{
			m_DoorOpenInterpolator -= (m_DoorOpenInterpolator) * 0.5f * (deltaTime * 4.f);
		}

		m_DoorOpenInterpolator = glm::clamp(m_DoorOpenInterpolator, 0.f, 1.f);

		const float rotation = glm::mix(m_DoorCloseRotation, m_DoorOpenRotation, m_DoorOpenInterpolator);

		//std::cout << m_DoorOpenRotation << "\n";

		glm::vec3 rot = GetEulerAngle();
		rot.x = rotation;
		SetEulerAngle(rot);

		// REWORK: I replaced btTransform by glm mat4 calculation. not tested yet!
		glm::mat4 tr = m_RigidBody.GetCenterOfMassTransform();
		glm::quat quat(rot);
		tr = tr * glm::mat4(quat);

		m_RigidBody.SetCenterOfMassTransform(tr);
	}
}