#include "Shader.h"

#include "Ganymede/Log/Log.h"
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "OGLBindingHelper.h"
#include "RenderTarget.h"
#include "ShaderBinary.h"
#include "SSBO.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>

namespace Ganymede
{
    Shader::Shader(const ShaderBinary& shaderBinary)
    {
        const ShaderBinary::Binary& programBinary = shaderBinary.GetBinaryContainer()[0];

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

    Shader::~Shader()
    {
        OGLBindingHelper::BindShader(0);
        GLCall(glDeleteProgram(m_RendererID));
    }

    void Shader::BindTexture(RenderTarget& texture, const char* textureName)
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

        OGLBindingHelper::BindShader(m_RendererID);
        glActiveTexture(GL_TEXTURE0 + textureSlot);
        texture.Bind();
        SetUniform1i(textureName, textureSlot);
        glActiveTexture(GL_TEXTURE31); // This is our "neutral" texture slot. We dont use it in game.
        texture.UnBind();
    }

    void Shader::SetUniform1i(const std::string& name, const int value) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        GLCall(glUniform1i(GetUniformLocation(name), value));
    }

    void Shader::SetUniform1iv(const std::string& name, const int* value, unsigned int count) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        glUniform1iv(GetUniformLocation(name), count, value);
    }

    void Shader::SetUniform2i(const std::string& name, int v1, int v2) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        GLCall(glUniform2i(GetUniformLocation(name), v1, v2));
    }

    void Shader::SetUniform2iv(const std::string& name, const int* values, unsigned int count) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        GLCall(glUniform3iv(GetUniformLocation(name), count, values));
    }

    void Shader::SetUniform1f(const std::string& name, const float value) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        GLCall(glUniform1f(GetUniformLocation(name), value));
    }

    void Shader::SetUniform1fv(const std::string& name, const float* value, unsigned int count) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        GLCall(glUniform1fv(GetUniformLocation(name), count, &value[0]));
    }

    void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
    }

    void Shader::SetUniform3f(const std::string& name, float value1, float value2, float value3) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        GLCall(glUniform3f(GetUniformLocation(name), value1, value2, value3));
    }

    void Shader::SetUniform2f(const std::string& name, float value1, float value2) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        GLCall(glUniform2f(GetUniformLocation(name), value1, value2));
    }

    void Shader::SetUniform3f(const std::string& name, const glm::vec3& value) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        GLCall(glUniform3f(GetUniformLocation(name), value.x, value.y, value.z));
    }

    void Shader::SetUniform2f(const std::string& name, const glm::vec2& value) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        GLCall(glUniform2f(GetUniformLocation(name), value.x, value.y));
    }

    void Shader::SetUniform3fv(const std::string& name, const float* values, unsigned int count) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        GLCall(glUniform3fv(GetUniformLocation(name), count, values));
    }

    void Shader::SetUniformMat4f(const std::string& name, const glm::mat4* matrix, unsigned int count) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        GLCall(glUniformMatrix4fv(GetUniformLocation(name), count, GL_FALSE, glm::value_ptr(matrix[0])));
    }

    void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) const
    {
        OGLBindingHelper::BindShader(m_RendererID);
        GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix)));
    }

    int Shader::GetUniformLocation(const std::string& name) const
    {
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        {
            return m_UniformLocationCache[name];
        }

        GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
        if (location == -1)
        {
            // std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
        }

        m_UniformLocationCache[name] = location;

        return location;
    }
}