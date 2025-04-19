#pragma once

#include "Ganymede/Core/Core.h"

#include <string>
#include <unordered_map>

#include "glm/glm.hpp"

namespace Ganymede
{
	class SSBO;
	class RenderTarget;

	struct GANYMEDE_API ShaderProgramSource
	{
		std::string VertexSource;
		std::string FragmentSource;
		std::string GeometrySource;
	};

	class GANYMEDE_API Shader
	{
	private:
		std::string m_FilePath;
		unsigned int m_RendererID;
		mutable std::unordered_map<std::string, int> m_UniformLocationCache;
	public:
		Shader(const std::string& filepath);
		~Shader();

		void Bind() const;
		void Unbind() const;

		void BindUBOBlock(const SSBO& ubo, const std::string& uniformBlockName) const;

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
		ShaderProgramSource ParseShader(const std::string& filepath);
		void ParseIncludeHierarchy(const std::string& filepath, const std::string& line, std::stringstream& stringOut);
		unsigned int CompileShader(unsigned int type, const std::string& source);
		unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader);

		std::unordered_map<std::string, int> m_ShaderTextureSlots;
		unsigned int m_NextAvailableTextureSlot = 0;
	};
}