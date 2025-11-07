#pragma once
#include "Ganymede/Graphics/ComputeShader.h"

namespace Ganymede
{
	class RenderTarget;
	class ShaderBinary;

	class GANYMEDE_API OGLComputeShader : public ComputeShader
	{
	public:
		explicit OGLComputeShader(const ShaderBinary& shaderBinary);
		~OGLComputeShader() override;

		OGLComputeShader(OGLComputeShader&& other) noexcept;
		OGLComputeShader& operator=(OGLComputeShader&& other) noexcept;

		unsigned int GetRendererID() const { return m_RendererID; }

		bool IsValid() const override { return m_RendererID != 0; }

		void Bind() override;
		void Unbind() override;

		void Dispatch(unsigned int numWgX, unsigned int numWgY, unsigned int numWgZ) override;

	private:
		unsigned int m_RendererID;
	};
}