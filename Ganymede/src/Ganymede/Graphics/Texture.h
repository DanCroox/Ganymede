#pragma once
#include "Ganymede/Core/Core.h"

#include <string>

namespace Ganymede
{
	class GANYMEDE_API Texture
	{
	public:
		Texture(const std::string& path);
		Texture(unsigned char* data, int width, int height, unsigned char channelCount, unsigned int bitDepth);

		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }
		inline unsigned int GetNumChannels() const { return m_ChannelCount; }
		inline unsigned int GetBitDepth() const { return m_BitDepth; }

		const std::vector<unsigned char>& GetBytes() const { return m_Bytes; }

	private:
		int m_Width;
		int m_Height;
		int m_ChannelCount;
		unsigned int m_BitDepth;

		std::vector<unsigned char> m_Bytes;
	};
}