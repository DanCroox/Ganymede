#include "Ganymede/World/MeshWorldObjectInstance.h"
#include "Ganymede/Physics/PhysicsWorld.h"
#include "Ganymede/World/MeshWorldObject.h"

namespace Ganymede
{
	MeshWorldObjectInstance::MeshWorldObjectInstance(const MeshWorldObject* meshWorldObject) :
		WorldObjectInstance(meshWorldObject),
		m_MeshWorldObject(meshWorldObject),
		m_CastShadows(meshWorldObject->GetCastShadows())
	{
		for (const MeshWorldObject::Mesh* mesh : meshWorldObject->m_Meshes)
		{
			AddMaterial(mesh->m_Material);
		}
	};

	const MeshWorldObject* MeshWorldObjectInstance::GetMeshWorldObject() const
	{
		return m_MeshWorldObject;
	}

	void MeshWorldObjectInstance::MakeRigidBody(float mass)
	{
		GM_CORE_ASSERT(m_PhysicsWorld != nullptr, "Physicsworld must not be nullptr.");
		if (!m_RigidBody.IsValid())
		{
			//REWORK: Implement proper physics and rigidbody property-ing. Maybe move physics code out of Worldobjectinstance and into meshworldobjectinstance
			//Maybe remove from instance at all
			m_RigidBody = m_PhysicsWorld->AddRigidBodyFromMeshWorldObject(*this, mass);
			m_RigidBody.SetDamping(.1f, .1f);
		}
	}

}