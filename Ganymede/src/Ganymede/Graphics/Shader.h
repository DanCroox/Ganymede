#pragma once

#include "Ganymede/Core/Core.h"

#include <string>

namespace Ganymede
{
	class ShaderBinary;

	class GANYMEDE_API Shader
	{
	public:
		virtual ~Shader() = default;

		virtual bool IsValid() const = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

	protected:
		Shader() = delete;
		explicit Shader(const ShaderBinary& shaderBinary) {};

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		Shader(Shader&& other) noexcept = default;
		Shader& operator=(Shader&& other) noexcept = default;
	};
}