#pragma once
#include "Ganymede/Core/Core.h"

#include <optional>
#include <string>

namespace Ganymede
{
	class ShaderBinary;

	class GANYMEDE_API ShaderLoader
	{
	public:
		static std::optional<ShaderBinary> Load(const std::string& filePath);
	};
}