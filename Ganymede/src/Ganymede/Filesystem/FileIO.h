#pragma once
#include "Ganymede/Core/Core.h"

#include <optional>

namespace Ganymede
{
	namespace FileIO
	{
		GANYMEDE_API bool WriteBytesToFile(const std::string& filename, const std::vector<uint8_t>& data);
		GANYMEDE_API std::optional<std::vector<uint8_t>> ReadBytesFromFile(const std::string& filename);
	}
}