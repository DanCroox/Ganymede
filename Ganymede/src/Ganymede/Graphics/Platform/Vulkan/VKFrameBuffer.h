#pragma once

#include "Ganymede/Graphics/FrameBuffer.h"
#include <volk.h>

namespace Ganymede
{
	class RenderTarget;

	class GANYMEDE_API VKFrameBuffer : public FrameBuffer
	{
	public:
		VKFrameBuffer(const FrameBufferAttachmentStorage& attachments, glm::u32vec2 renderDimension);
		~VKFrameBuffer() override;

		VKFrameBuffer(VKFrameBuffer&& other) noexcept;
		VKFrameBuffer& operator=(VKFrameBuffer&& other) noexcept;

		void Blit(
			FrameBuffer& m_SourceFrameBuffer,
			FrameBufferAttachmentTypee m_SourceAttachment,
			uint32_t m_SourceAttachmentLocation,
			FrameBufferAttachmentTypee m_DestAttachment,
			uint32_t m_DestAttachmentLocation,
			const glm::u32vec4& m_SourcePixelBounds,
			const glm::u32vec4& m_DestPixelBounds,
			FrameBufferBlitFilterType m_FilterType) override;

		bool IsValid() const override { return true; }

		glm::u32vec2 GetRenderDimension() const { return m_RenderDimension; }

		static unsigned int ToNativeAttachmentType(FrameBufferAttachmentTypee attachmentType, uint32_t attachmentLocation);
		static unsigned int ToNativeBlitFilterType(FrameBufferBlitFilterType filterType);

		VkRenderPass GetVkRenderPass() const { return m_VkRenderPass; }
		const std::vector<VkFramebuffer>& GetVkFrameBuffers() const{ return m_VkFramebuffers; }

		bool HasDepthAttachment() const { return m_HasDepthAttachment; }

		static VkAccessFlags AccessMaskForLayout(VkImageLayout layout);
		static VkPipelineStageFlags StageForLayout(VkImageLayout layout);
		const std::vector<VkPipelineColorBlendAttachmentState>& GetColorBlendAttachments() const { return m_ColorBlendAttachments; };

	private:
		VkRenderPass m_VkRenderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> m_VkFramebuffers;
		bool m_HasDepthAttachment = false;
		std::vector<VkPipelineColorBlendAttachmentState> m_ColorBlendAttachments;
	};
}