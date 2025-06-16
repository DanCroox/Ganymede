#pragma once
#include "Ganymede/Core/Core.h"

#include <string>
#include <vector>

namespace Ganymede
{
	class GANYMEDE_API ShaderBinary
	{
	public:
		struct Binary
		{
			std::vector<unsigned char> m_Data;
			unsigned int m_DataFormat;
		};
		using BinaryContainer = std::vector<Binary>;

		ShaderBinary(const std::string& filepath);
		const BinaryContainer& GetBinaryContainer() const { return m_BinaryContainer; }

		const std::string& GetFilePath() const { return m_FilePath; }

	private:
		enum class ShaderType : int
		{
			NONE = -1,
			COMPUTE,
			VERTEX,
			FRAGMENT,
			GEOMETRY,

			_COUNT
		};

		struct ProgramSource
		{
			std::string ComputeSource;
			std::string VertexSource;
			std::string FragmentSource;
			std::string GeometrySource;
		};
		
		ProgramSource ParseShader(const std::string& filepath);
		void ParseIncludeHierarchy(const std::string& filepath, const std::string& line, std::stringstream& stringOut);
		void CompileProgram(const ProgramSource& programSource);

		std::string m_FilePath;

		// Holds compiled shader binaries. It is a 2 dimensional container.
		// Can be used to store individual stages-binaries or entire program binaries (according to implementation).
		BinaryContainer m_BinaryContainer;
	};
}