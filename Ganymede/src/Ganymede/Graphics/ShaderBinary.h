#pragma once
#include "Ganymede/Core/Core.h"

#include "Ganymede/Data/SerializerTraits.h"
#include <string>
#include <vector>

namespace Ganymede
{
	class ShaderLoader;

	class GANYMEDE_API ShaderBinary
	{
	public:
		enum class ShaderType : int
		{
			NONE = -1,
			COMPUTE,
			VERTEX,
			FRAGMENT,
			GEOMETRY,

			_COUNT
		};

		struct Binary
		{
			std::vector<unsigned char> m_Data;
			unsigned int m_DataFormat = 0;
			ShaderType m_BinaryType = ShaderType::NONE;
		};

		const std::vector<Binary>& GetBinaryContainer() const { return m_BinaryContainer; }
		const std::string& GetFilePath() const { return m_FilePath; }

	private:
		friend class ShaderLoader;

		GM_SERIALIZABLE(ShaderBinary);
		ShaderBinary() = default;

		std::string m_FilePath;

		// Holds compiled shader binaries. It is a 2 dimensional container.
		// Can be used to store individual stages-binaries or entire program binaries (according to implementation).
		std::vector<Binary> m_BinaryContainer;
	};
}