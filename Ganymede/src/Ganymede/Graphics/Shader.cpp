#include "Shader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "glm/glm.hpp"
#include "GL/glew.h"
#include <glm/gtc/type_ptr.hpp>
#include "Renderer.h"
#include "SSBO.h"

namespace Ganymede
{
    Shader::Shader(const std::string& filepath)
        :m_FilePath(filepath), m_RendererID(0)
    {
        ShaderProgramSource source = ParseShader(filepath);
        m_RendererID = CreateShader(source.VertexSource, source.FragmentSource, source.GeometrySource);
    }

    Shader::~Shader()
    {
        GLCall(glDeleteProgram(m_RendererID));
    }

    ShaderProgramSource Shader::ParseShader(const std::string& filepath)
    {
        std::ifstream stream(filepath);
        ASSERT(stream.good());

        enum class ShaderType : int
        {
            NONE = -1, VERTEX = 0, FRAGMENT = 1, GEOMETRY = 2
        };

        std::string line;
        std::stringstream ss[3];
        ShaderType type = ShaderType::NONE;
        while (getline(stream, line))
        {
            if (line.find("#shader") != std::string::npos)
            {
                if (line.find("vertex") != std::string::npos)
                {
                    type = ShaderType::VERTEX;
                }
                else if (line.find("fragment") != std::string::npos)
                {
                    type = ShaderType::FRAGMENT;
                }
                else if (line.find("geometry") != std::string::npos)
                {
                    type = ShaderType::GEOMETRY;
                }
            }
            else
            {
                std::stringstream& outBuffer = ss[static_cast<int>(type)];
                if (line.find("#include") != std::string::npos)
                {
                    ParseIncludeHierarchy(filepath, line, outBuffer);
                }
                else
                {
                    outBuffer << line << "\n";
                }
            }
        }

        return { ss[0].str(), ss[1].str(), ss[2].str() };
    }

    void Shader::ParseIncludeHierarchy(const std::string& filepath, const std::string& line, std::stringstream& stringOut)
    {
        // Parse #include line to find given file and create relative file path
        int selectionStart = line.find("\"");
        int selectionEnd = line.rfind("\"");
        if (selectionStart != std::string::npos && selectionEnd != std::string::npos)
        {
            ++selectionStart; //remove the search terms itself
            selectionEnd;
            const std::string includeFile = line.substr(selectionStart, selectionEnd - selectionStart);
            selectionEnd = filepath.rfind("/");
            std::string fileDirectory = filepath.substr(0, selectionEnd + 1);
            const std::string finalIncludeFilePath = fileDirectory + includeFile;

            //read include file
            std::ifstream stream(finalIncludeFilePath);
            if (!stream)
                ASSERT(false, "Shader include file does not exist!");
            std::string includeLine = line;
            while (std::getline(stream, includeLine))
            {
                if (includeLine.find("#include") != std::string::npos)
                {
                    // Recursively parse include files
                    // TODO: Detect circular dependencies and ignore multipe includes of same file
                    ParseIncludeHierarchy(filepath, includeLine, stringOut);
                }
                else
                {
                    stringOut << includeLine << "\n";
                }
            }
        }
    }

    unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
    {
        GLCall(unsigned int id = glCreateShader(type));
        const char* src = source.c_str();
        GLCall(glShaderSource(id, 1, &src, nullptr));
        GLCall(glCompileShader(id));

        int result;
        GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
        if (result == GL_FALSE)
        {
            int length;
            GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
            char* message = (char*)alloca(length * sizeof(char));
            GLCall(glGetShaderInfoLog(id, length, &length, message));

            std::string shaderTypeString;
            if (type == GL_VERTEX_SHADER)
            {
                shaderTypeString = "vertex";
            }
            else if (type == GL_FRAGMENT_SHADER)
            {
                shaderTypeString = "fragment";
            }
            else if (type == GL_GEOMETRY_SHADER)
            {
                shaderTypeString = "geometry";
            }
            std::cout << "Failed to compile " << shaderTypeString << "shader!" << std::endl;
            std::cout << message << std::endl;
            GLCall(glDeleteShader(id));

            return 0;
        }

        return id;
    }

    unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader)
    {
        const bool hasVS = vertexShader.size() > 0;
        const bool hasFS = fragmentShader.size() > 0;
        const bool hasGS = geometryShader.size() > 0;

        if (!hasVS)
        {
            ASSERT("No vertex shader found!");
            return 0;
        }

        unsigned int program = glCreateProgram();
        unsigned int vs = 0;
        unsigned int fs = 0;
        unsigned int gs = 0;

        if (hasVS)
        {
            unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
            GLCall(glAttachShader(program, vs));
        }

        if (hasFS)
        {
            unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
            GLCall(glAttachShader(program, fs));
        }

        if (hasGS)
        {
            unsigned int gs = CompileShader(GL_GEOMETRY_SHADER, geometryShader);
            GLCall(glAttachShader(program, gs));
        }

        GLCall(glLinkProgram(program));
        GLCall(glValidateProgram(program));

        if (hasVS)
        {
            GLCall(glDeleteShader(vs));
        }

        if (fragmentShader.size() > 0)
        {
            GLCall(glDeleteShader(fs));
        }

        if (geometryShader.size() > 0)
        {
            GLCall(glDeleteShader(gs));
        }

        return program;
    }

    void Shader::Bind() const
    {
        GLCall(glUseProgram(m_RendererID));
    }

    void Shader::Unbind() const
    {
        GLCall(glUseProgram(0));
    }

    void Shader::BindUBOBlock(const SSBO& ubo, const std::string& uniformBlockName) const
    {
        GLCall(int result = glGetUniformBlockIndex(m_RendererID, uniformBlockName.c_str()));
        if (result == GL_INVALID_INDEX)
        {
            ASSERT("Cant find uniform buffer block index!");
            return;
        }

        GLCall(glUniformBlockBinding(m_RendererID, result, ubo.GetBindingPointID()));
    }

    void Shader::SetUniform1i(const std::string& name, const int value) const
    {
        GLCall(glUniform1i(GetUniformLocation(name), value));
    }

    void Shader::SetUniform1iv(const std::string& name, const int* value, unsigned int count) const
    {
        glUniform1iv(GetUniformLocation(name), count, value);
    }

    void Shader::SetUniform2i(const std::string& name, int v1, int v2) const
    {
        GLCall(glUniform2i(GetUniformLocation(name), v1, v2));
    }

    void Shader::SetUniform2iv(const std::string& name, const int* values, unsigned int count) const
    {
        GLCall(glUniform3iv(GetUniformLocation(name), count, values));
    }

    void Shader::SetUniform1f(const std::string& name, const float value) const
    {
        GLCall(glUniform1f(GetUniformLocation(name), value));
    }

    void Shader::SetUniform1fv(const std::string& name, const float* value, unsigned int count) const
    {
        GLCall(glUniform1fv(GetUniformLocation(name), count, &value[0]));
    }

    void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const
    {
        GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
    }

    void Shader::SetUniform3f(const std::string& name, float value1, float value2, float value3) const
    {
        GLCall(glUniform3f(GetUniformLocation(name), value1, value2, value3));
    }

    void Shader::SetUniform2f(const std::string& name, float value1, float value2) const
    {
        GLCall(glUniform2f(GetUniformLocation(name), value1, value2));
    }

    void Shader::SetUniform3f(const std::string& name, const glm::vec3& value) const
    {
        GLCall(glUniform3f(GetUniformLocation(name), value.x, value.y, value.z));
    }

    void Shader::SetUniform2f(const std::string& name, const glm::vec2& value) const
    {
        GLCall(glUniform2f(GetUniformLocation(name), value.x, value.y));
    }

    void Shader::SetUniform3fv(const std::string& name, const float* values, unsigned int count) const
    {
        GLCall(glUniform3fv(GetUniformLocation(name), count, values));
    }

    void Shader::SetUniformMat4f(const std::string& name, const glm::mat4* matrix, unsigned int count) const
    {
        GLCall(glUniformMatrix4fv(GetUniformLocation(name), count, GL_FALSE, glm::value_ptr(matrix[0])));
    }

    void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) const
    {
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