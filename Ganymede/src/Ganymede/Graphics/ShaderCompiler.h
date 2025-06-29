#pragma once
#include "Ganymede/Core/Core.h"

#include "ShaderBinary.h"
#include <optional>
#include <string>
#include <vector>

namespace Ganymede
{
	class GANYMEDE_API ShaderCompiler
	{
	public:
		ShaderCompiler() = delete;
		static bool Compile(const std::string& filePath, std::vector<ShaderBinary::Binary>& binaryDataOut);

	private:
		struct ProgramSource
		{
			std::string ComputeSource;
			std::string VertexSource;
			std::string FragmentSource;
			std::string GeometrySource;
		};

		static ProgramSource ParseShader(const std::string& filepath);
		static void ParseIncludeHierarchy(const std::string& filepath, const std::string& line, std::stringstream& stringOut);
		static bool CompileProgram(const ProgramSource& programSource, std::vector<ShaderBinary::Binary>& binaryDataOut);
		static unsigned int CompileShaderShaderStage(unsigned int type, const std::string& source);
	};
}