#pragma once
#include "Ganymede/Core/Core.h"

#include "entt/entt.hpp"
#include "glm/glm.hpp"

namespace Ganymede
{
	class GCDoor;
	class World;

	class GANYMEDE_API GSDoor
	{
	public:
		static void Initialize(World&, entt::entity, float deltaTime);
		static void Tick(World&, entt::entity, float deltaTime);

		static void SetDoorOpen(GCDoor& gcDoor, bool aDoorOpen);
	};
}
