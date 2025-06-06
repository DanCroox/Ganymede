#include "AssetLoader.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/Graphics/ShaderManager.h"
#include "Ganymede/Graphics/Texture.h"
#include "Ganymede/World/PointlightWorldObject.h"
#include "Ganymede/World/WorldObject.h"
#include "glm/glm.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "stb_image.h"
#include "StaticData.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstdio>
#include <fstream>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace Ganymede
{
    namespace AssetLoader_Private
    {
        static glm::mat4 AssimpMatrixToGLM(const aiMatrix4x4& aiMatrix)
        {
            glm::mat4 glmGlobalTransform = {
                aiMatrix.a1, aiMatrix.b1, aiMatrix.c1, aiMatrix.d1,
                aiMatrix.a2, aiMatrix.b2, aiMatrix.c2, aiMatrix.d2,
                aiMatrix.a3, aiMatrix.b3, aiMatrix.c3, aiMatrix.d3,
                aiMatrix.a4, aiMatrix.b4, aiMatrix.c4, aiMatrix.d4
            };

            return glmGlobalTransform;
        }
    }

    AssetLoader::AssetLoader() :
        m_DefaultWhite(TryLoadTextureFromPath("res/textures/default/default_albedo.png").value()),
        m_DefaultBlack(TryLoadTextureFromPath("res/textures/default/default_black.png").value()),
        m_DefaultNormal(TryLoadTextureFromPath("res/textures/default/default_normal.png").value())
    {}

    void AssetLoader::LoadNodeData(const aiNode& node, const aiScene& scene, const std::unordered_map<std::string, aiLight*>& lightsByNameLookup)
    {
        const std::string nodename = std::string(node.mName.C_Str());
        auto it = lightsByNameLookup.find(nodename);
        if (it != lightsByNameLookup.end())
        {
            aiMatrix4x4 globalTransform;
            const aiNode* parentNode = &node;
            while (parentNode->mParent != nullptr)
            {
                globalTransform = parentNode->mTransformation * globalTransform;
                parentNode = parentNode->mParent;
            }

            PointlightWorldObject* pointlightWorldObject = &m_StaticData.m_PointlightWorldObjects.emplace_back(nodename);

            // Transpose ai matrix to glm matrix (column major)
            glm::mat4 glmGlobalTransform = {
                globalTransform.a1, globalTransform.b1, globalTransform.c1, globalTransform.d1,
                globalTransform.a2, globalTransform.b2, globalTransform.c2, globalTransform.d2,
                globalTransform.a3, globalTransform.b3, globalTransform.c3, globalTransform.d3,
                globalTransform.a4, globalTransform.b4, globalTransform.c4, globalTransform.d4
            };

            const aiLight* light = it->second;
            aiVector3D lightpos = light->mPosition;

            aiColor3D color = light->mColorDiffuse;
            float brightness = std::max(std::max(color.r, color.g), color.b);

            if (brightness > 0)
            {
                color.r /= brightness;
                color.g /= brightness;
                color.b /= brightness;
            }

            if (node.mMetaData != nullptr)
            {
                for (int i = 0; i < node.mMetaData->mNumProperties; ++i)
                {
                    aiString& key = node.mMetaData->mKeys[i];
                    aiMetadataEntry& entry = node.mMetaData->mValues[i];

                    // Check for custom property: Dynamic or Static physics
                    if ((entry.mType == aiMetadataType::AI_FLOAT || entry.mType == aiMetadataType::AI_DOUBLE))
                    {
                        double value = *reinterpret_cast<double*>(entry.mData);
                        pointlightWorldObject->SetImportance(static_cast<int>(value));
                    }
                }
            }

            pointlightWorldObject->SetTransform(glmGlobalTransform);
            pointlightWorldObject->SetColor(color.r, color.g, color.b);
            pointlightWorldObject->SetBrightness(brightness * .00005f);
        }

        if (node.mNumMeshes > 0)
        {
            // Load mesh object
            const char* name = node.mName.C_Str();

            bool isSkeletal = false;
            for (unsigned int i = 0; i < node.mNumMeshes; i++)
            {
                if (scene.mMeshes[node.mMeshes[i]]->HasBones())
                {
                    isSkeletal = true;
                }
            }

            MeshWorldObject* meshWorldObject = isSkeletal ? new SkeletalMeshWorldObject(name) : new MeshWorldObject(name);

            if (isSkeletal)
            {
                m_SkeletalMeshToIndex[name] = m_StaticData.m_SkeletalMeshWorldObjects.size();
                meshWorldObject = &m_StaticData.m_SkeletalMeshWorldObjects.emplace_back(name);
            }
            else
            {
                meshWorldObject = &m_StaticData.m_MeshWorldObjects.emplace_back(name);
            }

            if (node.mMetaData != nullptr)
            {
                for (int i = 0; i < node.mMetaData->mNumProperties; ++i)
                {
                    aiString& key = node.mMetaData->mKeys[i];
                    aiMetadataEntry& entry = node.mMetaData->mValues[i];

                    // Check for custom property: Dynamic or Static physics
                    if ((entry.mType == aiMetadataType::AI_BOOL) &&
                        std::strcmp(key.C_Str(), "Physics") == 0)
                    {
                        bool value = *static_cast<bool*>(entry.mData);
                        meshWorldObject->SetPreferredPhysicsState(value ? MeshWorldObject::PreferredPhysicsState::Dynamic : MeshWorldObject::PreferredPhysicsState::Static);
                    }
                    else if ((entry.mType == aiMetadataType::AI_BOOL) &&
                        std::strcmp(key.C_Str(), "ExcludeFromNavigation") == 0)
                    {
                        bool value = *static_cast<bool*>(entry.mData);
                        meshWorldObject->SetExcludeFromNavigationMesh(value);
                    }
                    else if ((entry.mType == aiMetadataType::AI_BOOL) &&
                        std::strcmp(key.C_Str(), "CastShadows") == 0)
                    {
                        bool value = *static_cast<bool*>(entry.mData);
                        meshWorldObject->SetCastShadows(value);
                    }
                }
            }

            aiMatrix4x4 globalTransform;
            const aiNode* parentNode = &node;
            while (parentNode->mParent != nullptr)
            {
                globalTransform = parentNode->mTransformation * globalTransform;
                parentNode = parentNode->mParent;
            }

            // Transpose ai matrix to glm matrix (column major)
            glm::mat4 glmGlobalTransform = {
                globalTransform.a1, globalTransform.b1, globalTransform.c1, globalTransform.d1,
                globalTransform.a2, globalTransform.b2, globalTransform.c2, globalTransform.d2,
                globalTransform.a3, globalTransform.b3, globalTransform.c3, globalTransform.d3,
                globalTransform.a4, globalTransform.b4, globalTransform.c4, globalTransform.d4
            };

            meshWorldObject->SetTransform(glmGlobalTransform);

            for (unsigned int i = 0; i < node.mNumMeshes; i++)
            {
                aiMesh& mesh = *scene.mMeshes[node.mMeshes[i]];
                LoadMesh(meshWorldObject, mesh, node, scene);

                if (mesh.HasBones())
                {
                    LoadBones(static_cast<SkeletalMeshWorldObject*>(meshWorldObject), mesh, scene);
                }
            }
        }

        // Load data from child nodes recursively
        for (unsigned int i = 0; i < node.mNumChildren; i++)
        {
            LoadNodeData(*node.mChildren[i], scene, lightsByNameLookup);
        }
    }

    glm::mat4 MakeBoneLocalTransform(const aiNodeAnim& bone, unsigned int frameNumber)
    {
        const aiVectorKey& position = bone.mPositionKeys[frameNumber];
        const aiQuatKey& rotation = bone.mRotationKeys[frameNumber];
        const aiVectorKey& scale = bone.mScalingKeys[frameNumber];

        const glm::mat4 translationMat = glm::translate(glm::identity<glm::mat4>(), glm::vec3(position.mValue.x, position.mValue.y, position.mValue.z));
        const glm::mat4 rotationMat = glm::mat4_cast(glm::quat(rotation.mValue.w, rotation.mValue.x, rotation.mValue.y, rotation.mValue.z));
        const glm::mat4 scaleMat = glm::scale(glm::identity<glm::mat4>(), glm::vec3(scale.mValue.x, scale.mValue.y, scale.mValue.z));

        return  translationMat * rotationMat * scaleMat;
    }

    const aiNode* FindNodeByName(const aiString& name, const aiNode* node)
    {
        if (node->mName == name)
            return node;

        for (unsigned int i = 0; i < node->mNumChildren; ++i)
        {
            const aiNode* foundNode = FindNodeByName(name, node->mChildren[i]);
            if (foundNode != nullptr)
                return foundNode;
        }

        return nullptr;
    }

    const aiNodeAnim* GetParentBone(const aiNodeAnim& bone, const aiNode& rootNode, aiNodeAnim** bones, unsigned int numBones)
    {
        const aiString* nodeNameToSearch = &bone.mNodeName;
        const aiNode* node;
        while (node = FindNodeByName(*nodeNameToSearch, &rootNode))
        {
            for (unsigned int i = 0; i < numBones; ++i)
            {
                const aiNodeAnim& currentBone = *bones[i];
                if (node->mParent->mName == currentBone.mNodeName)
                {
                    // parent bone found
                    return bones[i];
                }
            }
            break;
        }

        return nullptr;
    }

    void CalcBoneTransform(const SkeletalMeshWorldObject& smwo, const glm::mat4& boneOffsetTransform, const aiNodeAnim& bone, aiNodeAnim** bones, unsigned int numBones, const aiNode& rootNode, unsigned int frameNumber, glm::mat4& matrixOut)
    {
        const aiNode* node = FindNodeByName(bone.mNodeName, &rootNode);
        std::vector<const aiNode*> nodeHierarchy;
        nodeHierarchy.push_back(node);

        const aiNode* currentNode = node;
        while (currentNode = currentNode->mParent)
        {
            nodeHierarchy.insert(nodeHierarchy.begin(), currentNode);
            if (currentNode == &rootNode)
                break;
        }

        for (unsigned int i = 0; i < nodeHierarchy.size(); ++i)
        {
            const aiNode* cNode = nodeHierarchy[i];
            glm::mat4 bTransform = AssetLoader_Private::AssimpMatrixToGLM(cNode->mTransformation);

            for (unsigned int j = 0; j < numBones; ++j)
            {
                const aiNodeAnim* cBone = bones[j];
                if (cBone->mNodeName == cNode->mName)
                {
                    bTransform = MakeBoneLocalTransform(*cBone, frameNumber);
                    break;
                }
            }

            matrixOut = matrixOut * bTransform;
        }

        matrixOut = matrixOut * boneOffsetTransform;
    }

    void AssetLoader::LoadBones(SkeletalMeshWorldObject* smwo, const aiMesh& aiMesh, const aiScene& scene)
    {
        const size_t meshID = smwo->m_Meshes.back().GetID();
        std::vector<MeshWorldObject::Mesh::Vertex>& meshVertices = m_StaticData.m_Meshes[meshID].m_Vertices;

        std::vector<unsigned int> boneIndexCounter;
        boneIndexCounter.reserve(aiMesh.mNumVertices);
        for (unsigned int i = 0; i < aiMesh.mNumVertices; ++i)
        {
            boneIndexCounter.push_back(0);
        }

        for (unsigned int boneIdx = 0; boneIdx < aiMesh.mNumBones; ++boneIdx)
        {
            const aiBone& bone = *aiMesh.mBones[boneIdx];
            smwo->AddBone({ AssetLoader_Private::AssimpMatrixToGLM(bone.mOffsetMatrix), boneIdx, bone.mName.C_Str() });

            for (unsigned int weightIdx = 0; weightIdx < bone.mNumWeights; ++weightIdx)
            {
                const aiVertexWeight& vertexWeight = bone.mWeights[weightIdx];
                meshVertices[vertexWeight.mVertexId].m_BoneIndex[boneIndexCounter[vertexWeight.mVertexId]] = (float)boneIdx;
                meshVertices[vertexWeight.mVertexId].m_BoneWeight[boneIndexCounter[vertexWeight.mVertexId]] = vertexWeight.mWeight;

                ++(boneIndexCounter[vertexWeight.mVertexId]);
            }
        }
    }

    void AssetLoader::LoadAnimation(const SkeletalMeshWorldObject& skeletalMwo, const aiAnimation& animation, const aiNode* rootNode)
    {
        const aiNode* meshNode = FindNodeByName(aiString(skeletalMwo.GetName().c_str()), rootNode);

        std::string animationName = std::string(animation.mName.C_Str());

        Animation& anim = m_StaticData.m_SceneAnimations.emplace_back();
        anim.m_BoneFrames.resize(skeletalMwo.GetBoneCount());
        anim.m_FPS = animation.mDuration / animation.mTicksPerSecond;
        anim.m_FPS *= 10;

        for (const auto& boneInfo : skeletalMwo.GetBones())
        {
            const aiNodeAnim* bone = nullptr;

            for (unsigned int i = 0; i < animation.mNumChannels; ++i)
            {
                if (std::string(animation.mChannels[i]->mNodeName.C_Str()) == boneInfo.m_BoneName)
                {
                    bone = animation.mChannels[i];
                    break;
                }
            }

            if (bone == nullptr)
                continue;

            Animation::BoneFrame& animBone = anim.m_BoneFrames[boneInfo.m_Index];
            for (unsigned int frameIdx = 0; frameIdx < (*bone).mNumPositionKeys; ++frameIdx)
            {
                glm::mat4 finalBoneTransform = glm::identity<glm::mat4>();
                CalcBoneTransform(skeletalMwo, boneInfo.m_OffsetTransform, *bone, animation.mChannels, animation.mNumChannels, *meshNode->mParent, frameIdx, finalBoneTransform);
                animBone.push_back(finalBoneTransform);
            }
        }
    }

    void AssetLoader::LoadMesh(MeshWorldObject* meshWorldObject, const aiMesh& aiMesh, const aiNode& node, const aiScene& scene)
    {
        const std::string meshName = aiMesh.mName.C_Str();

        auto it = m_MeshNameToIndex.find(meshName);
        if (it != m_MeshNameToIndex.end())
        {
            // Mesh already loaded
            meshWorldObject->m_Meshes.push_back({it->second});
            return;
        }

        aiMaterial** materials = scene.mMaterials;

        const unsigned int materialIndex = aiMesh.mMaterialIndex;
        const aiMaterial* aiMaterial = materials[materialIndex];

        // Object might hold multiple meshes (e.g. due to different materials)
        // Add a new mesh to it and setup material data and vertex data

        const size_t meshID = m_StaticData.m_Meshes.size();

        MeshWorldObject::Mesh& meshData = m_StaticData.m_Meshes.emplace_back(meshID);
        m_MeshNameToIndex.emplace(meshName, meshID);
        meshWorldObject->m_Meshes.push_back({ meshID });
        
        Material& meshMaterial = meshData.m_Material;

        bool hasAlbedoTexture = false;
        bool hasNormalTexture = false;
        bool hasRoughnessTexture = false;
        bool hasMetallicTexture = false;
        bool hasEmissionTexture = false;

        {
            aiString str;
            aiMaterial->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &str);
            const aiTexture* textureAi = scene.GetEmbeddedTexture(str.C_Str());
            const std::optional<Handle<Texture>> textureHandle = TryLoadAndStoreRAWTexture(textureAi);
            hasAlbedoTexture = textureHandle.has_value();
            meshMaterial.AddMaterialTextureSamplerProperty("u_Texture0", hasAlbedoTexture ? textureHandle.value() : m_DefaultWhite);
        }
        {
            aiString str;
            aiMaterial->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &str);
            const aiTexture* textureAi = scene.GetEmbeddedTexture(str.C_Str());
            const std::optional<Handle<Texture>> textureHandle = TryLoadAndStoreRAWTexture(textureAi);
            hasNormalTexture = textureHandle.has_value();
            meshMaterial.AddMaterialTextureSamplerProperty("u_Texture1", hasNormalTexture ? textureHandle.value() : m_DefaultNormal);
        }
        {
            aiString str;
            aiMaterial->GetTexture(aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS, 0, &str);
            const aiTexture* textureAi = scene.GetEmbeddedTexture(str.C_Str());
            const std::optional<Handle<Texture>> textureHandle = TryLoadAndStoreRAWTexture(textureAi);
            hasRoughnessTexture = textureHandle.has_value();
            meshMaterial.AddMaterialTextureSamplerProperty("u_Texture2", hasRoughnessTexture ? textureHandle.value() : m_DefaultWhite);
        }
        {
            aiString str;
            aiMaterial->GetTexture(aiTextureType::aiTextureType_METALNESS, 0, &str);
            const aiTexture* textureAi = scene.GetEmbeddedTexture(str.C_Str());
            const std::optional<Handle<Texture>> textureHandle = TryLoadAndStoreRAWTexture(textureAi);
            hasMetallicTexture = textureHandle.has_value();
            meshMaterial.AddMaterialTextureSamplerProperty("u_Texture3", hasMetallicTexture ? textureHandle.value() : m_DefaultWhite);
        }
        {
            aiString str;
            aiMaterial->GetTexture(aiTextureType::aiTextureType_EMISSIVE, 0, &str);
            const aiTexture* textureAi = scene.GetEmbeddedTexture(str.C_Str());
            const std::optional<Handle<Texture>> textureHandle = TryLoadAndStoreRAWTexture(textureAi);
            hasEmissionTexture = textureHandle.has_value();
            meshMaterial.AddMaterialTextureSamplerProperty("u_Texture4", hasEmissionTexture ? textureHandle.value() : m_DefaultWhite);
        }

        aiMaterialProperty** prop = aiMaterial->mProperties;

        for (unsigned int i = 0; i < aiMaterial->mNumProperties; ++i)
        {
            aiString propName = prop[i]->mKey;
            unsigned int dataLen = prop[i]->mDataLength;
            char* data = prop[i]->mData;

            if (strcmp(propName.C_Str(), "?mat.name") == 0)
            {
                // First 4 bytes are name length, followed by characters and null terminator
                const std::string materialName(static_cast<const char*>(data) + sizeof(uint32_t));

                // Parse shader from material name. If none found, default shader will be loaded (defined in material constructor)
                const size_t pos = materialName.find("_S#");
                if (pos != -1)
                {
                    const std::string shaderName = materialName.substr(pos + 3, materialName.size());
                    Shader* shader = m_ShaderManager.RegisterAndLoadShader(shaderName.c_str());
                    if (shader != nullptr)
                    {
                        meshMaterial.SetShader(shader);
                    }
                }
            }

            if (strcmp(propName.C_Str(), "$clr.diffuse") == 0)
            {
                glm::vec3 baseColor;
                memcpy(&baseColor.x, data, 12);
                meshMaterial.AddMaterialVector3fProperty("u_BaseColor", hasAlbedoTexture ? glm::vec3(1) : baseColor);
            }
            else if (strcmp(propName.C_Str(), "$mat.roughnessFactor") == 0)
            {
                float roughness = *reinterpret_cast<float*>(data);
                meshMaterial.AddMaterialFloatProperty("u_Roughness", hasRoughnessTexture ? 1.f : roughness);
            }
            else if (strcmp(propName.C_Str(), "$mat.reflectivity") == 0)
            {
                float metalness = *reinterpret_cast<float*>(data);
                meshMaterial.AddMaterialFloatProperty("u_Metalness", hasMetallicTexture ? 1.f : metalness);
            }
            else if (strcmp(propName.C_Str(), "$clr.emissive") == 0)
            {
                glm::vec4 emissionColor;
                memcpy(&emissionColor.x, data, 16);
                meshMaterial.AddMaterialVector3fProperty("u_EmissiveColor", hasEmissionTexture ? glm::vec3(1) : glm::vec3(emissionColor));
            }
        }

        // Material does not have a special shader defined so we use default shader.
        if (meshMaterial.GetShader() == nullptr)
        {
            meshMaterial.SetShader(m_ShaderManager.RegisterAndLoadShader("gbuffer"));
        }

        const int vertexCount = aiMesh.mNumVertices;

        const aiVector3D* vertices = aiMesh.mVertices;
        const aiVector3D* uvs = aiMesh.mTextureCoords[0];
        const aiVector3D* normals = aiMesh.mNormals;
        const aiVector3D* tangents = aiMesh.mTangents;
        const aiVector3D* bitangents = aiMesh.mBitangents;
        const aiColor4D* colors = aiMesh.mColors[0];
        const aiFace* faces = aiMesh.mFaces;

        MeshWorldObject::Mesh::AABB& boundingBoxVertices = meshData.m_BoundingBoxVertices;
        for (unsigned int i = 0; i < vertexCount; ++i)
        {
            MeshWorldObject::Mesh::Vertex vertex;

            vertex.m_Position = { vertices[i].x, vertices[i].y, vertices[i].z };
            vertex.m_UV = { uvs[i].x, uvs[i].y };
            vertex.m_Normal = { normals[i].x, normals[i].y, normals[i].z };
            vertex.m_Tangent = { tangents[i].x, tangents[i].y, tangents[i].z };
            vertex.m_Bitangent = { bitangents[i].x, bitangents[i].y, bitangents[i].z };

            if (colors != nullptr)
                vertex.m_Color = { colors[i].r, colors[i].g, colors[i].b, colors[i].a };

            meshData.m_Vertices.push_back(vertex);

            if (i == 0)
            {
                // Set first vertex to not rely on default 0-value. if all vertices lay in a negative space the bounding box would be wrong
                boundingBoxVertices[0].m_Position = vertex.m_Position;
                boundingBoxVertices[1].m_Position = vertex.m_Position;
                boundingBoxVertices[2].m_Position = vertex.m_Position;
                boundingBoxVertices[3].m_Position = vertex.m_Position;

                boundingBoxVertices[4].m_Position = vertex.m_Position;
                boundingBoxVertices[5].m_Position = vertex.m_Position;
                boundingBoxVertices[6].m_Position = vertex.m_Position;
                boundingBoxVertices[7].m_Position = vertex.m_Position;
            }


            // Top Left Front
            boundingBoxVertices[0].m_Position.x = glm::min(boundingBoxVertices[0].m_Position.x, vertex.m_Position.x);
            boundingBoxVertices[0].m_Position.y = glm::max(boundingBoxVertices[0].m_Position.y, vertex.m_Position.y);
            boundingBoxVertices[0].m_Position.z = glm::max(boundingBoxVertices[0].m_Position.z, vertex.m_Position.z);
            boundingBoxVertices[0].m_Normal = glm::vec3(-.333333f, .333333f, .333333f);

            // Top Right Front
            boundingBoxVertices[1].m_Position.x = glm::max(boundingBoxVertices[1].m_Position.x, vertex.m_Position.x);
            boundingBoxVertices[1].m_Position.y = glm::max(boundingBoxVertices[1].m_Position.y, vertex.m_Position.y);
            boundingBoxVertices[1].m_Position.z = glm::max(boundingBoxVertices[1].m_Position.z, vertex.m_Position.z);
            boundingBoxVertices[1].m_Normal = glm::vec3(.333333f, .333333f, .333333f);

            // Bottom Right Front
            boundingBoxVertices[2].m_Position.x = glm::max(boundingBoxVertices[2].m_Position.x, vertex.m_Position.x);
            boundingBoxVertices[2].m_Position.y = glm::min(boundingBoxVertices[2].m_Position.y, vertex.m_Position.y);
            boundingBoxVertices[2].m_Position.z = glm::max(boundingBoxVertices[2].m_Position.z, vertex.m_Position.z);
            boundingBoxVertices[2].m_Normal = glm::vec3(.333333f, -.333333f, .333333f);

            // Bottom Left Front
            boundingBoxVertices[3].m_Position.x = glm::min(boundingBoxVertices[3].m_Position.x, vertex.m_Position.x);
            boundingBoxVertices[3].m_Position.y = glm::min(boundingBoxVertices[3].m_Position.y, vertex.m_Position.y);
            boundingBoxVertices[3].m_Position.z = glm::max(boundingBoxVertices[3].m_Position.z, vertex.m_Position.z);
            boundingBoxVertices[3].m_Normal = glm::vec3(-.333333f, -.333333f, .333333f);

            //Top Left Back
            boundingBoxVertices[4].m_Position.x = glm::min(boundingBoxVertices[4].m_Position.x, vertex.m_Position.x);
            boundingBoxVertices[4].m_Position.y = glm::max(boundingBoxVertices[4].m_Position.y, vertex.m_Position.y);
            boundingBoxVertices[4].m_Position.z = glm::min(boundingBoxVertices[4].m_Position.z, vertex.m_Position.z);
            boundingBoxVertices[4].m_Normal = glm::vec3(-.333333f, .333333f, -.333333f);

            // Top Right Back
            boundingBoxVertices[5].m_Position.x = glm::max(boundingBoxVertices[5].m_Position.x, vertex.m_Position.x);
            boundingBoxVertices[5].m_Position.y = glm::max(boundingBoxVertices[5].m_Position.y, vertex.m_Position.y);
            boundingBoxVertices[5].m_Position.z = glm::min(boundingBoxVertices[5].m_Position.z, vertex.m_Position.z);
            boundingBoxVertices[5].m_Normal = glm::vec3(.333333f, .333333f, -.333333f);

            // Bottom Right Back
            boundingBoxVertices[6].m_Position.x = glm::max(boundingBoxVertices[6].m_Position.x, vertex.m_Position.x);
            boundingBoxVertices[6].m_Position.y = glm::min(boundingBoxVertices[6].m_Position.y, vertex.m_Position.y);
            boundingBoxVertices[6].m_Position.z = glm::min(boundingBoxVertices[6].m_Position.z, vertex.m_Position.z);
            boundingBoxVertices[6].m_Normal = glm::vec3(.333333f, -.333333f, -.333333f);

            // Bottom Left Back
            boundingBoxVertices[7].m_Position.x = glm::min(boundingBoxVertices[7].m_Position.x, vertex.m_Position.x);
            boundingBoxVertices[7].m_Position.y = glm::min(boundingBoxVertices[7].m_Position.y, vertex.m_Position.y);
            boundingBoxVertices[7].m_Position.z = glm::min(boundingBoxVertices[7].m_Position.z, vertex.m_Position.z);
            boundingBoxVertices[7].m_Normal = glm::vec3(-.333333f, -.333333f, -.333333f);

        }

        // Find center of bounding box
        meshData.m_BoundingBoxCenter = glm::vec3(0);
        meshData.m_BoundingBoxHalfSize = glm::vec3(0);
        for (const MeshWorldObject::Mesh::BoundingBoxVertex& bbVertex : boundingBoxVertices)
        {
            meshData.m_BoundingBoxCenter += bbVertex.m_Position;
        }
        meshData.m_BoundingBoxCenter /= boundingBoxVertices.size();

        meshData.m_BoundingBoxHalfSize.x = meshData.m_BoundingBoxCenter.x + boundingBoxVertices[1].m_Position.x;
        meshData.m_BoundingBoxHalfSize.y = meshData.m_BoundingBoxCenter.y + boundingBoxVertices[1].m_Position.y;
        meshData.m_BoundingBoxHalfSize.z = meshData.m_BoundingBoxCenter.z - boundingBoxVertices[1].m_Position.z;

        for (unsigned int i = 0; i < aiMesh.mNumFaces; ++i)
        {
            meshData.m_VertexIndicies.push_back(faces[i].mIndices[0]);
            meshData.m_VertexIndicies.push_back(faces[i].mIndices[1]);
            meshData.m_VertexIndicies.push_back(faces[i].mIndices[2]);
        }
    }

    void AssetLoader::LoadFromPath(const std::string& path)
    {
        std::vector<const WorldObject*> loadedAssetsStorage;

        Assimp::Importer importer;
        unsigned int assimp_read_flag = aiProcess_Triangulate |
            aiProcess_SortByPType |
            aiProcess_OptimizeMeshes |
            aiProcess_ValidateDataStructure |
            aiProcess_GenNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_LimitBoneWeights |
            aiProcess_JoinIdenticalVertices;
        //importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
        const aiScene* fbxPtr = importer.ReadFile(path, assimp_read_flag);

        if (fbxPtr == nullptr)
        {
            // Cant load fbx
            return;
        }

        std::unordered_map<std::string, aiLight*> lightsByNameLookup;
        const unsigned int lightCount = fbxPtr->mNumLights;
        for (unsigned int i = 0; i < lightCount; ++i)
        {
            aiLight* light = fbxPtr->mLights[i];
            lightsByNameLookup.insert(std::make_pair(std::string(light->mName.C_Str()), light));
        }

        const aiScene& scene = *fbxPtr;
        LoadNodeData(*fbxPtr->mRootNode, scene, lightsByNameLookup);

        if (scene.HasAnimations())
        {
            for (unsigned int animationIdx = 0; animationIdx < scene.mNumAnimations; ++animationIdx)
            {
                const aiAnimation& animation = *scene.mAnimations[animationIdx];
                if (std::strcmp(animation.mName.C_Str(), "Idle") == 0 || std::strcmp(animation.mName.C_Str(), "Walking") == 0)
                {
                    if (auto it = m_SkeletalMeshToIndex.find(std::string("Matschkopf")); it != m_SkeletalMeshToIndex.end())
                    {
                        LoadAnimation(m_StaticData.m_SkeletalMeshWorldObjects[it->second], animation, scene.mRootNode);
                    }
                }
            }
        }
    }

    std::optional<Handle<Texture>> AssetLoader::TryLoadTextureFromPath(const std::string& path)
    {
        const std::string textureName = Helpers::ParseFileNameFromPath(path);

        const Texture* texture = nullptr;
        auto textureSearchIt = m_TextureNameToIndex.find(textureName);
        if (textureSearchIt == m_TextureNameToIndex.end())
        {
            const size_t textureID = m_StaticData.m_Textures.size();

            m_TextureNameToIndex.emplace(textureName, textureID);
            m_StaticData.m_Textures.emplace_back(path);
            return Handle<Texture> { textureID };
        }
        else
        {
            const size_t textureID = textureSearchIt->second;
            return Handle<Texture> { textureID };
        }
    }

    ShaderManager& AssetLoader::GetShaderManager()
    {
        return m_ShaderManager;
    }

    std::optional<Handle<Texture>> AssetLoader::TryLoadAndStoreRAWTexture(const aiTexture* rawTexture)
    {
        if (rawTexture != nullptr)
        {
            std::string name = Helpers::ParseFileNameFromPath(rawTexture->mFilename.C_Str());

            auto textureSearchIt = m_TextureNameToIndex.find(name);
            if (textureSearchIt == m_TextureNameToIndex.end())
            {
                const unsigned int pixelsTotal = rawTexture->mWidth; // mHeight is 0 if embedded. mWidth is the byte count
                unsigned char* bytes = reinterpret_cast<unsigned char*>(rawTexture->pcData);

                // Texture not cached yet. Load and everything
                stbi_set_flip_vertically_on_load(1);
                int width;
                int height;
                int channelCount;
                unsigned char* buffer = stbi_load_from_memory(bytes, pixelsTotal, &width, &height, &channelCount, 4);

                if (buffer)
                {
                    const size_t textureID = m_StaticData.m_Textures.size();
                    m_TextureNameToIndex.emplace(name, textureID);
                    const Texture* texture = &m_StaticData.m_Textures.emplace_back(buffer, width, height, channelCount, textureID);
                    stbi_image_free(buffer);
                    return Handle<Texture> { textureID };
                }

                GM_CORE_ASSERT(false, "Couldn't load texture.");
                // Couldnt load texture
                {
                    return std::nullopt;
                }
            }
            else
            {
                // Texture already in cache. Reuse!
                const size_t textureID = textureSearchIt->second;
                return Handle<Texture>{ textureID };
            }
        }

        return std::nullopt;
    }
}