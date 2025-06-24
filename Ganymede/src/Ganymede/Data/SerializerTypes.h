#pragma once

#include "Ganymede/Data/StaticData.h"
#include "Ganymede/Graphics/Material.h"
#include "Ganymede/Graphics/ShaderBinary.h"
#include "Ganymede/Graphics/Texture.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/PointlightWorldObject.h"
#include "Ganymede/World/SkeletalMeshWorldObject.h"
#include "Ganymede/World/WorldObject.h"

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/ext/std_map.h>
#include <bitsery/ext/std_variant.h>
#include <bitsery/traits/array.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>

#include <variant>

namespace bitsery
{
    // Common
    template<typename S>
    void serialize(S& s, glm::vec2& vec)
    {
        s.value4b(vec.x);
        s.value4b(vec.y);
    }

    template<typename S>
    void serialize(S& s, glm::vec3& vec)
    {
        s.value4b(vec.x);
        s.value4b(vec.y);
        s.value4b(vec.z);
    }

    template<typename S>
    void serialize(S& s, glm::vec4& vec)
    {
        s.value4b(vec.x);
        s.value4b(vec.y);
        s.value4b(vec.z);
        s.value4b(vec.a);
    }

    template<typename S>
    void serialize(S& s, glm::mat4& mat)
    {
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
                s.value4b(mat[row][col]);
    }
}

namespace Ganymede
{
    // Handle
    template<typename S, typename T>
    void serialize(S& s, Handle<T>& handle)
    {
        static_assert(sizeof(size_t) == 8, "size_t must be 8 byte");
        s.value8b(handle.m_Index);
    }

    // WorldObject
    template<typename S>
    void serialize(S& s, WorldObject& wo)
    {
        static constexpr size_t MAX_NAME_LEN = 255;
        s.text1b(wo.m_Name, MAX_NAME_LEN);
        s.object(wo.m_Transform);
    }

    // MeshWorldObject
    template<typename S>
    void serialize(S& s, MeshWorldObject::Mesh::BoundingBoxVertex& bbox)
    {
        s.object(bbox.m_Position);
        s.object(bbox.m_Normal);
    }

    template<typename S>
    void serialize(S& s, MeshWorldObject::Mesh::Vertex& vertex)
    {
        s.object(vertex.m_Position);
        s.object(vertex.m_UV);
        s.object(vertex.m_Normal);
        s.object(vertex.m_Tangent);
        s.object(vertex.m_Bitangent);
        s.object(vertex.m_Color);

        s.value4b(vertex.m_BoneIndex[0]);
        s.value4b(vertex.m_BoneIndex[1]);
        s.value4b(vertex.m_BoneIndex[2]);
        s.value4b(vertex.m_BoneIndex[3]);

        s.value4b(vertex.m_BoneWeight[0]);
        s.value4b(vertex.m_BoneWeight[1]);
        s.value4b(vertex.m_BoneWeight[2]);
        s.value4b(vertex.m_BoneWeight[3]);
    }

    template<typename S>
    void serialize(S& s, MeshWorldObject::Mesh& mesh)
    {
        s.object(mesh.m_MaterialHandle);

        static constexpr size_t MAX_VERTEX_INDICES = 10000000;
        s.container(mesh.m_VertexIndicies, MAX_VERTEX_INDICES, [](S& s, unsigned int& idx)
            {
                s.value4b(idx);
            });

        static constexpr size_t MAX_VERTICES = 10000000;
        s.container(mesh.m_Vertices, MAX_VERTICES, [](S& s, MeshWorldObject::Mesh::Vertex& v)
            {
                s.object(v);
            });

        s.container(mesh.m_BoundingBoxVertices, [](S& s, MeshWorldObject::Mesh::BoundingBoxVertex& bbox)
            {
                s.object(bbox);
            });

        static constexpr size_t MAX_BBOX_INDICES = 36;
        s.container(mesh.m_BoundingBoxIndices, MAX_BBOX_INDICES, [](S& s, unsigned int& idx)
            {
                s.value4b(idx);
            });

        s.object(mesh.m_BoundingBoxCenter);
        s.object(mesh.m_BoundingBoxHalfSize);
        s.value8b(mesh.m_MeshID);
    }

    template<typename S>
    void serialize(S& s, MeshWorldObject& mwo)
    {
        serialize(s, static_cast<WorldObject&>(mwo));

        static constexpr size_t MAX_NUM_MESHES = 1000;
        s.container(mwo.m_Meshes, MAX_NUM_MESHES, [](S& s, Handle<MeshWorldObject::Mesh>& handle)
            {
                s.object(handle);
            });

        s.value1b(mwo.m_PreferredPhysicsState);
        s.boolValue(mwo.m_ExcludeFromNavigationMesh);
        s.boolValue(mwo.m_CastShadows);
    }

    // SkeletalMeshWorldObject
    template<typename S>
    void serialize(S& s, Bone& bone)
    {
        s.object(bone.m_OffsetTransform);
        s.value4b(bone.m_Index);

        static constexpr size_t MAX_NAME_LEN = 255;
        s.text1b(bone.m_BoneName, MAX_NAME_LEN);
    }
    
    template<typename S>
    void serialize(S& s, SkeletalMeshWorldObject& smesh)
    {
        serialize(s, static_cast<MeshWorldObject&>(smesh));

        static constexpr size_t MAX_NUM_BONES = 200;
        s.container(smesh.m_Bones, MAX_NUM_BONES, [](S& s, Bone& bone)
            {
                s.object(bone);
            });
    }

    // PointlightWorldObject
    template<typename S>
    void serialize(S& s, PointlightWorldObject& plwo)
    {
        serialize(s, static_cast<WorldObject&>(plwo));

        s.object(plwo.m_Color);
        s.value4b(plwo.m_Brightness);
        s.value4b(plwo.m_Importance);
    }

    // Texture
    template<typename S>
    void serialize(S& s, Texture& texture)
    {
        s.value4b(texture.m_Width);
        s.value4b(texture.m_Height);
        s.value4b(texture.m_ChannelCount);
        s.value4b(texture.m_BitDepth);

        constexpr size_t MAX_TEXTURE_BYTES = 100 * 1000 * 1000;
        s.container1b(texture.m_Bytes, MAX_TEXTURE_BYTES);
    }

    // Animation
    template<typename S>
    void serialize(S& s, Animation& anim)
    {
        s.value4b(anim.m_FPS);

        static constexpr size_t MAX_NAME_LEN = 255;
        s.text1b(anim.m_Name, MAX_NAME_LEN);

        static constexpr size_t MAX_NUM_BONES = 150;
        static constexpr size_t MAX_NUM_FRAMES_PER_BONE = 5000;
        s.container(anim.m_BoneFrames, MAX_NUM_BONES, [](S& s, std::vector<glm::mat4>& bones)
            {
                s.container(bones, MAX_NUM_FRAMES_PER_BONE, [](S& s, glm::mat4& framesTransforms)
                    {
                        s.object(framesTransforms);
                    });
            });
    }

    // Material
    template<typename S>
    void serialize(S& s, Material::MaterialProperty& matProp)
    {
        s.ext(matProp.m_Data, bitsery::ext::StdVariant{
                 [](S& s, float& value) { s.value4b(value); },
                 [](S& s, glm::vec3& value) { s.value4b(value.x); s.value4b(value.y); s.value4b(value.z); },
                 [](S& s, Handle<Texture>& value) { s.object(value); }
            });
    }

    template<typename S>
    void serialize(S& s, Material& material)
    {
        constexpr size_t MAX_NUM_MATERIAL_PROPERTIES = 100;
        constexpr size_t MAX_MATERIAL_PROPERTY_NAME_LEN = 255;
        s.ext(material.m_MaterialProperties, bitsery::ext::StdMap{ MAX_NUM_MATERIAL_PROPERTIES }, [MAX_MATERIAL_PROPERTY_NAME_LEN](S& s, std::string &key, Material::MaterialProperty& value) {
            s.text1b(key, MAX_MATERIAL_PROPERTY_NAME_LEN);
            s.object(value);
            });

        s.object(material.m_ShaderBinaryHandle);
    }

    // ShaderBinary
    template<typename S>
    void serialize(S& s, ShaderBinary::Binary& bin)
    {
        constexpr size_t MAX_SHADER_STAGE_BYTES = 10000000;
        s.container1b(bin.m_Data, MAX_SHADER_STAGE_BYTES);

        s.value4b(bin.m_DataFormat);
    }

    template<typename S>
    void serialize(S& s, ShaderBinary& shaderBinary)
    {
        static constexpr size_t MAX_PATH_LEN = 255;
        s.text1b(shaderBinary.m_FilePath, MAX_PATH_LEN);

        static constexpr size_t MAX_NUM_BINARY_STAGES = 4;
        s.container(shaderBinary.m_BinaryContainer, MAX_NUM_BINARY_STAGES, [](S& s, ShaderBinary::Binary& bin)
            {
                s.object(bin);
            });
    }

    // StaticData
    template<typename S>
    void serialize(S& s, StaticData& staticData)
    {
        static constexpr size_t MAX_NUM_WORLDOBJECT_PER_TYPE = 10000;
        s.container(staticData.m_MeshWorldObjects, MAX_NUM_WORLDOBJECT_PER_TYPE, [](S& s, MeshWorldObject& m)
            {
                s.object(m);
            });
        
        s.container(staticData.m_SkeletalMeshWorldObjects, MAX_NUM_WORLDOBJECT_PER_TYPE, [](S& s, SkeletalMeshWorldObject& m)
            {
                s.object(m);
            });

        s.container(staticData.m_PointlightWorldObjects, MAX_NUM_WORLDOBJECT_PER_TYPE, [](S& s, PointlightWorldObject& m)
            {
                s.object(m);
            });

        s.container(staticData.m_Textures, MAX_NUM_WORLDOBJECT_PER_TYPE, [](S& s, Texture& m)
            {
                s.object(m);
            });

        s.container(staticData.m_SceneAnimations, MAX_NUM_WORLDOBJECT_PER_TYPE, [](S& s, Animation& m)
            {
                s.object(m);
            });

        s.container(staticData.m_Meshes, MAX_NUM_WORLDOBJECT_PER_TYPE, [](S& s, MeshWorldObject::Mesh& m)
            {
                s.object(m);
            });

        s.container(staticData.m_Materials, MAX_NUM_WORLDOBJECT_PER_TYPE, [](S& s, Material& m)
            {
                s.object(m);
            });

        s.container(staticData.m_ShaderBinaries, MAX_NUM_WORLDOBJECT_PER_TYPE, [](S& s, ShaderBinary& m)
            {
                s.object(m);
            });
    }
}