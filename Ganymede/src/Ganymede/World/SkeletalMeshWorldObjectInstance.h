#pragma once
#include "MeshWorldObjectInstance.h"
#include "SkeletalMeshWorldObject.h"

class SkeletalMeshWorldObjectInstance : public MeshWorldObjectInstance
{
public:
	using MeshWorldObjectInstance::MeshWorldObjectInstance;

	const SkeletalMeshWorldObject* GetSkeletalMeshWorldObject() const
	{
		return static_cast<const SkeletalMeshWorldObject*>(m_MeshWorldObject);
	}

	const std::vector<glm::mat4>& GetAnimationBoneData() const { return m_AnimationBoneData; }

protected:
	std::vector<glm::mat4> m_AnimationBoneData;
};