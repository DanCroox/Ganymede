#pragma once
#include "Ganymede/Core/Core.h"

#include "entt/entt.hpp"
#include "glm/glm.hpp"

namespace Ganymede
{
	class GCTransform;
	class GCSkeletal;
	class GCCreature;
	class World;

	class GANYMEDE_API GSCreature
	{
	public:
		static void Initialize(World&, entt::entity, float deltaTime);
		static void Tick(World&, entt::entity, float deltaTime);

		static bool TryGoto(const GCTransform& gcTransform, GCCreature& gcCreature, glm::vec3 destination);
		static void UpdateMotion(GCTransform& gcTransform, GCCreature& gcCreature, float deltaTime);
		static void UpdateAnimation(GCCreature& gcCreature, GCSkeletal& gcSkeletal, float deltaTime);
	};
}