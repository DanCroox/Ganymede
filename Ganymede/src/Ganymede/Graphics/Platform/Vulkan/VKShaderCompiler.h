#pragma once
#include "Ganymede/Core/Core.h"
#include "Ganymede/Graphics/ShaderCompiler.h"

namespace Ganymede
{
	class GANYMEDE_API VKShaderCompiler : public ShaderCompiler
	{
	protected:
		bool CompileProgram(const ProgramSource& programSource, std::vector<ShaderBinary::Binary>& binaryDataOut) override;
	};
}