#pragma once

#include "Ganymede/Core/Core.h"
#include "RigidBody.h"
#include "glm/glm.hpp"
#include "Ganymede/World/MeshWorldObjectInstance.h"
#include "Ganymede/World/MeshWorldObject.h"


class btKinematicCharacterController;
class btRigidBody;
class btCollisionObject;
class btDiscreteDynamicsWorld;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;

namespace Ganymede
{
	struct GANYMEDE_API RayResult
	{
		bool m_HasHit = false;
		float m_HitFraction = 0.0f;
		void* m_CollisionObject = nullptr;
		glm::vec3 m_HitWorldLocation = glm::vec3(0.0f);
		glm::vec3 m_HitNormalWorld = glm::vec3(0.0f);
	};



	class GANYMEDE_API KinematicCharacterController
	{
	public:
		KinematicCharacterController() = default;
		~KinematicCharacterController() = default;

		KinematicCharacterController(btKinematicCharacterController* btCharacterController);

		void Delete();
		glm::mat4 GetGhostObjectWorldTransform() const;
		bool CanJump() const;
		void Jump(glm::vec3 force);
		void SetWalkDirection(glm::vec3 direction);

	private:
		btKinematicCharacterController* m_btCharacterController;
	};

	class GANYMEDE_API PhysicsWorld
	{
	public:
		PhysicsWorld();
		~PhysicsWorld();

		void Step(float time);

		KinematicCharacterController CreateCapsule(float radius, float height, float mass, glm::vec3 startPosition);
		RigidBody AddRigidBodyFromMeshWorldObject(MeshWorldObjectInstance& mwoi, float mass);
		// btCollisionObject* AddStaticBodyFromMeshWorldObject(MeshWorldObjectInstance& mwoi, float mass); 
		void RemoveRigidBody(RigidBody body);

		RayResult RayCast(glm::vec3 fromWorld, glm::vec3 toWorld);

	private:
		btDiscreteDynamicsWorld* m_DynamicsWorld = nullptr;
		btDefaultCollisionConfiguration* m_CollisionConfiguration = nullptr;
		btCollisionDispatcher* m_Dispatcher = nullptr;
		btBroadphaseInterface* m_OverlappingPairCache = nullptr;
		btSequentialImpulseConstraintSolver* m_Solver = nullptr;
	};
}