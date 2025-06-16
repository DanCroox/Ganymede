#pragma once

#include "Ganymede/Core/Core.h"

#include "glm/glm.hpp"
#include <string>
#include <unordered_map>

namespace Ganymede
{
	class RenderTarget;
	class ShaderBinary;

	class GANYMEDE_API Shader
	{
	public:
		Shader(const ShaderBinary& shaderBinary);
		~Shader();

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		Shader(Shader&& other) noexcept : 
			m_RendererID(other.m_RendererID),
			m_UniformLocationCache(other.m_UniformLocationCache),
			m_ShaderTextureSlots(other.m_ShaderTextureSlots),
			m_NextAvailableTextureSlot(other.m_NextAvailableTextureSlot)
		{
			other.m_RendererID = 0;
		}

		Shader& operator=(Shader&& other) noexcept
		{
			if (this != &other)
			{
				m_RendererID = other.m_RendererID;
				m_UniformLocationCache = other.m_UniformLocationCache;
				m_ShaderTextureSlots = other.m_ShaderTextureSlots;
				m_NextAvailableTextureSlot = other.m_NextAvailableTextureSlot;

				other.m_RendererID = 0;
			}

			return *this;
		}

		void BindTexture(RenderTarget& texture, const char* textureName);

		unsigned int GetRendererID() const { return m_RendererID; }
		inline bool IsValid() const { return m_RendererID != 0; }

		// Set Uniforms
		void SetUniform1f(const std::string& name, const float value) const;
		void SetUniform1i(const std::string& name, const int value) const;
		void SetUniform1iv(const std::string& name, const int* value, unsigned int count = 1) const;
		void SetUniform2i(const std::string& name, int v1, int v2) const;
		void SetUniform2iv(const std::string& name, const int* values, unsigned int count = 1) const;
		void SetUniform1fv(const std::string& name, const float* value, unsigned int count = 1) const;
		void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const;
		void SetUniform3f(const std::string& name, float value1, float value2, float value3) const;
		void SetUniform2f(const std::string& name, float value1, float value2) const;
		void SetUniform3f(const std::string& name, const glm::vec3& value) const;
		void SetUniform2f(const std::string& name, const glm::vec2& value) const;
		void SetUniform3fv(const std::string& name, const float* values, unsigned int count = 1) const;
		void SetUniformMat4f(const std::string& name, const glm::mat4* matrix, unsigned int count = 1) const;
		void SetUniformMat4f(const std::string& name, const glm::mat4& matrix) const;

	private:
		int GetUniformLocation(const std::string& name) const;

		mutable std::unordered_map<std::string, int> m_UniformLocationCache;
		std::unordered_map<std::string, int> m_ShaderTextureSlots;
		unsigned int m_NextAvailableTextureSlot = 0;
		unsigned int m_RendererID;
	};
}