#include "OGLShaderCompiler.h"

#include "GL/glew.h"

namespace Ganymede
{
	bool OGLShaderCompiler::CompileProgram(const ProgramSource& programSource, std::vector<ShaderBinary::Binary>& binaryDataOut)
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

		uint8_t shaderTypeBits = 0;

		if (hasCS)
		{
			shaderTypeBits |= ShaderBinaryTypeBits::COMPUTE;
			cs = CompileShaderShaderStage(GL_COMPUTE_SHADER, programSource.ComputeSource);
			glAttachShader(program, cs);
		}

		if (hasVS)
		{
			shaderTypeBits |= ShaderBinaryTypeBits::VERTEX;
			vs = CompileShaderShaderStage(GL_VERTEX_SHADER, programSource.VertexSource);
			glAttachShader(program, vs);
		}

		if (hasFS)
		{
			shaderTypeBits |= ShaderBinaryTypeBits::FRAGMENT;
			fs = CompileShaderShaderStage(GL_FRAGMENT_SHADER, programSource.FragmentSource);
			glAttachShader(program, fs);
		}

		if (hasGS)
		{
			shaderTypeBits |= ShaderBinaryTypeBits::GEOMETRY;
			gs = CompileShaderShaderStage(GL_GEOMETRY_SHADER, programSource.GeometrySource);
			glAttachShader(program, gs);
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
		binary.m_ShaderTypeBits = shaderTypeBits;
		binary.m_Data.resize(binaryLength);

		// Read binary and format
		glGetProgramBinary(program,
			binaryLength,
			&binaryLength,
			&binary.m_DataFormat,
			binary.m_Data.data());
		glDeleteProgram(program);

		if (hasCS)
		{
			glDeleteShader(cs);
		}

		if (hasVS)
		{
			glDeleteShader(vs);
		}

		if (hasFS)
		{
			glDeleteShader(fs);
		}

		if (hasGS)
		{
			glDeleteShader(gs);
		}

		return true;
	}

	unsigned int OGLShaderCompiler:: CompileShaderShaderStage(unsigned int type, const std::string& source)
	{
		unsigned int id = glCreateShader(type);
		const char* src = source.c_str();
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		int result;
		glGetShaderiv(id, GL_COMPILE_STATUS, &result);
		if (result == GL_FALSE)
		{
			int length;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
			char* message = (char*)alloca(length * sizeof(char));
			glGetShaderInfoLog(id, length, &length, message);

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
			glDeleteShader(id);

			return 0;
		}

		return id;
	}
}