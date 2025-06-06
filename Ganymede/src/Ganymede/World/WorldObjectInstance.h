#pragma once

#include "Ganymede/Core/Core.h"

#include <string>
#include <vector>
#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "glm/gtx/matrix_decompose.hpp"
#include "Ganymede/Physics/RigidBody.h"


namespace Ganymede
{
	class PhysicsWorld;
	class WorldObject;

	class GANYMEDE_API WorldObjectInstance
	{
	public:
		GM_GENERATE_BASE_CLASSTYPEINFO(WorldObjectInstance);

		enum class Mobility
		{
			_Invalid,
			Static,
			Dynamic
		};

		WorldObjectInstance() = default;

		WorldObjectInstance(const WorldObject* worldObject);

		void SetPhysicsWorld(PhysicsWorld& physicsWorld);

		~WorldObjectInstance()
		{
			OnDestroy();
			RemoveRigidBody();
		}

		const WorldObject* GetWorldObject() const;

		void SetPosition(glm::vec3 position);
		void SetPosition(float x, float y, float z);
		void SetEulerAngle(glm::vec3 eulerAngles);
		void SetEulerAngle(float x, float y, float z);
		void SetQuaternion(glm::quat quaternion);
		void SetScale(glm::vec3 scale);
		void SetScale(float scale);
		void SetScale(float x, float y, float z);

		// TODO: Caching! Returns the model matrix. Builds matrix each call! Be ware!
		glm::mat4 GetTransform() const;
		glm::mat4 GetLocalTransform() const;

		glm::vec3 GetPosition() const
		{
			return glm::vec3(GetTransform()[3]);
		}

		glm::vec3 GetEulerAngle() const { return m_EulerAngles; }
		const glm::quat& GetQuaternion() const { return m_Rotation; }
		glm::vec3 GetScale() const { return m_Scale; }

		void InternalTick(float deltaTime);

		virtual void OnCreate() {};
		virtual void Tick(float deltaTime) {};
		virtual void OnDestroy() {};

		void SetVisible(bool visible) { m_IsVisible = visible; }
		bool IsVisible() const { return m_IsVisible; }

		virtual void MakeRigidBody(float mass) {};
		void RemoveRigidBody();
		void EnableRigidBody(bool enable);

		void SetMobility(Mobility mobility)
		{
			m_Mobility = mobility;
		}

		// TODO: Needs to be integrated properly! Actually only used for decide if any instance will be occlusion culled or not!
		Mobility GetMobility() const;

		WorldObjectInstance* GetParent() const { return m_Parent; }

		const std::vector<WorldObjectInstance*>& GetChildren() const { return m_Children; }

		void AddChild(WorldObjectInstance* child)
		{
			for (unsigned int i = 0; i < m_Children.size(); ++i)
			{
				if (m_Children[i] == child)
				{
					// child already in list
					return;
				}
			}

			child->m_Parent = this;
			m_Children.push_back(child);
		};

		void RemoveChild(WorldObjectInstance* child)
		{
			for (unsigned int i = 0; i < m_Children.size(); ++i)
			{
				if (m_Children[i] == child)
				{
					m_Children.erase(m_Children.begin() + i);
					child->m_Parent = nullptr;
					return;
				}
			}
		};

		RigidBody GetRigidBody() { return m_RigidBody; }
		const RigidBody GetRigidBody() const { return m_RigidBody; }

		std::uint32_t GetInstanceID() const { return m_InstanceID; }

	protected:
		bool m_CreateExecuted = false;
		PhysicsWorld* m_PhysicsWorld;
		RigidBody m_RigidBody;
		Mobility m_Mobility = Mobility::Static;

	private:
		static std::uint32_t s_NextInstanceID;
		std::uint32_t m_InstanceID;

		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec3 m_EulerAngles = glm::vec3(0.0f);
		glm::quat m_Rotation = glm::quat();
		glm::vec3 m_Scale = glm::vec3(1.0f);

		const WorldObject* m_WorldObject;

		WorldObjectInstance* m_Parent = nullptr;
		std::vector<WorldObjectInstance*> m_Children;

		bool m_IsVisible = false;
	};
}