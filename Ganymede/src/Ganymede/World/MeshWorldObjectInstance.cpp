#include "Ganymede/World/MeshWorldObjectInstance.h"
#include "Ganymede/Physics/PhysicsWorld.h"

void MeshWorldObjectInstance::MakeRigidBody(float mass)
{
	ASSERT(m_PhysicsWorld != nullptr);
	if (!m_RigidBody.IsValid())
	{
		//REWORK: Implement proper physics and rigidbody property-ing. Maybe move physics code out of Worldobjectinstance and into meshworldobjectinstance
		//Maybe remove from instance at all
		m_RigidBody = m_PhysicsWorld->AddRigidBodyFromMeshWorldObject(*this, mass);
		m_RigidBody.SetDamping(.1f, .1f);
	}
}