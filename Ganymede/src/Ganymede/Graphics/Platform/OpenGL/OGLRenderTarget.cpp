#include "Ganymede/Graphics/RenderTarget.h"

#include "OGLContext.h"
#include <GL/glew.h>

namespace Ganymede
{
	namespace RenderTarget_Private
	{

	}

	RenderTarget::RenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		m_ComponentType(componentType),
		m_ChannelDataType(dataType),
		m_ChannelPrecision(precision),
		m_Size(size)
	{
		GM_CORE_ASSERT(size.x > 0 && size.y > 0, "Size needs to be at least 1x1 pixels.");

		glGenTextures(1, &m_RenderID);
		GM_CORE_ASSERT(m_RenderID > 0, "Couldn't create texture.");
	}

	RenderTarget::~RenderTarget()
	{
		glDeleteTextures(1, &m_RenderID);
	}

	void RenderTarget::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, m_RenderID);
	}

	void RenderTarget::UnBind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void RenderTarget::SetParameter(RenderTargetTypes::ParameterKey key, RenderTargetTypes::ParameterValue value)
	{
		Bind();
		glTexParameteri(GL_TEXTURE_2D, OGLContext::ToNativeParameterKey(key), OGLContext::ToNativeParameterValue(value));
		UnBind();
	}

	SinglesampleRenderTarget::SinglesampleRenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		Super(componentType, dataType, precision, size)
	{
		Bind();
		glTexImage2D(GL_TEXTURE_2D, 0, OGLContext::ToNativeInternalFormat(componentType, dataType, precision), size.x, size.y, 0, OGLContext::ToNativeChannelCount(componentType), OGLContext::ToNativeDataType(dataType, precision), NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		UnBind();
	}

	void SinglesampleRenderTarget::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, m_RenderID);
	}

	void SinglesampleRenderTarget::UnBind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	MultisampleRenderTarget::MultisampleRenderTarget(unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		Super(componentType, dataType, precision, size),
		m_SampleCount(sampleCount)
	{
		GM_CORE_ASSERT(sampleCount >= 2 && sampleCount <= 16, "Number of samples needs to be between 2 and 16.");

		Bind();
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, OGLContext::ToNativeInternalFormat(componentType, dataType, precision), size.x, size.y, GL_TRUE);
		UnBind();
	}

	void MultisampleRenderTarget::Bind()
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_RenderID);
	}

	void MultisampleRenderTarget::UnBind()
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	}

	CubeMapArrayRenderTarget::CubeMapArrayRenderTarget(unsigned int numTextures, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
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

	void CubeMapArrayRenderTarget::Bind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_RenderID);
	}

	void CubeMapArrayRenderTarget::UnBind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
	}
}