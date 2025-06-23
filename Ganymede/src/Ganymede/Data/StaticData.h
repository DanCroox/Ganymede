#pragma once

#include "SerializerTraits.h"
#include "Ganymede/Graphics/ShaderBinary.h"
#include "Ganymede/Graphics/Texture.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/PointlightWorldObject.h"
#include "Ganymede/World/SkeletalMeshWorldObject.h"
#include <vector>

namespace Ganymede
{
	class StaticData
	{
	public:
		GM_SERIALIZABLE(StaticData);

		std::vector<MeshWorldObject> m_MeshWorldObjects;
		std::vector<SkeletalMeshWorldObject> m_SkeletalMeshWorldObjects;
		std::vector<PointlightWorldObject> m_PointlightWorldObjects;
		std::vector<Texture> m_Textures;
		std::vector<Animation> m_SceneAnimations;
		std::vector<MeshWorldObject::Mesh> m_Meshes;
		std::vector<Material> m_Materials;
		std::vector<ShaderBinary> m_ShaderBinaries;

		static GANYMEDE_API StaticData* Instance;
	};
}