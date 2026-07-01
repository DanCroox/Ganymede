#include "OGLFrameBuffer.h"

#include "Ganymede/Log/Log.h"
#include "OGLContext.h"
#include "OGLRenderTarget.h"
#include <GL/glew.h>

namespace Ganymede
{
	namespace FrameBuffer_Private
	{
		static void BindFrameBufferTexture(FrameBufferAttachmentTypee attachmentType, uint32_t attachmentLocation, const OGLRenderTarget& renderTarget)
		{
			// Could be a faster approach to dynamic casts but sicne setting framebuffer should (hopefully) never used in per-frame operations
			// speed should not matter too much here.
			if (const OGLSinglesampleRenderTarget* oglRenderTarget = dynamic_cast<const OGLSinglesampleRenderTarget*>(&renderTarget))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, OGLFrameBuffer::ToNativeAttachmentType(attachmentType, attachmentLocation), GL_TEXTURE_2D, oglRenderTarget->GetRenderID(), 0);
				return;
			}
			else if (const OGLMultisampleRenderTarget* oglRenderTarget = dynamic_cast<const OGLMultisampleRenderTarget*>(&renderTarget))
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, OGLFrameBuffer::ToNativeAttachmentType(attachmentType, attachmentLocation), GL_TEXTURE_2D_MULTISAMPLE, oglRenderTarget->GetRenderID(), 0);
				return;
			}
			else if (const OGLCubeMapArrayRenderTarget* oglRenderTarget = dynamic_cast<const OGLCubeMapArrayRenderTarget*>(&renderTarget))
			{
				glFramebufferTexture(GL_FRAMEBUFFER, OGLFrameBuffer::ToNativeAttachmentType(attachmentType, attachmentLocation), oglRenderTarget->GetRenderID(), 0);
				return;
			}

			GM_CORE_ASSERT(false, "Unsupported render target type.");
		}
	}

	OGLFrameBuffer::OGLFrameBuffer(const FrameBufferAttachmentStorage& attachments, glm::u32vec2 renderDimension) :
		FrameBuffer(attachments, renderDimension),
		m_RenderID(0)
	{
		if (attachments.empty())
		{
			// Is hardware framebuffer
			// This is a special implementation only for 
			m_IsHardwareFrameBuffer = true;
			return;
		}

		m_IsHardwareFrameBuffer = false;

		glGenFramebuffers(1, &m_RenderID);
		GM_CORE_ASSERT(m_RenderID != 0, "Couldn't create framebuffer.");

		InitNativeFrameBuffer();
	}

	OGLFrameBuffer::~OGLFrameBuffer()
	{
		OGLContext::UnbindFrameBuffer();
		glDeleteFramebuffers(1, &m_RenderID);
	}

	OGLFrameBuffer::OGLFrameBuffer(OGLFrameBuffer&& other) noexcept :
		FrameBuffer(std::move(other)),
		m_IsHardwareFrameBuffer(other.m_IsHardwareFrameBuffer),
		m_RenderID(other.m_RenderID)
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

			other.m_RenderID = 0;
		}

		return *this;
	}

	void OGLFrameBuffer::Blit(
		FrameBuffer& m_SourceFrameBuffer,
		FrameBufferAttachmentTypee m_SourceAttachement,
		uint32_t m_SourceAttachementLocation,
		FrameBufferAttachmentTypee m_DestAttachement,
		uint32_t m_DestAttachementLocation,
		const glm::u32vec4& m_SourcePixelBounds,
		const glm::u32vec4& m_DestPixelBounds,
		FrameBufferBlitFilterType m_FilterType)
	{
		const unsigned int sourceFBId = static_cast<OGLFrameBuffer&>(m_SourceFrameBuffer).GetRenderID();
		const unsigned int destFBId = GetRenderID();

		unsigned int nativeBufferTypeBit = 0;
		if (m_SourceAttachement == FrameBufferAttachmentTypee::Depth)
		{
			nativeBufferTypeBit = GL_DEPTH_BUFFER_BIT;
		}
		else
		{
			nativeBufferTypeBit = GL_COLOR_BUFFER_BIT;
			glNamedFramebufferReadBuffer(sourceFBId, OGLFrameBuffer::ToNativeAttachmentType(m_SourceAttachement, m_SourceAttachementLocation));
			glNamedFramebufferDrawBuffer(destFBId, OGLFrameBuffer::ToNativeAttachmentType(m_DestAttachement, m_DestAttachementLocation));
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

	unsigned int OGLFrameBuffer::ToNativeAttachmentType(FrameBufferAttachmentTypee attachmentType, uint32_t attachmentLocation)
	{
		if (attachmentType == FrameBufferAttachmentTypee::Depth)
		{
			return GL_DEPTH_ATTACHMENT;
		}

		// Color attachments
		switch (attachmentLocation)
		{
		case 0: return GL_COLOR_ATTACHMENT0;
		case 1: return GL_COLOR_ATTACHMENT1;
		case 2: return GL_COLOR_ATTACHMENT2;
		case 3: return GL_COLOR_ATTACHMENT3;
		case 4: return GL_COLOR_ATTACHMENT4;
		case 5: return GL_COLOR_ATTACHMENT5;
		case 6: return GL_COLOR_ATTACHMENT6;
		case 7: return GL_COLOR_ATTACHMENT7;
		case 8: return GL_COLOR_ATTACHMENT8;
		case 9: return GL_COLOR_ATTACHMENT9;
		case 10: return GL_COLOR_ATTACHMENT10;
		case 11: return GL_COLOR_ATTACHMENT11;
		case 12: return GL_COLOR_ATTACHMENT12;
		case 13: return GL_COLOR_ATTACHMENT13;
		case 14: return GL_COLOR_ATTACHMENT14;
		case 15: return GL_COLOR_ATTACHMENT15;
		default: break;
		}
		
		GM_CORE_ASSERT(false, "Unsupported framebuffer attachment.")
		return -1;
	}

	unsigned int OGLFrameBuffer::ToNativeBlitFilterType(FrameBufferBlitFilterType filterType)
	{
		switch (filterType)
		{
		case FrameBufferBlitFilterType::Linear: return GL_LINEAR;
		case FrameBufferBlitFilterType::Nearest: return GL_NEAREST;
		default:
			GM_CORE_ASSERT(false, "Unsupported blit filter type.")
				return -1;
		}
	}

	void OGLFrameBuffer::InitNativeFrameBuffer()
	{
		std::vector<unsigned int> colorAttachments;
		colorAttachments.reserve(m_Attachments.size());

		OGLContext::BindFrameBuffer(*this);
		for (const uint32_t bindingLocation : m_Attachments)
		{
			const FrameBufferAttachment& attachment = *m_Attachments.TryGetByBindingLocation(bindingLocation);
			FrameBuffer_Private::BindFrameBufferTexture(attachment.m_AttachmentType, attachment.m_AttachmentLocation, *static_cast<const OGLRenderTarget*>(attachment.m_RenderTarget));
			if (attachment.m_AttachmentType != FrameBufferAttachmentTypee::Depth)
			{
				colorAttachments.push_back(ToNativeAttachmentType(attachment.m_AttachmentType, attachment.m_AttachmentLocation));
			}
		}
		
		glDrawBuffers(colorAttachments.size(), colorAttachments.data());
		GM_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer incomplete.");
	}
}