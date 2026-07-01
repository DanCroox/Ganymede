#pragma once

#include "Shader.h"

#include "glm/glm.hpp"
#include <string>
#include <unordered_map>

namespace Ganymede
{
	class RenderTarget;
	class ShaderBinary;

	class GANYMEDE_API GraphicsShader : public Shader
	{
	public:
		~GraphicsShader() = default;

		virtual void BindTexture(RenderTarget& texture, uint32_t bindingPoint) = 0;

		virtual void SetUniform1f(uint32_t bindingPoint, const float value) const = 0;
		virtual void SetUniform1i(uint32_t bindingPoint, const int value) const = 0;
		virtual void SetUniform1iv(uint32_t bindingPoint, const int* value, unsigned int count = 1) const = 0;
		virtual void SetUniform2i(uint32_t bindingPoint, int v1, int v2) const = 0;
		virtual void SetUniform2iv(uint32_t bindingPoint, const int* values, unsigned int count = 1) const = 0;
		virtual void SetUniform1fv(uint32_t bindingPoint, const float* value, unsigned int count = 1) const = 0;
		virtual void SetUniform4f(uint32_t bindingPoint, float v0, float v1, float v2, float v3) const = 0;
		virtual void SetUniform3f(uint32_t bindingPoint, float value1, float value2, float value3) const = 0;
		virtual void SetUniform2f(uint32_t bindingPoint, float value1, float value2) const = 0;
		virtual void SetUniform3f(uint32_t bindingPoint, const glm::vec3& value) const = 0;
		virtual void SetUniform2f(uint32_t bindingPoint, const glm::vec2& value) const = 0;
		virtual void SetUniform3fv(uint32_t bindingPoint, const float* values, unsigned int count = 1) const = 0;
		virtual void SetUniformMat4f(uint32_t bindingPoint, const glm::mat4* matrix, unsigned int count = 1) const = 0;
		virtual void SetUniformMat4f(uint32_t bindingPoint, const glm::mat4& matrix) const = 0;

	protected:
		explicit GraphicsShader(const ShaderBinary& shaderBinary) : Shader(shaderBinary) {};

		GraphicsShader(GraphicsShader&& other) noexcept = default;
		GraphicsShader& operator=(GraphicsShader&& other) noexcept = default;
	};
}