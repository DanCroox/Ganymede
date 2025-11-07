#pragma once

#include "Ganymede/Graphics/FrameBuffer.h"

namespace Ganymede
{
	class RenderTarget;

	class GANYMEDE_API OGLFrameBuffer : public FrameBuffer
	{
	public:
		OGLFrameBuffer(glm::u32vec2 renderDimension, bool isHardWareFrameBuffer);
		~OGLFrameBuffer() override;

		OGLFrameBuffer(OGLFrameBuffer&& other) noexcept;
		OGLFrameBuffer& operator=(OGLFrameBuffer&& other) noexcept;

		void SetFrameBufferAttachment(AttachmentType attachmentType, RenderTarget& frameBufferTexture) override;
		void SetColorBufferClearColor(const glm::vec4& color) override { m_ColorBufferClearColor = color; }
		void SetDepthBufferClearColor(float color) override { m_DepthBufferClearColor = color; }

		const std::unordered_map<AttachmentType, RenderTarget*>& GetFrameBufferAttachments() const override { return m_FrameBufferAttachments; }
		const glm::vec4& GetColorBufferClearColor() const override { return m_ColorBufferClearColor; }
		float GetDepthBufferClearColor() const override { return m_DepthBufferClearColor; }

		void Blit(
			FrameBuffer& m_SourceFrameBuffer,
			FrameBuffer::AttachmentType m_SourceAttachement,
			FrameBuffer::AttachmentType m_DestAttachement,
			const glm::u32vec4& m_SourcePixelBounds,
			const glm::u32vec4& m_DestPixelBounds,
			BlitFilterType m_FilterType) override;

		bool IsValid() const override { return m_IsHardwareFrameBuffer || m_RenderID != 0; }

		glm::u32vec2 GetRenderDimension() const { return m_RenderDimension; }
		unsigned int GetRenderID() const { return m_RenderID; }

		static unsigned int ToNativeAttachment(FrameBuffer::AttachmentType attachmentType);
		static unsigned int ToNativeBlitFilterType(FrameBuffer::BlitFilterType filterType);

	private:
		void UpdateActiveDrawBufferAttachments();

		bool m_IsHardwareFrameBuffer;
		unsigned int m_RenderID;

		glm::u32vec2 m_RenderDimension;

		std::unordered_map<AttachmentType, RenderTarget*> m_FrameBufferAttachments;
		glm::vec4 m_ColorBufferClearColor;
		float m_DepthBufferClearColor;
	};
}