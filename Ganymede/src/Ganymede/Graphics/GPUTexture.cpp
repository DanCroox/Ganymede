#include "GPUTexture.h"

#include "Texture.h"
#include "GL/glew.h"

namespace Ganymede
{
	GPUTexture::GPUTexture(const Texture& texture)
	{
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);

		int internalType = 0;
		int type = 0;
		if (texture.GetNumChannels() == 3)
		{
			internalType = GL_RGB8;
			type = GL_RGB;
		}
		else if (texture.GetNumChannels() == 4)
		{
			internalType = GL_RGBA8;
			type = GL_RGBA;
		}
		else
		{
			GM_CORE_ASSERT(false, "Unsupported number of channels in texture");
		}
		
		unsigned int dataFormat = 0;
		const unsigned int bitDepth = texture.GetBitDepth();
		if (bitDepth == 16)
		{
			dataFormat = GL_UNSIGNED_SHORT;
		}
		else if (bitDepth == 8)
		{
			dataFormat = GL_UNSIGNED_BYTE;
		}
		else
		{
			GM_CORE_ASSERT(false, "Unsupported number of bits per channel in texture");
		}

		glTexImage2D(GL_TEXTURE_2D, 0, internalType, texture.GetWidth(), texture.GetHeight(), 0, type, dataFormat, texture.GetBytes().data());

		// Enable mipmapping
		glGenerateMipmap(GL_TEXTURE_2D);

		// Lod bias for higher mip map resolution all over
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Anisotropy filtering
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8.f);

		Unbind();
	}

	GPUTexture::~GPUTexture()
	{
		GLCall(glDeleteTextures(1, &m_RendererID));
	}

	GPUTexture::GPUTexture(GPUTexture&& other) noexcept
		: m_RendererID(other.m_RendererID)
	{
		other.m_RendererID = 0;
	}

	GPUTexture& GPUTexture::operator=(GPUTexture&& other) noexcept
	{
		if (this != &other)
		{
			m_RendererID = other.m_RendererID;
			other.m_RendererID = 0;
		}

		return *this;
	}

	void GPUTexture::Bind(unsigned int slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

	void GPUTexture::Unbind() const
	{

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}