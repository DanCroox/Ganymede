#pragma once
#include "Ganymede/Core/Core.h"
#include "Ganymede/Graphics/Shader.h"

namespace Ganymede
{
	class RenderTarget;
	class ShaderBinary;

	class GANYMEDE_API OGLShader : public Shader
	{
	public:
		explicit OGLShader(const ShaderBinary& shaderBinary);
		~OGLShader() override;

		OGLShader(OGLShader&& other) noexcept;
		OGLShader& operator=(OGLShader&& other) noexcept;

		unsigned int GetRendererID() const { return m_RendererID; }

		bool IsValid() const override { return m_RendererID != 0; }
		void BindTexture(RenderTarget& texture, const char* textureName) override;

		void SetUniform1f(const std::string& name, const float value) const override;
		void SetUniform1i(const std::string& name, const int value) const override;
		void SetUniform1iv(const std::string& name, const int* value, unsigned int count = 1) const override;
		void SetUniform2i(const std::string& name, int v1, int v2) const override;
		void SetUniform2iv(const std::string& name, const int* values, unsigned int count = 1) const override;
		void SetUniform1fv(const std::string& name, const float* value, unsigned int count = 1) const override;
		void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const override;
		void SetUniform3f(const std::string& name, float value1, float value2, float value3) const override;
		void SetUniform2f(const std::string& name, float value1, float value2) const override;
		void SetUniform3f(const std::string& name, const glm::vec3& value) const override;
		void SetUniform2f(const std::string& name, const glm::vec2& value) const override;
		void SetUniform3fv(const std::string& name, const float* values, unsigned int count = 1) const override;
		void SetUniformMat4f(const std::string& name, const glm::mat4* matrix, unsigned int count = 1) const override;
		void SetUniformMat4f(const std::string& name, const glm::mat4& matrix) const override;

	private:
		int GetUniformLocation(const std::string& name) const;

		unsigned int m_RendererID;
		mutable std::unordered_map<std::string, int> m_UniformLocationCache;
		std::unordered_map<std::string, int> m_ShaderTextureSlots;
		unsigned int m_NextAvailableTextureSlot = 0;
	};
}