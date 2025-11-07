#pragma once
#include "Ganymede/Core/Core.h"

#include "Ganymede/Data/SerializerTraits.h"
#include "Platform/GraphicsFactory.h"
#include <string>
#include <vector>

namespace Ganymede
{
	namespace ShaderBinaryTypeBits
	{
		constexpr uint8_t COMPUTE = 1 << 0;
		constexpr uint8_t VERTEX = 1 << 1;
		constexpr uint8_t FRAGMENT = 1 << 2;
		constexpr uint8_t GEOMETRY = 1 << 3;
	}

	class GANYMEDE_API ShaderBinary
	{
	public:
		struct Binary
		{
			std::vector<unsigned char> m_Data;
			unsigned int m_DataFormat = 0;
			uint8_t m_ShaderTypeBits = 0;
		};

		GM_SERIALIZABLE(ShaderBinary);

		ShaderBinary() = default;

		std::string m_FilePath;

		// Holds compiled shader binaries. It is a 2 dimensional container.
		// Can be used to store individual stages-binaries or entire program binaries (according to implementation).
		std::vector<Binary> m_BinaryContainer;
	};
}