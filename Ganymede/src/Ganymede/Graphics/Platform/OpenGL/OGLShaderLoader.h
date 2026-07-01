#pragma once
#include "Ganymede/Core/Core.h"

#include <optional>
#include <string>

namespace Ganymede
{
	class ShaderBinary;

	namespace OGLShaderLoader
	{
		std::optional<ShaderBinary> Load(const std::string& filePath);
	};
}