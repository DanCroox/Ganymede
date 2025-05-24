#include "GSDoor.h"

#include "Ganymede/ECS/Components/GCDoor.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/ECS/Components/GCRigidBody.h"
#include "Ganymede/World/World.h"

namespace Ganymede
{
	void GSDoor::Initialize(World& world, entt::entity entity, float deltaTime)
	{
		std::optional<GCDoor> gcDoor = world.GetComponentFromEntity<GCDoor>(entity);
		std::optional<GCTransform> gcTransform = world.GetComponentFromEntity<GCTransform>(entity);

		gcDoor.value().m_DoorCloseRotation = gcTransform.value().getEulerRotation().y;
		gcDoor.value().m_DoorOpenRotation = gcDoor.value().m_DoorCloseRotation + 1.57079633f;
	}

	void GSDoor::Tick(World& world, entt::entity entity, float deltaTime)
	{
		std::optional<GCDoor> optGCDoor = world.GetComponentFromEntity<GCDoor>(entity);
		std::optional<GCTransform> optGCTransform = world.GetComponentFromEntity<GCTransform>(entity);
		std::optional<GCRigidBody> optGCRigidBody = world.GetComponentFromEntity<GCRigidBody>(entity);

		GCDoor& gcDoor = optGCDoor.value();
		GCTransform& gcTransform = optGCTransform.value();
		GCRigidBody& gcRigidBody = optGCRigidBody.value();

		if (gcDoor.m_IsDoorOpen)
		{
			gcDoor.m_DoorOpenInterpolator += (1.f - gcDoor.m_DoorOpenInterpolator) * 0.5f * (deltaTime * 5.f);
		}
		else
		{
			gcDoor.m_DoorOpenInterpolator -= (gcDoor.m_DoorOpenInterpolator) * 0.5f * (deltaTime * 4.f);
		}

		gcDoor.m_DoorOpenInterpolator = glm::clamp(gcDoor.m_DoorOpenInterpolator, 0.f, 1.f);

		const float rotation = glm::mix(gcDoor.m_DoorCloseRotation, gcDoor.m_DoorOpenRotation, gcDoor.m_DoorOpenInterpolator);

		//std::cout << m_DoorOpenRotation << "\n";

		glm::vec3 rot = gcTransform.getEulerRotation();
		rot.x = rotation;
		gcTransform.SetEulerRotation(rot);

		// REWORK: I replaced btTransform by glm mat4 calculation. not tested yet!
		glm::mat4 tr = gcRigidBody.m_RigidBody.GetCenterOfMassTransform();
		glm::quat quat(rot);
		tr = tr * glm::mat4(quat);

		gcRigidBody.m_RigidBody.SetCenterOfMassTransform(tr);
	}

	void GSDoor::SetDoorOpen(GCDoor& gcDoor, bool aDoorOpen)
	{
		gcDoor.m_IsDoorOpen = aDoorOpen;
	}
}