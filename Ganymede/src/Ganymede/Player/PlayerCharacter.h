#pragma once

#include "Ganymede/Core/Core.h"

#include <memory>
#include "Ganymede/World/WorldObjectInstance.h"
#include "Ganymede/Physics/PhysicsWorld.h"

	class GLFWwindow;

	namespace Ganymede
	{
		class EventCallbackHandle;
		class MouseButtonPressEvent;
		class KeyPressEvent;
		class KeyReleaseEvent;
		class PhysicsWorld;
		class FPSCamera;
		class CreatureMeshWorldObjectInstance;
		class PointlightWorldObjectInstance;

		class GANYMEDE_API PlayerCharacter
		{
		public:
			PlayerCharacter() = delete;
			PlayerCharacter(World& world, PhysicsWorld& physicsWorld, FPSCamera& camera);
			~PlayerCharacter();

			void Tick(float deltaTime);

			glm::vec3 GetPosition() const { return m_Position; }

			CreatureMeshWorldObjectInstance* creature = nullptr;

		private:
			std::unique_ptr<EventCallbackHandle> m_MouseButtonPressEventCBHandle;
			void OnMouseButtonPressEvent(MouseButtonPressEvent& event);

			std::unique_ptr<EventCallbackHandle> m_KeyPressEventCBHandle;
			void OnKeyPressEvent(KeyPressEvent& event);

			std::unique_ptr<EventCallbackHandle> m_KeyReleaseEventCBHandle;
			void OnKeyReleaseEvent(KeyReleaseEvent& event);

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

			float m_ForwardMotion = 0.0f;
			float m_SidewayMotion = 0.0f;
			float m_WalkSpeed = .07;

			float m_MoveSpeed = 0;

			KinematicCharacterController m_character;
			WorldObjectInstance* myOBJ2 = nullptr;

			float m_HeadBobSineModulatedTime = 0;
		};
	}