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
        m_UniformLocationCache(other.m_UniformLocationCache),
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
            m_UniformLocationCache = other.m_UniformLocationCache;
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

    void OGLGraphicsShader::BindTexture(RenderTarget& texture, const char* textureName)
    {
        if (m_ShaderTextureSlots.size() >= 31)
        {
            GM_CORE_ASSERT(false, "Couldn't bind texture to shader. No free texture slots.");
            return;
        }

        auto [it, inserted] = m_ShaderTextureSlots.try_emplace(textureName, m_NextAvailableTextureSlot);
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
        SetUniform1i(textureName, textureSlot);
        glActiveTexture(GL_TEXTURE31); // This is our "neutral" texture slot. We dont use it in game.
        static_cast<OGLRenderTarget&>(texture).UnBind();
    }

    void OGLGraphicsShader::SetUniform1i(const std::string& name, const int value) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform1i(GetUniformLocation(name), value);
    }

    void OGLGraphicsShader::SetUniform1iv(const std::string& name, const int* value, unsigned int count) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform1iv(GetUniformLocation(name), count, value);
    }

    void OGLGraphicsShader::SetUniform2i(const std::string& name, int v1, int v2) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform2i(GetUniformLocation(name), v1, v2);
    }

    void OGLGraphicsShader::SetUniform2iv(const std::string& name, const int* values, unsigned int count) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform3iv(GetUniformLocation(name), count, values);
    }

    void OGLGraphicsShader::SetUniform1f(const std::string& name, const float value) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform1f(GetUniformLocation(name), value);
    }

    void OGLGraphicsShader::SetUniform1fv(const std::string& name, const float* value, unsigned int count) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform1fv(GetUniformLocation(name), count, &value[0]);
    }

    void OGLGraphicsShader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
    }

    void OGLGraphicsShader::SetUniform3f(const std::string& name, float value1, float value2, float value3) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform3f(GetUniformLocation(name), value1, value2, value3);
    }

    void OGLGraphicsShader::SetUniform2f(const std::string& name, float value1, float value2) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform2f(GetUniformLocation(name), value1, value2);
    }

    void OGLGraphicsShader::SetUniform3f(const std::string& name, const glm::vec3& value) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
    }

    void OGLGraphicsShader::SetUniform2f(const std::string& name, const glm::vec2& value) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform2f(GetUniformLocation(name), value.x, value.y);
    }

    void OGLGraphicsShader::SetUniform3fv(const std::string& name, const float* values, unsigned int count) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniform3fv(GetUniformLocation(name), count, values);
    }

    void OGLGraphicsShader::SetUniformMat4f(const std::string& name, const glm::mat4* matrix, unsigned int count) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniformMatrix4fv(GetUniformLocation(name), count, GL_FALSE, glm::value_ptr(matrix[0]));
    }

    void OGLGraphicsShader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) const
    {
        OGLContext::BindShader(m_RendererID);
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    int OGLGraphicsShader::GetUniformLocation(const std::string& name) const
    {
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        {
            return m_UniformLocationCache[name];
        }

        int location = glGetUniformLocation(m_RendererID, name.c_str());
        if (location == -1)
        {
            // std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
        }

        m_UniformLocationCache[name] = location;

        return location;
    }
}