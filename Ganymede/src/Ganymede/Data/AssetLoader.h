#pragma once

#include "Ganymede/Core/Core.h"

#include "Ganymede/Graphics/ShaderManager.h"
#include "Ganymede/Graphics/Texture.h"
#include "Ganymede/System/FreeList.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/SkeletalMeshWorldObject.h"
#include <iostream>
#include "Handle.h"
#include "StaticData.h"
#include <unordered_map>
#include <vector>
#include <optional>

class aiAnimation;
class aiNode;
struct aiScene;
struct aiLight;
struct aiMesh;
struct aiTexture;

namespace Ganymede
{
	class GANYMEDE_API AssetLoader
	{
	public:
		AssetLoader();
		AssetLoader(const AssetLoader&) = delete;
		AssetLoader& operator=(const AssetLoader&) = delete;

		void LoadFromPath(const std::string& path);

		ShaderManager& GetShaderManager();

		StaticData m_StaticData;

	private:
		std::unordered_map<std::string, size_t> m_MeshNameToIndex;
		std::unordered_map<std::string, size_t> m_TextureNameToIndex;
		std::unordered_map<std::string, size_t> m_SkeletalMeshToIndex;
		
		ShaderManager m_ShaderManager;

		std::optional<Handle<Texture>> TryLoadAndStoreRAWTexture(const aiTexture* rawTexture);
		std::optional<Handle<Texture>> TryLoadTextureFromPath(const std::string& path);

		void LoadNodeData(const aiNode& node, const aiScene& scene, const std::unordered_map<std::string, aiLight*>& lightsByNameLookup);
		void LoadMesh(MeshWorldObject* meshWorldObject, const aiMesh& mesh, const aiNode& node, const aiScene& scene);
		void LoadBones(SkeletalMeshWorldObject* smwo, const aiMesh& mesh, const aiScene& scene);
		void LoadAnimation(const SkeletalMeshWorldObject& skeletalMwo, const aiAnimation& animation, const aiNode* rootNode);

		Handle<Texture> m_DefaultWhite;
		Handle<Texture> m_DefaultBlack;
		Handle<Texture> m_DefaultNormal;
	};
}