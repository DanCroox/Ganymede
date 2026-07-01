#pragma once

#include "Ganymede/Graphics/FrameBuffer.h"

namespace Ganymede
{
	class RenderTarget;

	class GANYMEDE_API OGLFrameBuffer : public FrameBuffer
	{
	public:
		OGLFrameBuffer(const FrameBufferAttachmentStorage& attachments, glm::u32vec2 renderDimension);
		~OGLFrameBuffer() override;

		OGLFrameBuffer(OGLFrameBuffer&& other) noexcept;
		OGLFrameBuffer& operator=(OGLFrameBuffer&& other) noexcept;

		void Blit(
			FrameBuffer& m_SourceFrameBuffer,
			FrameBufferAttachmentTypee m_SourceAttachement,
			uint32_t m_SourceAttachementLocation,
			FrameBufferAttachmentTypee m_DestAttachement,
			uint32_t m_DestAttachementLocation,
			const glm::u32vec4& m_SourcePixelBounds,
			const glm::u32vec4& m_DestPixelBounds,
			FrameBufferBlitFilterType m_FilterType) override;

		bool IsValid() const override { return m_IsHardwareFrameBuffer || m_RenderID != 0; }

		unsigned int GetRenderID() const { return m_RenderID; }

		static unsigned int ToNativeAttachmentType(FrameBufferAttachmentTypee attachmentType, uint32_t attachmentLocation);
		static unsigned int ToNativeBlitFilterType(FrameBufferBlitFilterType filterType);

	private:
		void InitNativeFrameBuffer();

		unsigned int m_RenderID;
		bool m_IsHardwareFrameBuffer;
	};
}