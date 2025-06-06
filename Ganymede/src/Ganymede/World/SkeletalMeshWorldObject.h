#pragma once

#include "Ganymede/Core/Core.h"

#include "MeshWorldObject.h"

namespace Ganymede
{
	struct Bone
	{
		glm::mat4 m_OffsetTransform;
		unsigned int m_Index;
		std::string m_BoneName;
	};


	struct BoneWeight
	{
		unsigned int m_BoneIndex; //like in vertex buffer
		float m_Weight;
	};

	struct Animation
	{
		using BoneFrame = std::vector<glm::mat4>;
		
		float m_FPS;
		std::string m_Name;
		// Has all frames related to a bone. m_BoneFrames[boneIndex] -> list of all animation frame-transforms for given boneIndex.
		std::vector<BoneFrame> m_BoneFrames;
	};

	class GANYMEDE_API SkeletalMeshWorldObject : public MeshWorldObject
	{
	public:
		using MeshWorldObject::MeshWorldObject;
		Type GetType() const override { return Type::SkeletalMesh; }

		void AddBone(const Bone& boneInfo) { m_Bones.push_back(boneInfo); }
		const std::vector<Bone>& GetBones() const { return m_Bones; }
		unsigned int GetBoneCount() const { return m_Bones.size(); }

	private:
		std::vector<Bone> m_Bones;
	};
}