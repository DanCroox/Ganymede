#include "OGLShaderLoader.h"

#include "Ganymede/Graphics/ShaderBinary.h"
#include "OGLShaderCompiler.h"

namespace Ganymede
{
	std::optional<ShaderBinary> OGLShaderLoader::Load(const std::string& filePath)
	{
		OGLShaderCompiler compiler;
		ShaderBinary shaderBinary;
		if (compiler.Compile(filePath, shaderBinary.m_BinaryContainer))
		{
			shaderBinary.m_FilePath = filePath;
			return shaderBinary;
		}

		return std::nullopt;
	}
}