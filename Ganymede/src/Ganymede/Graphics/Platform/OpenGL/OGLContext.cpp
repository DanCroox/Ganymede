#include "OGLContext.h"

#include "Ganymede/Graphics/FrameBuffer.h"
#include "Ganymede/Graphics/RenderTarget.h"
#include "Ganymede/Graphics/SSBO.h"
#include "Ganymede/Graphics/VertexObject.h"
#include <GL/glew.h>

namespace Ganymede
{
	unsigned int OGLContext::m_BoundFrameBuffer = 0;
	unsigned int OGLContext::m_BoundShader = 0;
	unsigned int OGLContext::m_BoundVertexArrayObject = 0;
	unsigned int OGLContext::m_BoundIndirectDrawBuffer = 0;
	glm::u32vec2 OGLContext::m_CurrentViewportDimension = {0, 0};
	
	void OGLContext::BindFrameBuffer(const FrameBuffer& frameBuffer)
	{
		const unsigned int renderID = frameBuffer.GetRenderID();
		if (renderID == m_BoundFrameBuffer)
		{
			return;
		}
	
		glBindFramebuffer(GL_FRAMEBUFFER, renderID);

		const glm::u32vec2 framebufferViewportDimension= frameBuffer.GetRenderDimension();
		if (m_CurrentViewportDimension != framebufferViewportDimension)
		{
			glViewport(0,0, framebufferViewportDimension.x, framebufferViewportDimension.y);
			m_CurrentViewportDimension = framebufferViewportDimension;
		}
		m_BoundFrameBuffer = renderID;
	}

	void OGLContext::UnbindFrameBuffer()
	{
		if (m_BoundFrameBuffer == 0)
		{
			return;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_BoundFrameBuffer = 0;
	}

	void OGLContext::BindShader(const Shader& shader)
	{
		const unsigned int rendererID = shader.GetRendererID();
		if (rendererID == m_BoundShader)
		{
			return;
		}

		glUseProgram(rendererID);
		m_BoundShader = rendererID;
	}

	void OGLContext::BindVertexArrayObject(const VertexObject& vo)
	{
		const unsigned int rendererID = vo.GetRenderID();
		if (rendererID == m_BoundVertexArrayObject)
		{
			return;
		}

		glBindVertexArray(rendererID);
		m_BoundVertexArrayObject = rendererID;
	}

	void OGLContext::BindIndirectDrawBuffer(SSBO& buffer)
	{
		const unsigned int bufferRenderID = buffer.m_RenderID;

		if (bufferRenderID == m_BoundIndirectDrawBuffer)
		{
			return;
		}

		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, bufferRenderID);
		m_BoundIndirectDrawBuffer = bufferRenderID;
	}

	void OGLContext::UnbindVertexArrayObject()
	{
		if (m_BoundVertexArrayObject == 0)
		{
			return;
		}

		glBindVertexArray(0);
		m_BoundVertexArrayObject = 0;
	}

	int OGLContext::ToNativeInternalFormat(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision)
	{
		GM_CORE_ASSERT(!(dataType == RenderTargetTypes::ChannelDataType::Float && precision == RenderTargetTypes::ChannelPrecision::B8), "8 bit float per channel is not allowed.");
		GM_CORE_ASSERT(!(dataType == RenderTargetTypes::ChannelDataType::UInt && precision == RenderTargetTypes::ChannelPrecision::B32), "32 bit integer per channel is not allowed.");

		if (componentType == RenderTargetTypes::ComponentType::RGBA)
		{
			switch (precision)
			{
			case RenderTargetTypes::ChannelPrecision::B8: return GL_RGBA8;
			case RenderTargetTypes::ChannelPrecision::B16: return dataType == RenderTargetTypes::ChannelDataType::Float ? GL_RGBA16F : GL_RGBA16;
			case RenderTargetTypes::ChannelPrecision::B32: return GL_RGBA32F;
			default:
				GM_CORE_ASSERT(false, "Unsupported channel precision.");
				break;
			}
		}
		else if (componentType == RenderTargetTypes::ComponentType::RGB)
		{
			switch (precision)
			{
			case RenderTargetTypes::ChannelPrecision::B8: return GL_RGB8;
			case RenderTargetTypes::ChannelPrecision::B16: return dataType == RenderTargetTypes::ChannelDataType::Float ? GL_RGB16F : GL_RGB16;
			case RenderTargetTypes::ChannelPrecision::B32: return GL_RGB32F;
			default:
				GM_CORE_ASSERT(false, "Unsupported channel precision.");
				break;
			}
		}
		else if (componentType == RenderTargetTypes::ComponentType::RG)
		{
			switch (precision)
			{
			case RenderTargetTypes::ChannelPrecision::B8: return GL_RG8;
			case RenderTargetTypes::ChannelPrecision::B16: return dataType == RenderTargetTypes::ChannelDataType::Float ? GL_RG16F : GL_RG16;
			case RenderTargetTypes::ChannelPrecision::B32: return GL_RG32F;
			default:
				GM_CORE_ASSERT(false, "Unsupported channel precision.");
				break;
			}
		}
		else if (componentType == RenderTargetTypes::ComponentType::R)
		{
			switch (precision)
			{
			case RenderTargetTypes::ChannelPrecision::B8: return GL_R8;
			case RenderTargetTypes::ChannelPrecision::B16: return dataType == RenderTargetTypes::ChannelDataType::Float ? GL_R16F : GL_R16;
			case RenderTargetTypes::ChannelPrecision::B32: return GL_R32F;
			default:
				GM_CORE_ASSERT(false, "Unsupported channel precision.");
				break;
			}
		}
		else if (componentType == RenderTargetTypes::ComponentType::Depth)
		{
			GM_CORE_ASSERT(!(precision == RenderTargetTypes::ChannelPrecision::B8), "8 bit depth component are not allowed.");
			GM_CORE_ASSERT(!(precision == RenderTargetTypes::ChannelPrecision::B16 && dataType == RenderTargetTypes::ChannelDataType::Float), "16 bit float depth component are not allowed.");
			switch (precision)
			{
			case RenderTargetTypes::ChannelPrecision::B16: return GL_DEPTH_COMPONENT16;
			case RenderTargetTypes::ChannelPrecision::B32: return dataType == RenderTargetTypes::ChannelDataType::Float ? GL_DEPTH_COMPONENT32F : GL_DEPTH_COMPONENT32;
			default:
				GM_CORE_ASSERT(false, "Unsupported channel precision.");
				break;
			}
		}

		GM_CORE_ASSERT(false, "Unsupported component format.");
		return -1;
	}

	int OGLContext::ToNativeDataType(RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision)
	{
		if (dataType == RenderTargetTypes::ChannelDataType::Float)
		{
			return GL_FLOAT;
		}
		else if (dataType == RenderTargetTypes::ChannelDataType::UInt)
		{
			switch (precision)
			{
			case RenderTargetTypes::ChannelPrecision::B8: return GL_UNSIGNED_BYTE;
			case RenderTargetTypes::ChannelPrecision::B16: return GL_UNSIGNED_SHORT;
			case RenderTargetTypes::ChannelPrecision::B32: return GL_UNSIGNED_INT;
			default:
				GM_CORE_ASSERT(false, "Unsupported channel precision for integer type");
				return -1;
			}
		}

		GM_CORE_ASSERT(false, "Unsupported channel data type.");
		return -1;
	}

	int OGLContext::ToNativeChannelCount(RenderTargetTypes::ComponentType componentType)
	{
		switch (componentType)
		{
		case RenderTargetTypes::ComponentType::RGBA: return GL_RGBA;
		case RenderTargetTypes::ComponentType::RGB: return GL_RGB;
		case RenderTargetTypes::ComponentType::RG: return GL_RG;
		case RenderTargetTypes::ComponentType::R: return GL_R;
		case RenderTargetTypes::ComponentType::Depth: return GL_DEPTH_COMPONENT;
		default:
			GM_CORE_ASSERT(false, "Unsupported channel count.");
			return -1;
		}
	}

	int OGLContext::ToNativeParameterKey(RenderTargetTypes::ParameterKey key)
	{
		switch (key)
		{
		case RenderTargetTypes::ParameterKey::MinFilter: return GL_TEXTURE_MIN_FILTER;
		case RenderTargetTypes::ParameterKey::MagFilter: return GL_TEXTURE_MAG_FILTER;
		case RenderTargetTypes::ParameterKey::HorizontalWrap: return GL_TEXTURE_WRAP_S;
		case RenderTargetTypes::ParameterKey::VerticalWrap: return GL_TEXTURE_WRAP_T;
		default:
			GM_CORE_ASSERT(false, "Unsupported parameter key.");
			return -1;
		}
	}

	int OGLContext::ToNativeParameterValue(RenderTargetTypes::ParameterValue value)
	{
		switch (value)
		{
		case RenderTargetTypes::ParameterValue::Nearest: return GL_NEAREST;
		case RenderTargetTypes::ParameterValue::Linear: return GL_LINEAR;
		case RenderTargetTypes::ParameterValue::MipMapLinear: return GL_LINEAR_MIPMAP_LINEAR;
		case RenderTargetTypes::ParameterValue::ClampToEdge: return GL_CLAMP_TO_EDGE;
		case RenderTargetTypes::ParameterValue::Repeat: return GL_REPEAT;
		default:
			GM_CORE_ASSERT(false, "Unsupported parameter key.");
			return -1;
		}
	}
}