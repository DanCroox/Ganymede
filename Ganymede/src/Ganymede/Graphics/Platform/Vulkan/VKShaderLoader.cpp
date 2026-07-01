#include "VKShaderLoader.h"

#include "Ganymede/Graphics/ShaderBinary.h"
#include "VKShaderCompiler.h"
#include <fstream>

namespace Ganymede
{
	std::optional<ShaderBinary> VKShaderLoader::Load(const std::string& filePath)
	{
		VKShaderCompiler compiler;
		ShaderBinary shaderBinary;
		shaderBinary.m_FilePath = filePath;

		if (compiler.Compile(filePath, shaderBinary.m_BinaryContainer))
		{
			return shaderBinary;
		}

		return std::nullopt;
	}
}