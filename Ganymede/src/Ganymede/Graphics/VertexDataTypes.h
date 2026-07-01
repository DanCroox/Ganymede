#pragma once

#include "Ganymede/Core/Core.h"

#include <vector>
#include <cstddef>
#include "Ganymede/World/MeshWorldObject.h"

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

    template<typename T>
    struct VertexDataDescriptor
    {
        using VertexDataType = T;
        VertexDataDescriptor() = default;
        virtual ~VertexDataDescriptor() = default;
    };

    #define M(member, primitiveType, numComponents) { primitiveType, numComponents, offsetof(VertexDataType, member) }

    #define VertexDataDefinition(type, basetype, ...)                                                           \
    struct type : public VertexDataDescriptor<basetype>                                                         \
    {                                                                                                           \
        GM_GENERATE_STATIC_CLASS_ID(type);                                                                      \
        static inline const std::vector<VertexDataPrimitiveTypeInfo>& GetVertexDataPrimitiveTypeInfo()          \
        {                                                                                                       \
            static const std::vector<VertexDataPrimitiveTypeInfo> vertexAttributePosition = { __VA_ARGS__ };    \
            return vertexAttributePosition;                                                                     \
        }                                                                                                       \
    };
    
    #define X(type, basetype, ...) VertexDataDefinition(type, basetype, __VA_ARGS__)
    #include "VertexDataTypes.def"
    #undef X
}