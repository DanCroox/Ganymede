#include "OGLRenderTarget.h"

#include "OGLContext.h"
#include <GL/glew.h>

namespace Ganymede
{
	OGLRenderTarget::OGLRenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		RenderTarget(componentType, dataType, precision, size)
	{
		GM_CORE_ASSERT(size.x > 0 && size.y > 0, "Size needs to be at least 1x1 pixels.");

		glGenTextures(1, &m_RenderID);
		GM_CORE_ASSERT(m_RenderID > 0, "Couldn't create texture.");
	}

	OGLRenderTarget::~OGLRenderTarget()
	{
		glDeleteTextures(1, &m_RenderID);
	}

	OGLRenderTarget::OGLRenderTarget(OGLRenderTarget&& other) noexcept
		: RenderTarget(std::move(other)),
		m_RenderID(other.m_RenderID)
	{
		other.m_RenderID = 0;
	}

	OGLRenderTarget& OGLRenderTarget::operator=(OGLRenderTarget&& other) noexcept
	{
		if (this != &other)
		{
			RenderTarget::operator=(std::move(other));
			m_RenderID = other.m_RenderID;

			other.m_RenderID = 0;
		}

		return *this;
	}

	void OGLRenderTarget::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, m_RenderID);
	}

	void OGLRenderTarget::UnBind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OGLRenderTarget::SetParameter(RenderTargetTypes::ParameterKey key, RenderTargetTypes::ParameterValue value)
	{
		Bind();
		glTexParameteri(GL_TEXTURE_2D, OGLContext::ToNativeParameterKey(key), OGLContext::ToNativeParameterValue(value));
		UnBind();
	}

	OGLSinglesampleRenderTarget::OGLSinglesampleRenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		Super(componentType, dataType, precision, size)
	{
		Bind();
		glTexImage2D(GL_TEXTURE_2D, 0, OGLContext::ToNativeInternalFormat(componentType, dataType, precision), size.x, size.y, 0, OGLContext::ToNativeChannelCount(componentType), OGLContext::ToNativeDataType(dataType, precision), NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		UnBind();
	}

	void OGLSinglesampleRenderTarget::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, m_RenderID);
	}

	void OGLSinglesampleRenderTarget::UnBind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	OGLMultisampleRenderTarget::OGLMultisampleRenderTarget(unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		Super(componentType, dataType, precision, size),
		m_SampleCount(sampleCount)
	{
		GM_CORE_ASSERT(sampleCount >= 2 && sampleCount <= 16, "Number of samples needs to be between 2 and 16.");

		Bind();
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, OGLContext::ToNativeInternalFormat(componentType, dataType, precision), size.x, size.y, GL_TRUE);
		UnBind();
	}

	void OGLMultisampleRenderTarget::Bind()
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_RenderID);
	}

	void OGLMultisampleRenderTarget::UnBind()
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	}

	OGLCubeMapArrayRenderTarget::OGLCubeMapArrayRenderTarget(unsigned int numTextures, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		Super(componentType, dataType, precision, size)
	{
		Bind();
		glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, OGLContext::ToNativeInternalFormat(componentType, dataType, precision), size.x, size.y, numTextures, 0, OGLContext::ToNativeChannelCount(componentType), OGLContext::ToNativeDataType(dataType, precision), NULL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		UnBind();
	}

	void OGLCubeMapArrayRenderTarget::Bind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_RenderID);
	}

	void OGLCubeMapArrayRenderTarget::UnBind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
	}
}