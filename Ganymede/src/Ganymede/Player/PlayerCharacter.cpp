#include "PlayerCharacter.h"
#include "Ganymede/Core/Application.h"
#include "FPSCamera.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/World/World.h"
#include "Ganymede/AI/NavMesh.h"
#include "Ganymede/World/MeshWorldObjectInstance.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"

#include <iostream>
#include "Ganymede/World/CreatureMeshWorldObjectInstance.h"
#include "Ganymede/World/MeshWorldObjectInstanceDoor.h"
#include "Ganymede/Events/Event.h"
#include "Ganymede/Runtime/WindowEvents.h"

#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/euler_angles.hpp"
#include <GLFW/glfw3.h>

namespace Ganymede
{
	PlayerCharacter::PlayerCharacter(World& world, PhysicsWorld& physicsWorld, FPSCamera& camera)
	{
		//TODO DELETE ALL PROERLY
		m_Camera = &camera;
		m_World = &world;
		m_PhysicsWorld = &physicsWorld;
		m_character = m_PhysicsWorld->CreateCapsule(.1, .5, 75, glm::vec3(0, 0, 0));

		m_KeyPressEventCBHandle = std::make_unique<EventCallbackHandle>();
		m_KeyReleaseEventCBHandle = std::make_unique<EventCallbackHandle>();

		EventSystem& eventSystem = Application::Get().GetEventSystem();
		eventSystem.SubscribeEvent<KeyPressEvent>(*m_KeyPressEventCBHandle, EVENT_BIND_TO_MEMBER(PlayerCharacter::OnKeyPressEvent));
		eventSystem.SubscribeEvent<KeyReleaseEvent>(*m_KeyReleaseEventCBHandle, EVENT_BIND_TO_MEMBER(PlayerCharacter::OnKeyReleaseEvent));	
	}

	void PlayerCharacter::OnKeyPressEvent(KeyPressEvent& event)
	{
		m_MoveSpeed = m_WalkSpeed;
	}

	void PlayerCharacter::OnKeyReleaseEvent(KeyReleaseEvent& event)
	{
		m_MoveSpeed = 0.0f;
	}

	PlayerCharacter::~PlayerCharacter()
	{
		// REWORK: Do proper pointer handling!!!!
		m_character.Delete();
		
		EventSystem& eventSystem = Application::Get().GetEventSystem();
		eventSystem.UnsubscribeEvent<KeyPressEvent>(*m_KeyPressEventCBHandle);
		eventSystem.UnsubscribeEvent<KeyReleaseEvent>(*m_KeyReleaseEventCBHandle);
	}

	void PlayerCharacter::Tick(float deltaTime)
	{
		FPSCamera& cam = *m_Camera;

		glm::mat4 physicsMat = m_character.GetGhostObjectWorldTransform();
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(physicsMat, scale, rotation, translation, skew, perspective);

		cam.Update(deltaTime);

		glm::vec3 walkDirection = cam.GetFrontVector();
		walkDirection.y = 0;
		walkDirection = glm::normalize(walkDirection);
		glm::vec3 up = cam.GetUpVector();
		glm::vec3 left = glm::normalize(glm::cross(walkDirection, up));

		if (glm::length(walkDirection) > 0.00001f)
		{
			walkDirection = glm::normalize(walkDirection);
		}

		const glm::vec3 from = cam.GetPosition();
		const glm::vec3 to = from + (cam.GetFrontVector() * 1000.f);

		/**/
		const RayResult hitResult = m_PhysicsWorld->RayCast(from, to);
		if (hitResult.m_HasHit)
		{
			MeshWorldObjectInstance* mwoi = reinterpret_cast<MeshWorldObjectInstance*>(hitResult.m_CollisionObject);
			//TODO Globals::worldPartitionManager->m_DebugNode = Globals::worldPartitionManager->FindWorldPartitionNodesByMeshWorldObjectInstance(mwoi);	
		}

		walkDirection *= m_MoveSpeed;

		// Todo: Add air resistance
		if (m_character.CanJump())
			m_TargetWalkDirection += ((walkDirection - m_TargetWalkDirection) * deltaTime * 10.f);

		m_TargetMoveSpeed += ((m_MoveSpeed - m_TargetMoveSpeed) * deltaTime);

		m_character.SetWalkDirection(m_TargetWalkDirection);
		m_Position = translation;


		// Head bobbing
		m_HeadBobSineModulatedTime += deltaTime * glm::length(m_TargetWalkDirection) * 350.0f;
		float leftHeadBobSineTime = m_HeadBobSineModulatedTime * .5;

		//float sinusWave2 = glm::sin(glfwGetTime() * m_TargetMoveSpeed * 150);
		glm::vec3 translationHeadBob = translation + glm::vec3(0, glm::sin(m_HeadBobSineModulatedTime) * .03f, 0);

		//glm::vec3 camRightVecor = glm::cross(cam.GetFrontVector(), cam.GetUpVector());
		translationHeadBob += left * glm::sin(leftHeadBobSineTime) * .03f;

		cam.SetPosition(translationHeadBob + glm::vec3(0, 1.2, 0));
		//cam.SetRollInDegree((glm::sin(leftHeadBobSineTime) * 2.f - 1.f) * .15f);
		cam.SetRollInDegree(glm::sin(leftHeadBobSineTime) * .5f);

		glm::mat4 rot = glm::lookAtLH(cam.GetPosition(), cam.GetPosition() + cam.GetFrontVector(), cam.GetUpVector());
		glm::vec3 eulerAngles;
		glm::quat orientation = glm::conjugate(glm::toQuat(rot));
		eulerAngles = glm::eulerAngles(orientation);


		//cam.SetPosition(glm::vec3(0, 2, 0));

	}
}