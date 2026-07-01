#include "OGLGraphicsShader.h"

#include "Ganymede/Graphics/ShaderBinary.h"
#include "GL/glew.h"
#include "glm/gtc/type_ptr.hpp"
#include "OGLContext.h"
#include "OGLRenderTarget.h"

namespace Ganymede
{
    OGLGraphicsShader::OGLGraphicsShader(const ShaderBinary& shaderBinary) : GraphicsShader(shaderBinary)
    {
        const ShaderBinary::Binary& programBinary = shaderBinary.m_BinaryContainer[0];

        GM_CORE_ASSERT(!(programBinary.m_ShaderTypeBits & ShaderBinaryTypeBits::COMPUTE), "Trying to construct a graphics shader with a compute shader binary.");

        m_RendererID = glCreateProgram();
        GLenum binaryFormat = 0;
        glProgramBinary(m_RendererID,
            programBinary.m_DataFormat,
            programBinary.m_Data.data(),
            programBinary.m_Data.size());

        GLint linked = GL_FALSE;
        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &linked);
        GM_CORE_ASSERT(linked == GL_TRUE, "Program not linked");
    }

    OGLGraphicsShader::~OGLGraphicsShader()
    {
        glDeleteProgram(m_RendererID);
    }

    OGLGraphicsShader::OGLGraphicsShader(OGLGraphicsShader&& other) noexcept :
        GraphicsShader(std::move(other)),
        m_RendererID(other.m_RendererID),
        m_ShaderTextureSlots(other.m_ShaderTextureSlots),
        m_NextAvailableTextureSlot(other.m_NextAvailableTextureSlot)
    {
        other.m_RendererID = 0;
    }

    OGLGraphicsShader& OGLGraphicsShader::operator=(OGLGraphicsShader&& other) noexcept
    {
        if (this != &other)
        {
            GraphicsShader::operator=(std::move(other));
            m_RendererID = other.m_RendererID;
            m_ShaderTextureSlots = other.m_ShaderTextureSlots;
            m_NextAvailableTextureSlot = other.m_NextAvailableTextureSlot;

            other.m_RendererID = 0;
        }

        return *this;
    }

    void OGLGraphicsShader::Bind()
    {
        OGLContext::BindShader(m_RendererID);
    }

    void OGLGraphicsShader::Unbind()
    {
        OGLContext::UnbindShader(m_RendererID);
    }

    void OGLGraphicsShader::BindTexture(RenderTarget& texture, uint32_t bindingPoint)
    {
        if (m_ShaderTextureSlots.size() >= 31)
        {
            GM_CORE_ASSERT(false, "Couldn't bind texture to shader. No free texture slots.");
            return;
        }

        auto [it, inserted] = m_ShaderTextureSlots.try_emplace(bindingPoint, m_NextAvailableTextureSlot);
        int textureSlot;
        if (inserted)
        {
            textureSlot = m_NextAvailableTextureSlot;
            ++m_NextAvailableTextureSlot;
        }
        else
        {
            textureSlot = it->second;
        }

        OGLContext::BindShader(m_RendererID);
        glActiveTexture(GL_TEXTURE0 + textureSlot);
        static_cast<OGLRenderTarget&>(texture).Bind();
        SetUniform1i(bindingPoint, textureSlot);
        glActiveTexture(GL_TEXTURE31); // This is our "neutral" texture slot. We dont use it in game.
        static_cast<OGLRenderTarget&>(texture).UnBind();
    }

    void OGLGraphicsShader::SetUniform1i(uint32_t bindingPoint, const int value) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform1i(bindingPoint, value);
    }

    void OGLGraphicsShader::SetUniform1iv(uint32_t bindingPoint, const int* value, unsigned int count) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform1iv(bindingPoint, count, value);
    }

    void OGLGraphicsShader::SetUniform2i(uint32_t bindingPoint, int v1, int v2) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform2i(bindingPoint, v1, v2);
    }

    void OGLGraphicsShader::SetUniform2iv(uint32_t bindingPoint, const int* values, unsigned int count) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform3iv(bindingPoint, count, values);
    }

    void OGLGraphicsShader::SetUniform1f(uint32_t bindingPoint, const float value) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform1f(bindingPoint, value);
    }

    void OGLGraphicsShader::SetUniform1fv(uint32_t bindingPoint, const float* value, unsigned int count) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform1fv(bindingPoint, count, &value[0]);
    }

    void OGLGraphicsShader::SetUniform4f(uint32_t bindingPoint, float v0, float v1, float v2, float v3) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform4f(bindingPoint, v0, v1, v2, v3);
    }

    void OGLGraphicsShader::SetUniform3f(uint32_t bindingPoint, float value1, float value2, float value3) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform3f(bindingPoint, value1, value2, value3);
    }

    void OGLGraphicsShader::SetUniform2f(uint32_t bindingPoint, float value1, float value2) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform2f(bindingPoint, value1, value2);
    }

    void OGLGraphicsShader::SetUniform3f(uint32_t bindingPoint, const glm::vec3& value) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform3f(bindingPoint, value.x, value.y, value.z);
    }

    void OGLGraphicsShader::SetUniform2f(uint32_t bindingPoint, const glm::vec2& value) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform2f(bindingPoint, value.x, value.y);
    }

    void OGLGraphicsShader::SetUniform3fv(uint32_t bindingPoint, const float* values, unsigned int count) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform3fv(bindingPoint, count, values);
    }

    void OGLGraphicsShader::SetUniformMat4f(uint32_t bindingPoint, const glm::mat4* matrix, unsigned int count) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniformMatrix4fv(bindingPoint, count, GL_FALSE, glm::value_ptr(matrix[0]));
    }

    void OGLGraphicsShader::SetUniformMat4f(uint32_t bindingPoint, const glm::mat4& matrix) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniformMatrix4fv(bindingPoint, 1, GL_FALSE, glm::value_ptr(matrix));
    }
}