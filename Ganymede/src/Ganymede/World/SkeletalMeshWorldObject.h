#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include "WorldObject.h"
#include "MeshWorldObject.h"
#include "MeshWorldObjectInstance.h"
#include "Ganymede/Graphics/Texture.h"


struct BoneInfo
{
	glm::mat4 m_OffsetTransform;
	unsigned int m_Index;
};


struct BoneWeight
{
	unsigned int m_BoneIndex; //wie in vertex buffer
	float m_Weight;
};

struct Animation
{
	struct Bone
	{
		std::vector<glm::mat4> m_Frames;
	};

	std::string m_Name;
	std::vector<Bone> m_Bones;

	float m_FPS;
};

class SkeletalMeshWorldObject : public MeshWorldObject
{
	using MeshWorldObject::MeshWorldObject;
public:
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

	Type GetType() const override { return Type::SkeletalMesh; }

private:
	friend class AssetLoader;
	std::unordered_map<std::string, BoneInfo> m_BoneInfoByBoneName;
	unsigned int m_NumBones = 0;
};