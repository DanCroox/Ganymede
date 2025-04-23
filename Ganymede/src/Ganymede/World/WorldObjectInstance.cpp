#include "WorldObjectInstance.h"

#include "glm/gtx/matrix_decompose.hpp"
#include "Ganymede/Common/Helpers.h"
#include "Ganymede/Physics/PhysicsWorld.h"

namespace Ganymede
{
	std::uint32_t WorldObjectInstance::s_NextInstanceID = 0;

	WorldObjectInstance::WorldObjectInstance(const WorldObject* worldObject) :
		m_WorldObject(worldObject),
		m_PhysicsWorld(nullptr)
	{
		m_InstanceID = s_NextInstanceID++;

		glm::mat4 trans = worldObject == nullptr ? glm::mat4(1.0f) : m_WorldObject->GetTransform();

		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(trans, scale, rotation, translation, skew, perspective);

		m_Position = translation;
		m_Rotation = rotation;
		m_Scale = scale;

		m_EulerAngles = glm::eulerAngles(rotation) * 3.14159f / 180.f;
	};
	
	void WorldObjectInstance::SetPhysicsWorld(PhysicsWorld& physicsWorld)
	{
		m_PhysicsWorld = &physicsWorld;
	}

	void WorldObjectInstance::SetPosition(glm::vec3 position)
	{
		m_Position = position;
	}

	const WorldObject* WorldObjectInstance::GetWorldObject() const
	{
		return m_WorldObject;
	}

	void WorldObjectInstance::SetPosition(float x, float y, float z)
	{
		m_Position.x = x;
		m_Position.y = y;
		m_Position.z = z;
	}

	void WorldObjectInstance::SetEulerAngle(glm::vec3 eulerAngles)
	{
		m_EulerAngles = eulerAngles;

		m_Rotation = glm::quat(glm::vec3(m_EulerAngles.x, m_EulerAngles.y, m_EulerAngles.z));
	}

	void WorldObjectInstance::SetEulerAngle(float x, float y, float z)
	{
		m_EulerAngles.x = x;
		m_EulerAngles.y = y;
		m_EulerAngles.z = z;

		m_Rotation = glm::quat(glm::vec3(x, y, z));
	}

	void WorldObjectInstance::SetQuaternion(glm::quat quaternion)
	{
		m_Rotation = quaternion;
		m_EulerAngles = glm::eulerAngles(m_Rotation) * 3.14159f / 180.f;
	}

	void WorldObjectInstance::SetScale(glm::vec3 scale)
	{
		m_Scale = scale;
	}

	void WorldObjectInstance::SetScale(float scale)
	{
		m_Scale.x = scale;
		m_Scale.y = scale;
		m_Scale.z = scale;
	}

	void WorldObjectInstance::SetScale(float x, float y, float z)
	{
		m_Scale.x = x;
		m_Scale.y = y;
		m_Scale.z = z;
	}

	void WorldObjectInstance::InternalTick(float deltaTime)
	{
		if (!m_CreateExecuted)
		{
			m_CreateExecuted = true;
			OnCreate();
		}

		Tick(deltaTime);
	}

	glm::mat4 WorldObjectInstance::GetTransform() const
	{
		if (m_RigidBody.IsValid())
		{
			const glm::mat4 rigidTransform = m_RigidBody.GetWorldTransform();
			const glm::vec3 scale = m_RigidBody.GetCollisionShapeLocalScaling();
			return glm::scale(rigidTransform, scale);
		}

		glm::mat4 globalTransform = GetLocalTransform();

		if (m_Parent != nullptr)
		{
			globalTransform = m_Parent->GetTransform() * globalTransform;
		}

		return globalTransform;
	}

	glm::mat4 WorldObjectInstance::GetLocalTransform() const
	{
		// TODO: reduntant code
		if (m_RigidBody.IsValid())
		{
			const glm::mat4 rigidTransform = m_RigidBody.GetWorldTransform();
			const glm::vec3 scale = m_RigidBody.GetCollisionShapeLocalScaling();
			return glm::scale(rigidTransform, scale);
		}

		/*
		glm::mat4 localTransform = glm::mat4(1.0f);
		localTransform = glm::translate(localTransform, m_Position);
		localTransform = localTransform * glm::mat4(m_Rotation);
		localTransform = glm::scale(localTransform, m_Scale);
		*/

		glm::mat4 translation = glm::translate(glm::mat4(1), m_Position);
		glm::mat4 rotation = glm::mat4(m_Rotation);
		glm::mat4 scale = glm::scale(glm::mat4(1), m_Scale);

		return translation * rotation * scale;
	}

	void WorldObjectInstance::RemoveRigidBody()
	{
		if (m_RigidBody.IsValid())
		{
			// REWORK: Do proper ppointer handling!!
			m_PhysicsWorld->RemoveRigidBody(m_RigidBody);
			m_RigidBody.Delete();
		}
	}

	void WorldObjectInstance::EnableRigidBody(bool enable)
	{
		if (m_RigidBody.IsValid())
		{
			// TODO: Add righid body temporary disable
		}
	}

	WorldObjectInstance::Mobility WorldObjectInstance::GetMobility() const
	{
		// Todo: Static mobility gets propergated to all children. Currently we do full dynamic lookup by iterating thru the parents up the chain.
		// Could be improved by updating all childrens mobility if an objects mobility changes. This way we dont need to cycle thru parents always.
		// However we do it this way for now for simplicity reasons
		if (m_Mobility == Mobility::Static)
		{
			// If static, we dont need to check any parents for mobility
			return m_Mobility;
		}

		// This objects mobility is dynamic. Search for potential static parent
		WorldObjectInstance* currentParent = m_Parent;
		while (currentParent != nullptr)
		{
			if (currentParent->m_Mobility == Mobility::Static)
			{
				// Static parent found. This object will be Static as well
				return Mobility::Static;
			}

			currentParent = currentParent->GetParent();
		}

		// Object is dynamic and no static parent found
		return m_Mobility;
	}
}