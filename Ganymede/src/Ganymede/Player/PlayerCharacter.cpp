#include "PlayerCharacter.h"

#include "FPSCamera.h"
#include "Ganymede/Physics/PhysicsWorld.h"
#include "Ganymede/Common/Helpers.h"
#include "Ganymede/World/World.h"
#include "Ganymede/AI/NavMesh.h"
#include "Ganymede/World/MeshWorldObjectInstance.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "Ganymede/World/PointlightWorldObjectInstance.h"

#include <iostream>
#include "Ganymede/World/CreatureMeshWorldObjectInstance.h"
#include "Ganymede/World/MeshWorldObjectInstanceDoor.h"
#include "Ganymede/Graphics/WorldPartition.h"

#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/euler_angles.hpp"
#include <GLFW/glfw3.h>

/*
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		
		std::cout << "hallo";
	}
}
*/

PlayerCharacter::PlayerCharacter(World& world, PhysicsWorld& physicsWorld, FPSCamera& camera)
{
	//TODO DELETE ALL PROERLY
	m_Camera = &camera;
	m_World = &world;
	m_PhysicsWorld = &physicsWorld;
	m_character = m_PhysicsWorld->CreateCapsule(.1, .5, 75, glm::vec3(0, 0, 0));
	
	//myBodyEmpty->AddChild(myLamp);
	//myLamp->SetPosition(-.6, -.4, 1.f);
//	light = Globals::world->CreateEmptyWorldObjectInstance<PointlightWorldObjectInstance>();
//	light->SetBrightness(0);
//	light->SetColor(.2,.5,.6);
//	light->SetVisible(false);
}

PlayerCharacter::~PlayerCharacter()
{
	// REWORK: Do proper pointer handling!!!!
	m_character.Delete();
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
	
	glm::vec3 forward = cam.GetFrontVector();
	forward.y = 0;
	forward = glm::normalize(forward);
	glm::vec3 up = cam.GetUpVector();
	glm::vec3 left = glm::normalize(glm::cross(forward, up));

	//btVector3 btForward(forward.x, 0, forward.z);
	//btVector3 btLeft(left.x, 0, left.z);

	glm::vec3 walkDirection(0, 0, 0);

	float moveSpeed = 0;

	if (glfwGetKey(m_GLFWWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		moveSpeed = m_WalkSpeed;
		walkDirection += forward;
	}
	if (glfwGetKey(m_GLFWWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		moveSpeed = m_WalkSpeed;
		walkDirection += (forward * -1.0f);
	}
	if (glfwGetKey(m_GLFWWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		moveSpeed = m_WalkSpeed;
		walkDirection += (left * -1.0f);
	}
	if (glfwGetKey(m_GLFWWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		moveSpeed = m_WalkSpeed;
		walkDirection += left;
	}

	if (glfwGetKey(m_GLFWWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && moveSpeed != 0)
		moveSpeed = m_RunSpeed;

	if (glm::length(walkDirection) > 0.00001f)
	{
		walkDirection = glm::normalize(walkDirection);
	}

	if (glfwGetKey(m_GLFWWindow, GLFW_KEY_SPACE) == GLFW_PRESS && !m_JumpBTNDown)
	{
		m_JumpBTNDown = true;
		if(m_character.CanJump())
			m_character.Jump({ 0 ,7.5 ,0 });
	}
	else if (glfwGetKey(m_GLFWWindow, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		m_JumpBTNDown = false;
	}

	if (glfwGetMouseButton(m_GLFWWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !m_FireButtonDown)
	{
		m_FireButtonDown = true;
		if (WorldObjectInstance* woi = m_World->CreateWorldObjectInstance("Projectile"))
		{
			const glm::vec3 camFrontVector = cam.GetFrontVector();
			const glm::vec3 camPosition = cam.GetPosition();

			woi->SetPosition(camPosition + (camFrontVector * 1.f));
			woi->SetPhysicsWorld(*m_PhysicsWorld); //REWORK: FIND BETTER WAY TO DEAL WITH PHYISCS IN WORLDOBJECTS (lights are also world objects but feature physics which is weird)
			woi->MakeRigidBody(30);
			woi->GetRigidBody().SetRestitution(.001f); // bouncyness ... less is less bouncy
			woi->SetMobility(WorldObjectInstance::Mobility::Dynamic);
			
			const glm::vec3 instancePosition = woi->GetPosition();

			const glm::vec3 impulse = camPosition + (camFrontVector * 200.f);

			// TODO: Get rid of btVector3 and use common types across all systems
			woi->GetRigidBody().ApplyImpulse(impulse, instancePosition);
		}
	}
	else if (m_FireButtonDown && glfwGetMouseButton(m_GLFWWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		m_FireButtonDown = false;
	}

	if (glfwGetKey(m_GLFWWindow, GLFW_KEY_V) == GLFW_PRESS && !m_MouseDown)
	{
		m_MouseDown = true;

		const glm::vec3 from = cam.GetPosition();
		const glm::vec3 to = from + cam.GetFrontVector() * 100.f;

		const RayResult hitResult = m_PhysicsWorld->RayCast(from, to);
		if (hitResult.m_HasHit)
		{
			float a = hitResult.m_HitFraction;
			glm::vec3 asdf = from * (1.f - a) + to * a;


			if (myOBJ2 == nullptr)
			{
				myOBJ2 = m_World->CreateWorldObjectInstance("Projectile");
			}

			start = hitResult.m_HitWorldLocation;
			std::cout << start.x << ", "<< start.y << ", " << start.z << "\n";
		}
	}
	else if (glfwGetKey(m_GLFWWindow, GLFW_KEY_V) == GLFW_RELEASE)
	{
		m_MouseDown = false;
	}

	if (glfwGetKey(m_GLFWWindow, GLFW_KEY_B) == GLFW_PRESS && !m_MouseDown2)
	{
		m_MouseDown2 = true;

		const glm::vec3 from = cam.GetPosition();
		const glm::vec3 to = from + (cam.GetFrontVector() * 1000.f);

		/**/
		const RayResult hitResult = m_PhysicsWorld->RayCast(from, to);
		if (hitResult.m_HasHit)
		{
			float a = hitResult.m_HitFraction;
			glm::vec3 asdf = from * (1.f - a) + to * a;

			MeshWorldObjectInstance* mwoi = reinterpret_cast<MeshWorldObjectInstance*>(hitResult.m_CollisionObject);

			if (MeshWorldObjectInstanceDoor* doorMwoi = dynamic_cast<MeshWorldObjectInstanceDoor*>(mwoi))
			{
				doorMwoi->SetDoorOpen(!doorMwoi->IsDoorOpen());
			}
		}

		/*
		const glm::vec3 from = cam.GetPosition();
		const glm::vec3 to = from + cam.GetFrontVector() * 100.f;

		const btCollisionWorld::ClosestRayResultCallback hitResult = Globals::physicsWorld->RayCast(from, to);
		if (hitResult.hasHit())
		{
			float a = hitResult.m_collisionObject->getHitFraction();

			if (myOBJ2 == nullptr)
			{
				myOBJ2 = Globals::world->CreateWorldObjectInstance("Projectile");
			}

			end = { hitResult.m_hitPointWorld.getX(), hitResult.m_hitPointWorld.getY(), hitResult.m_hitPointWorld.getZ() };

			int result = Globals::navMesh->FindPath(&start.x, &end.x, 0, 0);

			std::vector<glm::vec3> waypoints;

			for (int i = 0; i < result; ++i)
			{
				glm::vec3 pos = { Globals::navMesh->m_PathStore->PosX[i],
				Globals::navMesh->m_PathStore->PosY[i],
				Globals::navMesh->m_PathStore->PosZ[i] };

				waypoints.push_back(pos);

				auto point = Globals::world->CreateWorldObjectInstance("Projectile");
				point->SetPosition(pos.x, pos.y, pos.z);
			}

			if (creature != nullptr)
			{
				creature->TryGoto(end);
			}

			std::cout << result << "\n";
		
		 } */
	}
	else if (glfwGetKey(m_GLFWWindow, GLFW_KEY_B) == GLFW_RELEASE)
	{
		m_MouseDown2 = false;
	}

	if (glfwGetKey(m_GLFWWindow, GLFW_KEY_Z) == GLFW_PRESS && !m_BtnOcclusionOnOffDown)
	{
		m_BtnOcclusionOnOffDown = true;
		//Globals::renderer->m_DebugDisableOcclusionCulling = !Globals::renderer->m_DebugDisableOcclusionCulling;
	}
	else if (glfwGetKey(m_GLFWWindow, GLFW_KEY_Z) == GLFW_RELEASE && m_BtnOcclusionOnOffDown)
	{

		m_BtnOcclusionOnOffDown = false;
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

	walkDirection *= moveSpeed;

	// Todo: Add air resistance
	if(m_character.CanJump())
		m_TargetWalkDirection += ((walkDirection - m_TargetWalkDirection) * deltaTime * 10.f);

	m_TargetMoveSpeed += ((moveSpeed - m_TargetMoveSpeed) * deltaTime);

	m_character.SetWalkDirection(m_TargetWalkDirection);
	m_Position = translation;
	
	
	// Head bobbing
	m_HeadBobSineModulatedTime += deltaTime * glm::length(m_TargetWalkDirection) * 350.0f;
	float leftHeadBobSineTime = m_HeadBobSineModulatedTime *.5;

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
	//light->SetPosition(translation.x, translation.y + .5, translation.z);
}
