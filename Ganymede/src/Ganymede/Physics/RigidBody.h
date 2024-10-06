#pragma once
#include "Ganymede/Core/Core.h"

#include "glm/glm.hpp"


class btRigidBody;

namespace Ganymede
{
	class PhysicsWorld;
	
	class GANYMEDE_API RigidBody
	{
	public:
		RigidBody() : m_RigidBody(nullptr) {};
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
		void SetRestitution(float restitution); // bouncyness ... less is less bouncy
		void ApplyImpulse(glm::vec3 impulse, glm::vec3 position);
		void SetFriction(float friction);
		void SetSleepingThresholds(float linear, float angular);

	private:
		friend class PhysicsWorld;
		btRigidBody* m_RigidBody;
	};
}