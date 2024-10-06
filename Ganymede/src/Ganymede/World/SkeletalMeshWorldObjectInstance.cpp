#include "SkeletalMeshWorldObjectInstance.h"
#include "SkeletalMeshWorldObject.h"

namespace Ganymede
{
	const SkeletalMeshWorldObject* SkeletalMeshWorldObjectInstance::GetSkeletalMeshWorldObject() const
	{
		return static_cast<const SkeletalMeshWorldObject*>(m_MeshWorldObject);
	}
}