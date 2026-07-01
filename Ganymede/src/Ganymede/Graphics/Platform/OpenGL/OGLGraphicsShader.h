#pragma once
#include "Ganymede/Graphics/GraphicsShader.h"

namespace Ganymede
{
	class RenderTarget;
	class ShaderBinary;

	class GANYMEDE_API OGLGraphicsShader : public GraphicsShader
	{
	public:
		explicit OGLGraphicsShader(const ShaderBinary& shaderBinary);
		~OGLGraphicsShader() override;

		OGLGraphicsShader(OGLGraphicsShader&& other) noexcept;
		OGLGraphicsShader& operator=(OGLGraphicsShader&& other) noexcept;

		unsigned int GetRendererID() const { return m_RendererID; }

		bool IsValid() const override { return m_RendererID != 0; }

		void Bind() override;
		void Unbind() override;

		void BindTexture(RenderTarget& texture, uint32_t bindingPoint) override;

		void SetUniform1f(uint32_t bindingPoint, const float value) const override;
		void SetUniform1i(uint32_t bindingPoint, const int value) const override;
		void SetUniform1iv(uint32_t bindingPoint, const int* value, unsigned int count = 1) const override;
		void SetUniform2i(uint32_t bindingPoint, int v1, int v2) const override;
		void SetUniform2iv(uint32_t bindingPoint, const int* values, unsigned int count = 1) const override;
		void SetUniform1fv(uint32_t bindingPoint, const float* value, unsigned int count = 1) const override;
		void SetUniform4f(uint32_t bindingPoint, float v0, float v1, float v2, float v3) const override;
		void SetUniform3f(uint32_t bindingPoint, float value1, float value2, float value3) const override;
		void SetUniform2f(uint32_t bindingPoint, float value1, float value2) const override;
		void SetUniform3f(uint32_t bindingPoint, const glm::vec3& value) const override;
		void SetUniform2f(uint32_t bindingPoint, const glm::vec2& value) const override;
		void SetUniform3fv(uint32_t bindingPoint, const float* values, unsigned int count = 1) const override;
		void SetUniformMat4f(uint32_t bindingPoint, const glm::mat4* matrix, unsigned int count = 1) const override;
		void SetUniformMat4f(uint32_t bindingPoint, const glm::mat4& matrix) const override;

	private:
		unsigned int m_RendererID;
		std::unordered_map<uint32_t, int> m_ShaderTextureSlots;
		unsigned int m_NextAvailableTextureSlot = 0;
	};
}