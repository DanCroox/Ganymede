#include "Texture.h"

#include "stb_image.h"

namespace Ganymede
{
	Texture::Texture(const std::string& path)
	{
		unsigned char* buffer = nullptr;

		if (stbi_is_16_bit(path.c_str()))
		{
			// If loading 16bit failes, it is probably 8bit (or 32bit - but we don't support it for now)
			// Seems in stb there is no better way to check if it is a 16bit or 8bit texture
			if (const unsigned short* buffer16 = stbi_load_16(path.c_str(), reinterpret_cast<int*>(&m_Width), reinterpret_cast<int*>(&m_Height), reinterpret_cast<int*>(&m_ChannelCount), 0))
			{
				buffer = (unsigned char*)buffer16;
				m_BitDepth = 16;
			}
		}
		else
		{
			buffer = stbi_load(path.c_str(), reinterpret_cast<int*>(&m_Width), reinterpret_cast<int*>(&m_Height), reinterpret_cast<int*>(&m_ChannelCount), 0);
			m_BitDepth = 8;
		}

		if (buffer == nullptr)
		{
			GM_CORE_ASSERT(false, "Couldn't load texture");
			return;
		}

		if (m_BitDepth != 16 && m_BitDepth != 8)
		{
			GM_CORE_ASSERT(false, "Invalid bit depth");
			return;
		}

		// No matter how many channels the texture really has - Here we can define how many channels we would like to output.
		const unsigned int bytesPerTexel = (m_BitDepth / 8);
		const unsigned int charBufferSize = m_Width * m_Height * m_ChannelCount * bytesPerTexel;
		m_Bytes.resize(charBufferSize);
		memcpy(m_Bytes.data(), buffer, charBufferSize);
		stbi_image_free(buffer);
	}

	Texture::Texture(unsigned char* data, int width, int height, unsigned char channelCount, unsigned int bitDepth) :
		m_Width(width),
		m_Height(height),
		m_ChannelCount(channelCount),
		m_BitDepth(bitDepth)
	{
		if (m_BitDepth != 16 && m_BitDepth != 8)
		{
			GM_CORE_ASSERT(false, "Invalid bit depth");
			return;
		}

		const unsigned int bytesPerTexel = (m_BitDepth / 8);
		const unsigned int charBufferSize = m_Width * m_Height * m_ChannelCount * bytesPerTexel;
		m_Bytes.resize(charBufferSize);

		memcpy(m_Bytes.data(), data, charBufferSize);
	}
}