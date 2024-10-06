#pragma once
#include "Ganymede/Core/Core.h"

#include <unordered_map>

namespace Ganymede
{
	class Shader;

	class GANYMEDE_API ShaderManager
	{
	public:
		~ShaderManager();

		Shader* RegisterAndLoadShader(const char* aShaderName);
		void UnloadAndRemoveShader(const char* aShaderName);
		Shader* GetShader(const char* aShaderName);

	private:
		std::unordered_map<std::string, Shader*> m_LoadedShaders;
	};
}