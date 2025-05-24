#pragma once

#include "entt/entt.hpp"
#include "Ganymede/World/WorldObjectInstance.h"

namespace Ganymede
{
	class WorldObject;
	class MeshWorldObject;
	class PointlightWorldObject;
	class World;
}

class EntityHelpers
{
public:
	static entt::entity CreateWorldEntity(
		Ganymede::World& world,
		const Ganymede::WorldObject& wo,
		Ganymede::WorldObjectInstance::Mobility mobility);

	static entt::entity CreateMeshEntity(
		Ganymede::World& world,
		const Ganymede::MeshWorldObject& mwo,
		Ganymede::WorldObjectInstance::Mobility mobility);

	static entt::entity CreateMeshEntityWithPhysics(
		Ganymede::World& world,
		const Ganymede::MeshWorldObject& mwo,
		Ganymede::PhysicsWorld& physicsWorld,
		float weight,
		Ganymede::WorldObjectInstance::Mobility mobility);

	static entt::entity CreatePointlightEntity(
		Ganymede::World& world,
		const Ganymede::PointlightWorldObject& plwo,
		Ganymede::WorldObjectInstance::Mobility mobility);
};