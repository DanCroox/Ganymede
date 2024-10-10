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
#include "Ganymede/Input/KeyCodes.h"

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
		if (event.GetKeyCode() == KeyCode::Key_W)
		{
			m_ForwardMotion = 1.0f;
		}
		else if (event.GetKeyCode() == KeyCode::Key_S)
		{
			m_ForwardMotion = -1.0f;
		}
		else if (event.GetKeyCode() == KeyCode::Key_D)
		{
			m_SidewayMotion = 1.0f;
		}
		else if (event.GetKeyCode() == KeyCode::Key_A)
		{
			m_SidewayMotion = -1.0f;
		}
	}

	void PlayerCharacter::OnKeyReleaseEvent(KeyReleaseEvent& event)
	{
		if (event.GetKeyCode() == KeyCode::Key_W)
		{
			m_ForwardMotion = 0.0f;
		}
		else if (event.GetKeyCode() == KeyCode::Key_S)
		{
			m_ForwardMotion = 0.0f;
		}
		else if (event.GetKeyCode() == KeyCode::Key_D)
		{
			m_SidewayMotion = 0.0f;
		}
		else if (event.GetKeyCode() == KeyCode::Key_A)
		{
			m_SidewayMotion = 0.0f;
		}
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

		//translation = glm::vec3(physicsMat[3]);

		cam.Update(deltaTime);

		glm::vec3 forwardDirection = cam.GetFrontVector();
		glm::vec3 rightDirection = cam.GetRightVector();

		glm::vec3 walkDirection = (forwardDirection * m_ForwardMotion) + (rightDirection * m_SidewayMotion);
		walkDirection *= m_WalkSpeed;

		m_character.SetWalkDirection(walkDirection);
		cam.SetPosition(translation + glm::vec3(0, 1.2, 0));

		m_Position = translation;
		return;
		if (glm::length(forwardDirection) > 0.00001f)
		{
			forwardDirection = glm::normalize(forwardDirection);
		}

		const glm::vec3 from = cam.GetPosition();
		const glm::vec3 to = from + (cam.GetFrontVector() * 1000.f);

		// Todo: Add air resistance
		if (m_character.CanJump())
			m_TargetWalkDirection += ((forwardDirection - m_TargetWalkDirection) * deltaTime * 10.f);

		m_TargetMoveSpeed += ((m_MoveSpeed - m_TargetMoveSpeed) * deltaTime);

		m_character.SetWalkDirection(m_TargetWalkDirection);
		
		// Head bobbing
		m_HeadBobSineModulatedTime += deltaTime * glm::length(m_TargetWalkDirection) * 350.0f;
		float leftHeadBobSineTime = m_HeadBobSineModulatedTime * .5;

		//float sinusWave2 = glm::sin(glfwGetTime() * m_TargetMoveSpeed * 150);
		glm::vec3 translationHeadBob = translation + glm::vec3(0, glm::sin(m_HeadBobSineModulatedTime) * .03f, 0);

		//glm::vec3 camRightVecor = glm::cross(cam.GetFrontVector(), cam.GetUpVector());
		translationHeadBob += rightDirection * glm::sin(leftHeadBobSineTime) * .03f;

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