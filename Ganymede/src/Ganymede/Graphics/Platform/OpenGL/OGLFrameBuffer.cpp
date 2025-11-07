#include "OGLFrameBuffer.h"

#include "Ganymede/Log/Log.h"
#include "OGLContext.h"
#include "OGLRenderTarget.h"
#include <GL/glew.h>

namespace Ganymede
{
	namespace FrameBuffer_Private
	{
		static void BindFrameBufferTexture(FrameBuffer::AttachmentType attachmentType, const OGLRenderTarget& renderTarget)
		{
			// Could be a faster approach to dynamic casts but sicne setting framebuffer should (hopefully) never used in per-frame operations
			// speed should not matter too much here.
			if (const OGLSinglesampleRenderTarget* oglRenderTarget = dynamic_cast<const OGLSinglesampleRenderTarget*>(&renderTarget))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, OGLFrameBuffer::ToNativeAttachment(attachmentType), GL_TEXTURE_2D, oglRenderTarget->GetRenderID(), 0);
				return;
			}
			else if (const OGLMultisampleRenderTarget* oglRenderTarget = dynamic_cast<const OGLMultisampleRenderTarget*>(&renderTarget))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, OGLFrameBuffer::ToNativeAttachment(attachmentType), GL_TEXTURE_2D_MULTISAMPLE, oglRenderTarget->GetRenderID(), 0);
				return;
			}
			else if (const OGLCubeMapArrayRenderTarget* oglRenderTarget = dynamic_cast<const OGLCubeMapArrayRenderTarget*>(&renderTarget))
			{
				glFramebufferTexture(GL_FRAMEBUFFER, OGLFrameBuffer::ToNativeAttachment(attachmentType), oglRenderTarget->GetRenderID(), 0);
				return;
			}

			GM_CORE_ASSERT(false, "Unsupported render target type.");
		}
	}

	OGLFrameBuffer::OGLFrameBuffer(glm::u32vec2 renderDimension, bool isHardWareFrameBuffer) :
		FrameBuffer(renderDimension, isHardWareFrameBuffer),
		m_ColorBufferClearColor({ 0.0f, 1.0f, 0.0f, 1.0f }),
		m_DepthBufferClearColor(1.0f),
		m_RenderDimension(renderDimension),
		m_IsHardwareFrameBuffer(isHardWareFrameBuffer),
		m_RenderID(0)
	{
		if (isHardWareFrameBuffer)
		{
			return;
		}

		glGenFramebuffers(1, &m_RenderID);
		GM_CORE_ASSERT(m_RenderID != 0, "Couldn't create framebuffer.");
	}

	OGLFrameBuffer::~OGLFrameBuffer()
	{
		OGLContext::UnbindFrameBuffer();
		glDeleteFramebuffers(1, &m_RenderID);
	}

	OGLFrameBuffer::OGLFrameBuffer(OGLFrameBuffer&& other) noexcept :
		FrameBuffer(std::move(other)),
		m_IsHardwareFrameBuffer(other.m_IsHardwareFrameBuffer),
		m_RenderID(other.m_RenderID),
		m_RenderDimension(other.m_RenderDimension),
		m_FrameBufferAttachments(std::move(other.m_FrameBufferAttachments)),
		m_ColorBufferClearColor(other.m_ColorBufferClearColor),
		m_DepthBufferClearColor(other.m_DepthBufferClearColor)
	{
		other.m_RenderID = 0;
	}

	OGLFrameBuffer& OGLFrameBuffer::operator=(OGLFrameBuffer&& other) noexcept
	{
		if (this != &other)
		{
			FrameBuffer::operator=(std::move(other));
			m_IsHardwareFrameBuffer = other.m_IsHardwareFrameBuffer;
			m_RenderID = other.m_RenderID;
			m_RenderDimension = other.m_RenderDimension;
			m_FrameBufferAttachments = std::move(other.m_FrameBufferAttachments);
			m_ColorBufferClearColor = other.m_ColorBufferClearColor;
			m_DepthBufferClearColor = other.m_DepthBufferClearColor;

			m_RenderID = 0;
		}
		return *this;
	}

	void OGLFrameBuffer::SetFrameBufferAttachment(AttachmentType attachmentType, RenderTarget& frameBufferTexture)
	{
		OGLContext::BindFrameBuffer(*this);
		FrameBuffer_Private::BindFrameBufferTexture(attachmentType, static_cast<OGLRenderTarget&>(frameBufferTexture));
		m_FrameBufferAttachments[attachmentType] = &frameBufferTexture;
		UpdateActiveDrawBufferAttachments();
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GM_CORE_ASSERT(false, "Framebuffer incomplete.");
		}
	}

	void OGLFrameBuffer::Blit(
		FrameBuffer& m_SourceFrameBuffer,
		FrameBuffer::AttachmentType m_SourceAttachement,
		FrameBuffer::AttachmentType m_DestAttachement,
		const glm::u32vec4& m_SourcePixelBounds,
		const glm::u32vec4& m_DestPixelBounds,
		BlitFilterType m_FilterType)
	{
		const unsigned int sourceFBId = static_cast<OGLFrameBuffer&>(m_SourceFrameBuffer).GetRenderID();
		const unsigned int destFBId = GetRenderID();

		unsigned int nativeBufferTypeBit = 0;
		if (m_SourceAttachement == FrameBuffer::AttachmentType::Depth)
		{
			nativeBufferTypeBit = GL_DEPTH_BUFFER_BIT;
		}
		else
		{
			nativeBufferTypeBit = GL_COLOR_BUFFER_BIT;
			glNamedFramebufferReadBuffer(sourceFBId, OGLFrameBuffer::ToNativeAttachment(m_SourceAttachement));
			glNamedFramebufferDrawBuffer(destFBId, OGLFrameBuffer::ToNativeAttachment(m_DestAttachement));
		}

		glBlitNamedFramebuffer(
			sourceFBId,
			destFBId,
			m_SourcePixelBounds.x,
			m_SourcePixelBounds.y,
			m_SourcePixelBounds.z,
			m_SourcePixelBounds.a,
			m_DestPixelBounds.x,
			m_DestPixelBounds.y,
			m_DestPixelBounds.z,
			m_DestPixelBounds.a,
			nativeBufferTypeBit,
			OGLFrameBuffer::ToNativeBlitFilterType(m_FilterType)
		);
	}

	unsigned int OGLFrameBuffer::ToNativeAttachment(FrameBuffer::AttachmentType attachmentType)
	{
		switch (attachmentType)
		{
		case FrameBuffer::AttachmentType::Color0: return GL_COLOR_ATTACHMENT0;
		case FrameBuffer::AttachmentType::Color1: return GL_COLOR_ATTACHMENT1;
		case FrameBuffer::AttachmentType::Color2: return GL_COLOR_ATTACHMENT2;
		case FrameBuffer::AttachmentType::Color3: return GL_COLOR_ATTACHMENT3;
		case FrameBuffer::AttachmentType::Color4: return GL_COLOR_ATTACHMENT4;
		case FrameBuffer::AttachmentType::Color5: return GL_COLOR_ATTACHMENT5;
		case FrameBuffer::AttachmentType::Color6: return GL_COLOR_ATTACHMENT6;
		case FrameBuffer::AttachmentType::Color7: return GL_COLOR_ATTACHMENT7;
		case FrameBuffer::AttachmentType::Color8: return GL_COLOR_ATTACHMENT8;
		case FrameBuffer::AttachmentType::Color9: return GL_COLOR_ATTACHMENT9;
		case FrameBuffer::AttachmentType::Color10: return GL_COLOR_ATTACHMENT10;
		case FrameBuffer::AttachmentType::Color11: return GL_COLOR_ATTACHMENT11;
		case FrameBuffer::AttachmentType::Color12: return GL_COLOR_ATTACHMENT12;
		case FrameBuffer::AttachmentType::Color13: return GL_COLOR_ATTACHMENT13;
		case FrameBuffer::AttachmentType::Color14: return GL_COLOR_ATTACHMENT14;
		case FrameBuffer::AttachmentType::Color15: return GL_COLOR_ATTACHMENT15;
		case FrameBuffer::AttachmentType::Depth: return GL_DEPTH_ATTACHMENT;
		default:
			GM_CORE_ASSERT(false, "Unsupported framebuffer attachment.")
				return -1;
		}
	}

	unsigned int OGLFrameBuffer::ToNativeBlitFilterType(FrameBuffer::BlitFilterType filterType)
	{
		switch (filterType)
		{
		case FrameBuffer::BlitFilterType::Linear: return GL_LINEAR;
		case FrameBuffer::BlitFilterType::Nearest: return GL_NEAREST;
		default:
			GM_CORE_ASSERT(false, "Unsupported blit filter type.")
				return -1;
		}
	}

	void OGLFrameBuffer::UpdateActiveDrawBufferAttachments()
	{
		std::vector<unsigned int> activeAttachments;
		activeAttachments.reserve(m_FrameBufferAttachments.size());

		for (const auto& pair : m_FrameBufferAttachments)
		{
			if (pair.first != FrameBuffer::AttachmentType::Depth)
			{
				// Depth attachments doesn't need to be activated by glDrawBuffers. Those are active once they are bound to the framebuffer.
				activeAttachments.push_back(ToNativeAttachment(pair.first));
			}
		}

		if (activeAttachments.size() > 0)
		{
			glDrawBuffers(activeAttachments.size(), activeAttachments.data());
		}
	}
}