#pragma once

#include "Ganymede/Core/Core.h"

#include "MeshWorldObject.h"

namespace Ganymede
{
	struct GANYMEDE_API BoneInfo
	{
		glm::mat4 m_OffsetTransform;
		unsigned int m_Index;
	};


	struct GANYMEDE_API BoneWeight
	{
		unsigned int m_BoneIndex; //wie in vertex buffer
		float m_Weight;
	};

	struct GANYMEDE_API Animation
	{
		struct Bone
		{
			std::vector<glm::mat4> m_Frames;
		};

		std::string m_Name;
		std::vector<Bone> m_Bones;

		float m_FPS;
	};

	class GANYMEDE_API SkeletalMeshWorldObject : public MeshWorldObject
	{
	public:
		using MeshWorldObject::MeshWorldObject;
		void SetBoneInfoByName(const std::string& name, const BoneInfo& boneInfo)
		{
			m_BoneInfoByBoneName[name] = boneInfo;
		}

		bool HasBoneName(const std::string& name)
		{
			return m_BoneInfoByBoneName.count(name) > 0;
		}

		bool TryGetBoneInfoByName(const std::string& name, BoneInfo& boneInfoOut) const
		{
			if (m_BoneInfoByBoneName.count(name) > 0)
			{
				boneInfoOut = m_BoneInfoByBoneName.at(name);
				return true;
			}

			return false;
		}

		unsigned int GetBoneCount() const
		{
			return m_NumBones;
		}

		//REWORK: Rework this bonecount- ref thingy...just done to fix the namespace dependencies
		unsigned int& GetBoneCountRef() { return m_NumBones; }
		const std::unordered_map<std::string, BoneInfo>& GetBoneInfoByBoneNameRef() const { return m_BoneInfoByBoneName; }

		Type GetType() const override { return Type::SkeletalMesh; }

	private:
		std::unordered_map<std::string, BoneInfo> m_BoneInfoByBoneName;
		unsigned int m_NumBones = 0;
	};
}