#pragma once

#include "Ganymede/Core/Core.h"

#include <vector>
#include <cstddef>
#include "Ganymede/World/MeshWorldObject.h"
#include "RenderBatch.h"

namespace Ganymede
{
    enum class VertexDataPrimitiveType
    {
        Char,
        UChar,
        Short,
        UShort,
        Int,
        UInt,
        Float,
        Double
    };

    struct VertexDataPrimitiveTypeInfo
    {
        VertexDataPrimitiveType m_PrimitiveType = VertexDataPrimitiveType::Char;
        unsigned int m_NumComponents = 0;
        unsigned int m_ByteOffset = 0;
    };

    struct VertexDataDescriptor
    {
        VertexDataDescriptor() = default;
        virtual ~VertexDataDescriptor() = default;
    };

    #define M(member, primitiveType, numComponents) { primitiveType, numComponents, offsetof(VertexDataType, member) }

    #define VertexDataDefinition(type, basetype, ...)                                                           \
    struct type : public VertexDataDescriptor                                                                   \
    {                                                                                                           \
        GM_GENERATE_STATIC_CLASS_ID(type);                                                                      \
        using VertexDataType = basetype;                                                                        \
        static inline const std::vector<VertexDataPrimitiveTypeInfo>& GetVertexDataPrimitiveTypeInfo()          \
        {                                                                                                       \
            static const std::vector<VertexDataPrimitiveTypeInfo> vertexAttributePosition = { __VA_ARGS__ };    \
            return vertexAttributePosition;                                                                     \
        }                                                                                                       \
    };

    VertexDataDefinition(MeshVertexData, MeshWorldObject::Mesh::Vertex,
        M(m_Position, VertexDataPrimitiveType::Float, 3),
        M(m_UV, VertexDataPrimitiveType::Float, 2),
        M(m_Normal, VertexDataPrimitiveType::Float, 3),
        M(m_Tangent, VertexDataPrimitiveType::Float, 3),
        M(m_Bitangent, VertexDataPrimitiveType::Float, 3),
        M(m_BoneIndex, VertexDataPrimitiveType::UInt, 4),
        M(m_BoneWeight, VertexDataPrimitiveType::Float, 4));

    VertexDataDefinition(MeshInstanceVertexData, IData,
        M(mv, VertexDataPrimitiveType::Float, 16),
        M(instance, VertexDataPrimitiveType::Float, 16),
        M(pid, VertexDataPrimitiveType::Float, 3)
        );

    VertexDataDefinition(Vec3VertexData, glm::vec3,
        M(x, VertexDataPrimitiveType::Float, 3)
    );

    VertexDataDefinition(Vec2VertexData, glm::vec2,
        M(x, VertexDataPrimitiveType::Float, 2)
    );
}