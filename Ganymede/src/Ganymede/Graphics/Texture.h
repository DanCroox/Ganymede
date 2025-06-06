#pragma once
#include "Ganymede/Core/Core.h"

#include <string>

namespace Ganymede
{
	class GANYMEDE_API Texture
	{
	public:
		Texture(const std::string& path);
		Texture(unsigned char* data, int width, int height, unsigned char channelCount, size_t textureID);
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		Texture& operator=(Texture&& other) noexcept;
		Texture(Texture&& other) noexcept;

		void Bind(unsigned int slot = 0) const;
		void Unbind() const;

		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }
	private:
		void PushTextureToGPU(unsigned char* data);
		unsigned int m_RendererID;

		std::string m_FilePath;
		int m_Width, m_Height, m_ChannelCount;
		size_t m_TextureID;
	};
}