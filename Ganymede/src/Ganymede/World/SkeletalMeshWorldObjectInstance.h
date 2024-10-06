#pragma once

#include "Ganymede/Core/Core.h"


#include "MeshWorldObjectInstance.h"


namespace Ganymede
{
	class SkeletalMeshWorldObject;
	class GANYMEDE_API SkeletalMeshWorldObjectInstance : public MeshWorldObjectInstance
	{
	public:
		using MeshWorldObjectInstance::MeshWorldObjectInstance;

		const SkeletalMeshWorldObject* GetSkeletalMeshWorldObject() const;

		const std::vector<glm::mat4>& GetAnimationBoneData() const { return m_AnimationBoneData; }

	protected:
		std::vector<glm::mat4> m_AnimationBoneData;
	};
}