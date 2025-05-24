#include "EntityHelpers.h"

#include "Ganymede/ECS/Components/GCDynamicMobility.h"
#include "Ganymede/ECS/Components/GCEntityID.h"
#include "Ganymede/ECS/Components/GCIgnoreForNavMesh.h"
#include "Ganymede/ECS/Components/GCMesh.h"
#include "Ganymede/ECS/Components/GCName.h"
#include "Ganymede/ECS/Components/GCPointlight.h"
#include "Ganymede/ECS/Components/GCRigidBody.h"
#include "Ganymede/ECS/Components/GCStaticMobility.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/Physics/PhysicsWorld.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/PointlightWorldObject.h"
#include "Ganymede/World/World.h"
#include "Ganymede/World/WorldObject.h"


entt::entity EntityHelpers::CreateWorldEntity(
	Ganymede::World& world,
	const Ganymede::WorldObject& wo,
	Ganymede::WorldObjectInstance::Mobility mobility)
{
	using namespace Ganymede;

	entt::entity entity = world.CreateEntity();
	world.AddComponent<GCEntityID>(entity);
	world.AddComponent<GCName>(entity).m_Name = wo.GetName();
	world.AddComponent<GCTransform>(entity).SetMatrix(wo.GetTransform());

	if (mobility == Ganymede::WorldObjectInstance::Mobility::Dynamic)
	{
		world.AddComponent<GCDynamicMobility>(entity);
	}
	else if (mobility == Ganymede::WorldObjectInstance::Mobility::Static)
	{
		world.AddComponent<GCStaticMobility>(entity);
	}
	else
	{
		GM_ASSERT(false, "Unknown mobility");
	}

	return entity;
}


entt::entity EntityHelpers::CreateMeshEntity(
	Ganymede::World& world,
	const Ganymede::MeshWorldObject& mwo,
	Ganymede::WorldObjectInstance::Mobility mobility)
{
	using namespace Ganymede;

	entt::entity entity = CreateWorldEntity(world, mwo, mobility);
	world.AddComponent<GCMesh>(entity, mwo.m_Meshes);

	if (mwo.GetExcludeFromNavigationMesh())
	{
		world.AddComponent<GCIgnoreForNavMesh>(entity);
	}

	return entity;
}

entt::entity EntityHelpers::CreateMeshEntityWithPhysics(
	Ganymede::World& world,
	const Ganymede::MeshWorldObject& mwo,
	Ganymede::PhysicsWorld& physicsWorld,
	float mass,
	Ganymede::WorldObjectInstance::Mobility mobility)
{
	using namespace Ganymede;

	entt::entity entity = CreateMeshEntity(world, mwo, mobility);
	std::optional<GCMesh> gcMesh = world.GetComponentFromEntity<GCMesh>(entity);
	std::optional<GCTransform> gcTransform = world.GetComponentFromEntity<GCTransform>(entity);

	GM_ASSERT(world.HasComponents<GCMesh>(entity), "Entity misses GCMesh component.");
	GM_ASSERT(world.HasComponents<GCTransform>(entity), "Entity misses GCTransform component.");

	GCRigidBody& rigidBody = world.AddComponent<GCRigidBody>(entity);
	rigidBody.m_RigidBody = physicsWorld.AddRigidBodyFromMeshWorldObject(gcMesh.value(), gcTransform.value(), mass);
	rigidBody.m_RigidBody.SetDamping(.1f, .1f);

	return entity;
}

entt::entity EntityHelpers::CreatePointlightEntity(
	Ganymede::World& world,
	const Ganymede::PointlightWorldObject& plwo,
	Ganymede::WorldObjectInstance::Mobility mobility)
{
	using namespace Ganymede;

	entt::entity entity = CreateWorldEntity(world, plwo, mobility);
	GCPointlight& gcPointlight = world.AddComponent<GCPointlight>(entity);
	gcPointlight.m_Brightness = plwo.GetBrightness();
	gcPointlight.m_Color = plwo.GetColor();

	return entity;
}