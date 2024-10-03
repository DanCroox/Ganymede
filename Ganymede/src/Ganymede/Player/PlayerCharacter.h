#pragma once

#include "Ganymede/World/WorldObjectInstance.h"
#include "Ganymede/World/MeshWorldObjectInstance.h"


class CreateMeshWorldObjectInstance;
class btKinematicCharacterController;
class PointlightWorldObjectInstance;
class GLFWwindow;
class PhysicsWorld;
class FPSCamera;

class PlayerCharacter
{
public:
	PlayerCharacter() = delete;
	PlayerCharacter(World& world, PhysicsWorld& physicsWorld, FPSCamera& camera);
	~PlayerCharacter();

	void Tick(float deltaTime);

	glm::vec3 GetPosition() const { return m_Position; }

	CreateMeshWorldObjectInstance* creature = nullptr;
private:

	FPSCamera* m_Camera;
	World* m_World;
	PhysicsWorld* m_PhysicsWorld;
	glm::vec3 m_Position;
	// TODO : replace by proper input system
	bool m_MouseDown = false;
	bool m_MouseDown2 = false;
	bool m_FireButtonDown = false;
	glm::vec3 start;
	glm::vec3 end;

	bool m_JumpBTNDown = false;
	bool m_BtnOcclusionOnOffDown = false;
	float m_TargetMoveSpeed = 0;
	glm::vec3 m_TargetWalkDirection = glm::vec3(0);

	float m_WalkSpeed = .025;
	float m_RunSpeed = .5;

	KinematicCharacterController m_character;

	WorldObjectInstance* myOBJ2 = nullptr;

	PointlightWorldObjectInstance* light;

	GLFWwindow* m_GLFWWindow;

	float m_HeadBobSineModulatedTime = 0;
};