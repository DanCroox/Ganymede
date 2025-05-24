#include "Texture.h"
#include "GL/glew.h"
#include "stb_image.h"

namespace Ganymede
{

	Texture::Texture(const std::string& path)
		: m_FilePath(path),
		m_Width(0),
		m_Height(0),
		m_ChannelCount(0)
	{
		m_FilePath = path;
		// flip texture cause in opengl 0 is at bottom while in texture is top
		stbi_set_flip_vertically_on_load(1);
		// 4 == rgba channels to load
		unsigned char* buffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_ChannelCount, 4);

		GM_CORE_ASSERT(buffer != nullptr, "File broken or not existent.");

		PushTextureToGPU(buffer);

		if (buffer)
		{
			stbi_image_free(buffer);
		}
	}

	Texture::Texture(unsigned char* data, int width, int height, unsigned char channelCount)
	{
		m_Width = width;
		m_Height = height;
		m_ChannelCount = channelCount;
		PushTextureToGPU(data);
	}
	Texture::~Texture()
	{
		GLCall(glDeleteTextures(1, &m_RendererID));
	}

	void Texture::Bind(unsigned int slot) const
	{
		GLCall(glActiveTexture(GL_TEXTURE0 + slot));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
	}

	void Texture::Unbind() const
	{
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));
	}

	void Texture::PushTextureToGPU(unsigned char* data)
	{
		GLCall(glGenTextures(1, &m_RendererID));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));

		// GL_RGBA -> supplied format like texture data has 4 channels
		// GL_RGBA8 -> how to deal supplied data internally
		int internalType = GL_RGBA8;
		int type = GL_RGBA;
		/*
		if (m_ChannelCount == 4)
		{
			internalType = GL_RGBA8;
			type = GL_RGBA;
		}
		else if (m_ChannelCount == 3)
		{
			internalType = GL_RGB8;
			type = GL_RGB;
		}
		else if (m_ChannelCount == 2)
		{
			internalType = GL_RG8;
			type = GL_RG;
		}
		else if (m_ChannelCount == 1)
		{
			internalType = GL_R8;
			type = GL_R;
		}
		else
		{
			ASSERT(false);
		}
		*/


		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, internalType, m_Width, m_Height, 0, type, GL_UNSIGNED_BYTE, data));

		// Enable mipmapping
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));
		// Lod bias for higher mip map resolution all over
		GLCall(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)); // horizontal
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)); // vertical

		GLCall(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8.f));

		GLCall(glBindTexture(GL_TEXTURE_2D, 0));
	}
}