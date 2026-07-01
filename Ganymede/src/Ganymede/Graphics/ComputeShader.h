#pragma once

#include "Shader.h"

namespace Ganymede
{
	class ShaderBinary;

	class GANYMEDE_API ComputeShader : public Shader
	{
	public:
		virtual void Dispatch(unsigned int numWgX, unsigned int numWgY, unsigned int numWgZ) = 0;

	protected:
		explicit ComputeShader(const ShaderBinary& shaderBinary) : Shader(shaderBinary) {};

		ComputeShader(ComputeShader&& other) noexcept = default;
		ComputeShader& operator=(ComputeShader&& other) noexcept = default;
	};
}