#include "Ganymede/Graphics/GPUCommands.h"

#include "Ganymede/Graphics/Platform/OpenGL/OGLFrameBuffer.h"
#include "Ganymede/Graphics/RenderTarget.h"
#include "Ganymede/Graphics/Shader.h"
#include "Ganymede/Graphics/VertexObject.h"
#include "OGLContext.h"
#include "OGLVertexObject.h"
#include <GL/glew.h>

namespace Ganymede
{
	namespace GPUCommands
	{
		void Rendering::BindShader(const Shader& shader)
		{
			OGLContext::BindShader(shader);
		}
		
		void Rendering::BindVertexObject(const VertexObject& vo)
		{
			OGLContext::BindVertexArrayObject(static_cast<const OGLVertexObject &>(vo));
		}

		void Compute::Dispatch(Shader& shader, unsigned int numWgX, unsigned int numWgY, unsigned int numWgZ)
		{
			OGLContext::BindShader(shader);
			glDispatchCompute(numWgX, numWgY, numWgZ);
		}

		void RenderTarget::ClearRenderTarget(
			Ganymede::RenderTarget& renderTarget,
			unsigned int mipLayer,
			unsigned int destX,
			unsigned int destY,
			unsigned int destDepth,
			unsigned int extendX,
			unsigned int extendY,
			unsigned int extendDepth,
			const void* pixelDataBytes)
		{
			glClearTexSubImage(
				renderTarget.GetRenderID(),
				mipLayer,
				destX, destY,
				destDepth,
				extendX, extendY,
				extendDepth,
				OGLContext::ToNativeChannelCount(renderTarget.GetComponentType()),
				OGLContext::ToNativeDataType(renderTarget.GetChannelDataType(), renderTarget.GetChannelPrecision()),
				pixelDataBytes
			);
		}

		void FrameBufferCommands::Blit(const FrameBuffer::BlitFrameBufferConfig& blitConfig)
		{
			for (const auto& entry : blitConfig.m_AttachementsToBlit)
			{
				const unsigned int sourceFBId = static_cast<OGLFrameBuffer&>(entry.m_SourceFrameBuffer).GetRenderID();
				const unsigned int destFBId = static_cast<OGLFrameBuffer&>(entry.m_DestFrameBuffer).GetRenderID();

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
					glNamedFramebufferReadBuffer(sourceFBId, OGLFrameBuffer::ToNativeAttachment(entry.m_SourceAttachement));
					glNamedFramebufferDrawBuffer(destFBId, OGLFrameBuffer::ToNativeAttachment(entry.m_DestAttachement));
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
					OGLFrameBuffer::ToNativeBlitFilterType(entry.m_FilterType)
				);
			}
		}
	}
}