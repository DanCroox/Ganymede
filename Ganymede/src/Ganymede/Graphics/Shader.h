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
		virtual ~Shader() = default;

		virtual bool IsValid() const = 0;
		virtual void BindTexture(RenderTarget& texture, const char* textureName) = 0;

		virtual void SetUniform1f(const std::string& name, const float value) const = 0;
		virtual void SetUniform1i(const std::string& name, const int value) const = 0;
		virtual void SetUniform1iv(const std::string& name, const int* value, unsigned int count = 1) const = 0;
		virtual void SetUniform2i(const std::string& name, int v1, int v2) const = 0;
		virtual void SetUniform2iv(const std::string& name, const int* values, unsigned int count = 1) const = 0;
		virtual void SetUniform1fv(const std::string& name, const float* value, unsigned int count = 1) const = 0;
		virtual void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const = 0;
		virtual void SetUniform3f(const std::string& name, float value1, float value2, float value3) const = 0;
		virtual void SetUniform2f(const std::string& name, float value1, float value2) const = 0;
		virtual void SetUniform3f(const std::string& name, const glm::vec3& value) const = 0;
		virtual void SetUniform2f(const std::string& name, const glm::vec2& value) const = 0;
		virtual void SetUniform3fv(const std::string& name, const float* values, unsigned int count = 1) const = 0;
		virtual void SetUniformMat4f(const std::string& name, const glm::mat4* matrix, unsigned int count = 1) const = 0;
		virtual void SetUniformMat4f(const std::string& name, const glm::mat4& matrix) const = 0;

	protected:
		Shader() = delete;
		explicit Shader(const ShaderBinary& shaderBinary) {};

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		Shader(Shader&& other) noexcept = default;
		Shader& operator=(Shader&& other) noexcept = default;
	};
}