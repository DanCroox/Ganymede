#pragma once
#include "Ganymede/Core/Core.h"

#include "Ganymede/Graphics/ShaderBinary.h"
#include <string>
#include <vector>

namespace Ganymede
{
	class GANYMEDE_API ShaderCompiler
	{
	public:
		ShaderCompiler() = default;
		virtual ~ShaderCompiler() = default;

		bool Compile(const std::string& filePath, std::vector<ShaderBinary::Binary>& binaryDataOut);

	protected:
		struct ProgramSource
		{
#ifndef GM_RETAIL
			std::string m_Name;
#endif //GM_RETAIL
			std::string ComputeSource;
			std::string VertexSource;
			std::string FragmentSource;
			std::string GeometrySource;
		};

		virtual bool CompileProgram(const ProgramSource& programSource, std::vector<ShaderBinary::Binary>& binaryDataOut) = 0;

	private:
		void ParseIncludeHierarchy(const std::string& filepath, const std::string& line, std::stringstream& stringOut);
		ProgramSource ParseShader(const std::string& filepath);
	};
}