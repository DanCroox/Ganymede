#pragma once

#include "glm/vec3.hpp"
#include "Ganymede/Common/Helpers.h"


class MeshWorldObjectInstance;
class btKinematicCharacterController;
class btRigidBody;
class btCollisionObject;
class btDiscreteDynamicsWorld;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;

struct RayResult
{
	bool m_HasHit = false;
	float m_HitFraction = 0.0f;
	void* m_CollisionObject = nullptr;
	glm::vec3 m_HitWorldLocation = glm::vec3(0.0f);
	glm::vec3 m_HitNormalWorld = glm::vec3(0.0f);
};

class RigidBody
{
public:
	RigidBody() = default;
	~RigidBody() = default;

	// REWORK: DO proper pointer handling!!!!!!
	RigidBody(btRigidBody* rigidBody);

	bool IsValid() const;
	void Delete();
	void SetDamping(float linearDamping, float angularDamping);
	glm::mat4 GetCenterOfMassTransform() const;
	void SetCenterOfMassTransform(glm::mat4 transform);
	glm::mat4 GetWorldTransform() const;
	glm::vec3 GetCollisionShapeLocalScaling() const;
	void SetRestitution(float restitution);
	void ApplyImpulse(glm::vec3 impulse, glm::vec3 position);

private:
	friend class PhysicsWorld;
	btRigidBody* m_RigidBody;
};

class KinematicCharacterController
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

class PhysicsWorld
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