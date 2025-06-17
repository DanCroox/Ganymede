#pragma once
#include "Ganymede/Core/Core.h"
#include "Ganymede/Data/Handle.h"

namespace Ganymede
{
	class Texture;

	class GANYMEDE_API GPUTexture
	{
	public:
		GPUTexture(const Texture& textureHandle);
		~GPUTexture();

		GPUTexture(const GPUTexture&) = delete;
		GPUTexture& operator=(const GPUTexture&) = delete;

		GPUTexture& operator=(GPUTexture&& other) noexcept;
		GPUTexture(GPUTexture&& other) noexcept;

		void Bind(unsigned int slot = 0) const;
		void Unbind() const;

	private:
		unsigned int m_Width;
		unsigned int m_Height;
		unsigned int m_ChannelCount;
		unsigned int m_BitDepth;
		unsigned int m_RendererID;
	};
}