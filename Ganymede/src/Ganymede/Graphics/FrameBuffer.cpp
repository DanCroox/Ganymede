#include "FrameBuffer.h"

#include "RenderTarget.h"
#include "OGLBindingHelper.h"
#include "Ganymede/Log/Log.h"
#include <GL/glew.h>

namespace Ganymede
{
	namespace FrameBuffer_Private
	{
		static unsigned int ToNativeAttachment(FrameBuffer::AttachmentType attachmentType)
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

		static unsigned int ToNativeBlitFilterType(FrameBuffer::BlitFilterType filterType)
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

		static void BindFrameBufferTexture(FrameBuffer::AttachmentType attachmentType, const RenderTarget& renderTarget)
			{
			// Could be a faster approach to dynamic casts but sicne setting framebuffer should (hopefully) never used in per-frame operations
			// speed should not matter too much here.
			if (dynamic_cast<const SinglesampleRenderTarget*>(&renderTarget) != nullptr)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, FrameBuffer_Private::ToNativeAttachment(attachmentType), GL_TEXTURE_2D, renderTarget.GetRenderID(), 0);
				return;
			}
			else if (dynamic_cast<const MultisampleRenderTarget*>(&renderTarget) != nullptr)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, FrameBuffer_Private::ToNativeAttachment(attachmentType), GL_TEXTURE_2D_MULTISAMPLE, renderTarget.GetRenderID(), 0);
				return;
			}
			else if (dynamic_cast<const CubeMapArrayRenderTarget*>(&renderTarget) != nullptr)
			{
				glFramebufferTexture(GL_FRAMEBUFFER, FrameBuffer_Private::ToNativeAttachment(attachmentType), renderTarget.GetRenderID(), 0);
				return;
			}

			GM_CORE_ASSERT(false, "Unsupported render target type.");
		}
	}

	FrameBuffer::FrameBuffer(glm::u32vec2 renderDimension, bool isHardWareFrameBuffer) :
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
	
	FrameBuffer::~FrameBuffer()
	{
		OGLBindingHelper::UnbindFrameBuffer();
		glDeleteFramebuffers(1, &m_RenderID);
	}

	void FrameBuffer::SetFrameBufferAttachment(AttachmentType attachmentType, RenderTarget& frameBufferTexture)
	{
		OGLBindingHelper::BindFrameBuffer(*this);
		FrameBuffer_Private::BindFrameBufferTexture(attachmentType, frameBufferTexture);
		m_FrameBufferAttachments[attachmentType] = &frameBufferTexture;
		UpdateActiveDrawBufferAttachments();
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GM_CORE_ASSERT(false, "Framebuffer incomplete.");
		}
	}

	void FrameBuffer::Blit(const BlitFrameBufferConfig& blitConfig)
	{
		for (const auto& entry : blitConfig.m_AttachementsToBlit)
		{
			const unsigned int sourceFBId = entry.m_SourceFrameBuffer.GetRenderID();
			const unsigned int destFBId = entry.m_DestFrameBuffer.GetRenderID();

			const glm::u32vec4& sourcePixelBounds = entry.m_SourcePixelBounds;
			const glm::u32vec4& destPixelBounds = entry.m_DestPixelBounds;

			unsigned int nativeBufferTypeBit = 0;
			if (entry.m_SourceAttachement == FrameBuffer::AttachmentType::Depth)
			{
				nativeBufferTypeBit = GL_DEPTH_BUFFER_BIT;
			}
			else
			{
				nativeBufferTypeBit = GL_COLOR_BUFFER_BIT;
				glNamedFramebufferReadBuffer(sourceFBId, FrameBuffer_Private::ToNativeAttachment(entry.m_SourceAttachement));
				glNamedFramebufferDrawBuffer(destFBId, FrameBuffer_Private::ToNativeAttachment(entry.m_DestAttachement));
			}

			glBlitNamedFramebuffer(
				sourceFBId,
				destFBId,
				sourcePixelBounds.x,
				sourcePixelBounds.y,
				sourcePixelBounds.z,
				sourcePixelBounds.a,
				destPixelBounds.x,
				destPixelBounds.y,
				destPixelBounds.z,
				destPixelBounds.a,
				nativeBufferTypeBit,
				FrameBuffer_Private::ToNativeBlitFilterType(entry.m_FilterType)
			);
		}
	}

	void FrameBuffer::UpdateActiveDrawBufferAttachments()
	{
		std::vector<unsigned int> activeAttachments;
		activeAttachments.reserve(m_FrameBufferAttachments.size());

		for (const auto& pair : m_FrameBufferAttachments)
		{
			if (pair.first != FrameBuffer::AttachmentType::Depth)
			{
				// Depth attachments doesn't need to be activated by glDrawBuffers. Those are active once they are bound to the framebuffer.
				activeAttachments.push_back(FrameBuffer_Private::ToNativeAttachment(pair.first));
			}
		}

		if (activeAttachments.size() > 0)
		{
			glDrawBuffers(activeAttachments.size(), activeAttachments.data());
		}
	}
}