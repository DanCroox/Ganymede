#include "OGLShaderLoader.h"

#include "Ganymede/Graphics/ShaderBinary.h"
#include "Ganymede/Graphics/ShaderCompiler.h"

namespace Ganymede
{
	std::optional<ShaderBinary> OGLShaderLoader::Load(const std::string& filePath)
	{
		// TODO: Here we need to decide how to treat the file. We can treat it as a source and compile it or assume its a prebuild binary and load accordingly.
		ShaderBinary shaderBinary;
		const bool compilationSuccess = ShaderCompiler::Compile(filePath, shaderBinary.m_BinaryContainer);
		if (compilationSuccess)
		{
			shaderBinary.m_FilePath = filePath;
			return shaderBinary;
		}

		return std::nullopt;
	}
}