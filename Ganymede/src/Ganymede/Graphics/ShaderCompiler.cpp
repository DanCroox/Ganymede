#include "ShaderCompiler.h"

#include "Ganymede/Log/Log.h"
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "SSBO.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>

namespace Ganymede
{
    bool ShaderCompiler::Compile(const std::string& filePath, std::vector<ShaderBinary::Binary>& binaryDataOut)
    {
        ProgramSource source = ParseShader(filePath);
        const bool compilationSuccess = CompileProgram(source, binaryDataOut);

        GM_CORE_ASSERT(compilationSuccess, "Shader compilation failed");

        return compilationSuccess;
    }

    ShaderCompiler::ProgramSource ShaderCompiler::ParseShader(const std::string& filepath)
    {
        std::ifstream stream(filepath);
        GM_CORE_ASSERT(stream.good(), "Couldn't load shader source from path.");

        std::string line;
        std::stringstream ss[static_cast<int>(ShaderBinary::ShaderType::_COUNT)];
        ShaderBinary::ShaderType type = ShaderBinary::ShaderType::NONE;
        while (getline(stream, line))
        {
            if (line.find("#shader") != std::string::npos)
            {
                if (line.find("compute") != std::string::npos)
                {
                    type = ShaderBinary::ShaderType::COMPUTE;
                }
                else if (line.find("vertex") != std::string::npos)
                {
                    type = ShaderBinary::ShaderType::VERTEX;
                }
                else if (line.find("fragment") != std::string::npos)
                {
                    type = ShaderBinary::ShaderType::FRAGMENT;
                }
                else if (line.find("geometry") != std::string::npos)
                {
                    type = ShaderBinary::ShaderType::GEOMETRY;
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

        return { ss[0].str(), ss[1].str(), ss[2].str(), ss[3].str() };
    }

    void ShaderCompiler::ParseIncludeHierarchy(const std::string& filepath, const std::string& line, std::stringstream& stringOut)
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
                GM_CORE_ASSERT(false, "Shader include file does not exist!");
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

    bool ShaderCompiler::CompileProgram(const ProgramSource& programSource, std::vector<ShaderBinary::Binary>& binaryDataOut)
    {
        const bool hasCS = programSource.ComputeSource.size() > 0;
        const bool hasVS = programSource.VertexSource.size() > 0;
        const bool hasFS = programSource.FragmentSource.size() > 0;
        const bool hasGS = programSource.GeometrySource.size() > 0;

        if (!hasCS && !hasVS)
        {
            // All shaders except compute require a vertex shader
            GM_CORE_ASSERT(false, "No vertex shader found!");
            return false;
        }

        unsigned int program = glCreateProgram();
        unsigned int cs = 0;
        unsigned int vs = 0;
        unsigned int fs = 0;
        unsigned int gs = 0;

        if (hasCS)
        {
            cs = CompileShaderShaderStage(GL_COMPUTE_SHADER, programSource.ComputeSource);
            GLCall(glAttachShader(program, cs));
        }

        if (hasVS)
        {
            vs = CompileShaderShaderStage(GL_VERTEX_SHADER, programSource.VertexSource);
            GLCall(glAttachShader(program, vs));
        }

        if (hasFS)
        {
            fs = CompileShaderShaderStage(GL_FRAGMENT_SHADER, programSource.FragmentSource);
            GLCall(glAttachShader(program, fs));
        }

        if (hasGS)
        {
            gs = CompileShaderShaderStage(GL_GEOMETRY_SHADER, programSource.GeometrySource);
            GLCall(glAttachShader(program, gs));
        }

        glProgramParameteri(program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);

        glLinkProgram(program);
        glValidateProgram(program);

        GLint linked = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE)
        {
            GM_CORE_ASSERT(linked == GL_TRUE, "Program not properly created/linked.");
            return false;
        }

        int binaryLength = 0;
        glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &binaryLength);

        ShaderBinary::Binary& binary = binaryDataOut.emplace_back();
        binary.m_Data.resize(binaryLength);

        // Binary und Format auslesen
        glGetProgramBinary(program,
            binaryLength,
            &binaryLength,  // zurückgegebene Länge
            &binary.m_DataFormat,  // Treiberspezifisches Format
            binary.m_Data.data());
        glDeleteProgram(program);

        if (hasCS)
        {
            GLCall(glDeleteShader(cs));
        }

        if (hasVS)
        {
            GLCall(glDeleteShader(vs));
        }

        if (hasFS)
        {
            GLCall(glDeleteShader(fs));
        }

        if (hasGS)
        {
            GLCall(glDeleteShader(gs));
        }

        return true;
    }

    unsigned int ShaderCompiler:: CompileShaderShaderStage(unsigned int type, const std::string& source)
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
            else if (type == GL_COMPUTE_SHADER)
            {
                shaderTypeString = "compute";
            }
            std::cout << "Failed to compile " << shaderTypeString << "shader!" << std::endl;
            std::cout << message << std::endl;
            GLCall(glDeleteShader(id));

            return 0;
        }

        return id;
    }
}