#include <string>

#include "ShaderManager.h"

#include "Ganymede/Log/Log.h"
#include "Shader.h"

namespace Ganymede
{
	namespace ShaderManager_Private
	{
		const std::string locBaseShaderFolder("res/shaders/");
	}

	ShaderManager::~ShaderManager()
	{
		for (auto& it : m_LoadedShaders) {
			// Do stuff
			delete it.second;
		}

		m_LoadedShaders.clear();
	}

	Shader* ShaderManager::RegisterAndLoadShader(const char* aShaderName)
	{
		Shader* shader = GetShader(aShaderName);
		if (shader != nullptr)
		{
			return shader;
		}

		shader = new Shader(ShaderManager_Private::locBaseShaderFolder + std::string(aShaderName) + std::string(".shader"));
		m_LoadedShaders.insert(std::make_pair(aShaderName, shader));
		return shader;
	}

	void ShaderManager::UnloadAndRemoveShader(const char* aShaderName)
	{
		auto shader = m_LoadedShaders.find(aShaderName);
		if (shader == m_LoadedShaders.end())
		{
			return;
		}

		delete shader->second;
		m_LoadedShaders.erase(shader);
	}

	Shader* ShaderManager::GetShader(const char* aShaderName)
	{
		auto shader = m_LoadedShaders.find(aShaderName);
		if (shader == m_LoadedShaders.end())
		{
			return nullptr;
		}

		return shader->second;
	}

}