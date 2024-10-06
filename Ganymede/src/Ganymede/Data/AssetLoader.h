#pragma once

#include "Ganymede/Core/Core.h"

#include <iostream>
#include <vector>
#include <unordered_map>
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/Graphics/ShaderManager.h"
#include "Ganymede/World/SkeletalMeshWorldObject.h"


class aiAnimation;
class aiNode;
struct aiScene;
struct aiLight;
struct aiMesh;
struct aiTexture;

namespace Ganymede
{
	class Texture;

	class GANYMEDE_API AssetLoader
	{
	private:


	public:
		AssetLoader();
		AssetLoader(const AssetLoader&) = delete;
		AssetLoader& operator=(const AssetLoader&) = delete;

		~AssetLoader();

		std::vector<const WorldObject*> LoadFromPath(const std::string& path);
		const WorldObject* GetWorldObjectByName(const std::string& name) const;

		const Animation* GetAnimationByName(const std::string& name) const;

		const Texture* TryLoadTextureFromPath(const std::string& path);

		ShaderManager& GetShaderManager();

	private:
		ShaderManager m_ShaderManager;
		const Texture* TryLoadAndStoreRAWTexture(const aiTexture* rawTexture);

		void LoadNodeData(const aiNode& node, const aiScene& scene, std::vector<const WorldObject*>& loadedAssetsStorage, const std::unordered_map<std::string, aiLight*>& lightsByNameLookup);
		void LoadMesh(MeshWorldObject* meshWorldObject, const aiMesh& mesh, const aiNode& node, const aiScene& scene);
		void LoadBones(SkeletalMeshWorldObject* smwo, const aiMesh& mesh, const aiScene& scene);
		void LoadAnimation(const SkeletalMeshWorldObject& skeletalMwo, const aiAnimation& animation, const aiNode* rootNode);

		std::unordered_map<std::string, const WorldObject*> m_WorldObjects;
		std::unordered_map<std::string, const Texture*> m_Textures;
		std::unordered_map<std::string, Animation> m_SceneAnimations;
		std::unordered_map<std::string, MeshWorldObject::Mesh> m_WorldObjectMeshes;

		const Texture* m_DefaultWhite;
		const Texture* m_DefaultBlack;
		const Texture* m_DefaultNormal;
	};
}