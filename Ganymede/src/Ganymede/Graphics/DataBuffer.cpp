#include "DataBuffer.h"
#include "Renderer.h"
#include "GL/glew.h"
#include "Ganymede/Log/Log.h"

namespace Ganymede
{
    namespace DataBufferNativeFunctions_Private
    {
        static int ToNativeBufferType(DataBufferType bufferType)
        {
            switch (bufferType)
            {
            case DataBufferType::Dynamic: return GL_DYNAMIC_DRAW;
            case DataBufferType::Static: return GL_STATIC_DRAW;
            default:
                GM_CORE_ASSERT(false, "Unsuppported vertex buffer type.");
                return -1;
            }
        }
    }
    
    unsigned int DataBufferNativeFunctions::GenerateBuffer()
    {
        unsigned int renderID;
        glGenBuffers(1, &renderID);
        GM_CORE_ASSERT(renderID != 0, "Couldn't create buffer.");
        return renderID;
    }

    void DataBufferNativeFunctions::DeleteBuffer(unsigned int renderID)
    {
        if (renderID != 0)
        {
            glDeleteBuffers(1, &renderID);
        }
    }

    void DataBufferNativeFunctions::BindBuffer(unsigned int renderID)
    {
        glBindBuffer(GL_ARRAY_BUFFER, renderID);
    }

    void DataBufferNativeFunctions::UnBindBuffer()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void DataBufferNativeFunctions::Write(const void* data, unsigned int numBytes, unsigned int byteOffset)
    {
        glBufferSubData(GL_ARRAY_BUFFER, byteOffset, numBytes, data);
    }

    void DataBufferNativeFunctions::InitializeBufferData(const void* data, unsigned int numBytes, DataBufferType bufferType)
    {
        glBufferData(GL_ARRAY_BUFFER, numBytes, data, DataBufferNativeFunctions_Private::ToNativeBufferType(bufferType));
    }
}