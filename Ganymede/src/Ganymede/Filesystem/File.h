#pragma once
#include "Ganymede/Core/Core.h"

#include <optional>

namespace Ganymede
{
	namespace File
	{
		GANYMEDE_API bool WriteBytesToFile(const std::string& filename, const std::vector<uint8_t>& data);
		GANYMEDE_API std::optional<std::vector<uint8_t>> ReadBytesFromFile(const std::string& filename);
		GANYMEDE_API bool EndsWith(std::string_view str, std::string_view suffix);
	}
}