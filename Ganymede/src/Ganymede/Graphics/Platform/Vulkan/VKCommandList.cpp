#include "VKCommandList.h"

#include "VKContext.h"
#include "VKFrameBuffer.h"
#include "VKRenderTarget.h"
#include "VKVertexObject.h"
#include "VKPipeline.h"

namespace Ganymede
{
	VKCommandList::VKCommandList(VKBackBuffer<VkCommandBuffer> vkCommandBuffer) :
		CommandList(),
		m_VkCommandBuffer(vkCommandBuffer)
	{
		m_BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		m_BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	}

	VKCommandList::~VKCommandList()
	{
		VKContext& vkContext = VKContext::GetInstance();
		vkFreeCommandBuffers(vkContext.device.device(), vkContext.device.commandPoolHandle(), static_cast<uint32_t>(m_VkCommandBuffer.size()), m_VkCommandBuffer.data());
		g_VKClearBackBuffer(m_VkCommandBuffer);
	}

	void VKCommandList::Reset()
	{
		vkResetCommandBuffer(m_VkCommandBuffer[VKContext::GetInstance().m_FiFIndex], 0);
	}

	void VKCommandList::Begin()
	{
		m_BoundFramebuffer = nullptr;
		m_BoundPipeline = nullptr;

		if (vkBeginCommandBuffer(m_VkCommandBuffer[VKContext::GetInstance().m_FiFIndex], &m_BeginInfo) != VK_SUCCESS)
			throw std::runtime_error("failed to begin recording command buffer!");
	}

	void VKCommandList::End()
	{
		VKContext& vkContext = VKContext::GetInstance();
		const uint32_t fifIndex = vkContext.m_FiFIndex;
		const uint32_t scIndex = vkContext.m_SCIndex;

		if (m_BoundFramebuffer != nullptr)
		{
			vkCmdEndRenderPass(m_VkCommandBuffer[fifIndex]);

			for (const uint32_t bindingLocation : m_BoundFramebuffer->GetAttachments())
			{
				const FrameBufferAttachment& fbAttachment = *m_BoundFramebuffer->GetAttachments().TryGetByBindingLocation(bindingLocation);
				VKRenderTarget& renderTarget = *const_cast<VKRenderTarget*>(static_cast<const VKRenderTarget*>(fbAttachment.m_RenderTarget));

				VkImageLayout finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				if (fbAttachment.m_AttachmentType == FrameBufferAttachmentTypee::Depth)
				{
					finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				}
				renderTarget.m_CurrentLayout[scIndex] = finalLayout;
			}

			m_BoundFramebuffer = nullptr;
		}

		if (vkEndCommandBuffer(m_VkCommandBuffer[fifIndex]) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer!");
	}

	void VKCommandList::BindFrameBuffer(const FrameBuffer& framebuffer)
	{
		VKContext& vkContext = VKContext::GetInstance();
		const uint32_t fifIndex = vkContext.m_FiFIndex;
		const uint32_t scIndex = vkContext.m_SCIndex;

		const VKFrameBuffer& vkFramebuffer = static_cast<const VKFrameBuffer&>(framebuffer);
		VkRenderPassBeginInfo rpInfo{};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.renderPass = vkFramebuffer.GetVkRenderPass();
		rpInfo.framebuffer = vkFramebuffer.GetVkFrameBuffers()[scIndex];
		rpInfo.renderArea.offset = { 0, 0 };
		rpInfo.renderArea.extent = vkContext.extent;
		std::vector<VkClearValue> clearValues;
		clearValues.resize(vkFramebuffer.GetAttachments().size());
		uint32_t cli = 0;
		for (const uint32_t bindingLocation : vkFramebuffer.GetAttachments())
		{
			const FrameBufferAttachment& fbAttachments = *vkFramebuffer.GetAttachments().TryGetByBindingLocation(bindingLocation);
			if (fbAttachments.m_AttachmentType == FrameBufferAttachmentTypee::Color)
			{
				clearValues[cli].color = {
					vkFramebuffer.GetColorBufferClearColor().x,
					vkFramebuffer.GetColorBufferClearColor().y,
					vkFramebuffer.GetColorBufferClearColor().z,
					vkFramebuffer.GetColorBufferClearColor().a
				};
			}
			else
			{
				clearValues[cli].depthStencil = { vkFramebuffer.GetDepthBufferClearColor(), 0 };
			}
			++cli;
		}

		rpInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpInfo.pClearValues = clearValues.data();

		if (m_BoundFramebuffer != nullptr)
		{
			vkCmdEndRenderPass(m_VkCommandBuffer[fifIndex]);

			for (const uint32_t bindingLocation : m_BoundFramebuffer->GetAttachments())
			{
				const FrameBufferAttachment& fbAttachment = *m_BoundFramebuffer->GetAttachments().TryGetByBindingLocation(bindingLocation);
				VKRenderTarget& renderTarget = *const_cast<VKRenderTarget*>(static_cast<const VKRenderTarget*>(fbAttachment.m_RenderTarget));

				VkImageLayout finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				if (fbAttachment.m_AttachmentType == FrameBufferAttachmentTypee::Depth)
				{
					finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				}
				renderTarget.m_CurrentLayout[scIndex] = finalLayout;
			}

			m_BoundFramebuffer = nullptr;
		}

		// Implizite Layout-Barrieren vor RenderPass-Start
		for (const uint32_t bindingLocation : vkFramebuffer.GetAttachments())
		{
			const FrameBufferAttachment& fbAttachment = *vkFramebuffer.GetAttachments().TryGetByBindingLocation(bindingLocation);
			const VKRenderTarget& renderTarget = *const_cast<VKRenderTarget*>(static_cast<const VKRenderTarget*>(fbAttachment.m_RenderTarget));

			const VkImageLayout expectedLayout = fbAttachment.m_AttachmentType == FrameBufferAttachmentTypee::Depth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			const VkImageLayout currentLayout = renderTarget.m_CurrentLayout[scIndex];
			if (currentLayout != expectedLayout && currentLayout != VK_IMAGE_LAYOUT_UNDEFINED)
			{
				VkImageMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = currentLayout;
				barrier.newLayout = expectedLayout;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.image = renderTarget.m_VkImage[scIndex];
				barrier.subresourceRange.aspectMask = renderTarget.m_VkImageAspectFlags;
				barrier.subresourceRange.baseMipLevel = 0;
				barrier.subresourceRange.levelCount = 1;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;
				barrier.srcAccessMask = VKFrameBuffer::AccessMaskForLayout(currentLayout);
				barrier.dstAccessMask = VKFrameBuffer::AccessMaskForLayout(expectedLayout);

				vkCmdPipelineBarrier(
					m_VkCommandBuffer[fifIndex],
					VKFrameBuffer::StageForLayout(currentLayout),
					VKFrameBuffer::StageForLayout(expectedLayout),
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);
			}
		}

		vkCmdBeginRenderPass(m_VkCommandBuffer[fifIndex], &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Layout nach RenderPass-Start auf Working-Layout setzen
		for (const uint32_t bindingLocation : vkFramebuffer.GetAttachments())
		{
			const FrameBufferAttachment& fbAttachment = *vkFramebuffer.GetAttachments().TryGetByBindingLocation(bindingLocation);
			VKRenderTarget& renderTarget = *const_cast<VKRenderTarget*>(static_cast<const VKRenderTarget*>(fbAttachment.m_RenderTarget));

			VkImageLayout workingLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			if (fbAttachment.m_AttachmentType == FrameBufferAttachmentTypee::Depth)
			{
				workingLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
			renderTarget.m_CurrentLayout[scIndex] = workingLayout;
		}

		m_BoundFramebuffer = &vkFramebuffer;
	}

	void VKCommandList::BindPipeline(const Pipeline& pipeline)
	{
		VKContext& vkContext = VKContext::GetInstance();
		const uint32_t fifIndex = vkContext.m_FiFIndex;

		const VKPipeline& vkPipeline = static_cast<const VKPipeline&>(pipeline);
		vkCmdBindPipeline(m_VkCommandBuffer[fifIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline.GetVkPipeline());
		m_BoundPipeline = &vkPipeline;

		vkCmdBindDescriptorSets(m_VkCommandBuffer[fifIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_BoundPipeline->GetVkPipelineLayout(), 0, 1, &vkContext.m_BindlessDescriptorSet, 0, nullptr);
	}

	void VKCommandList::DrawGeometry(const VertexObject& vertexObject, PCData pcData)
	{
		if (m_BoundPipeline == nullptr)
		{
			GM_CORE_ASSERT(false, "No pipeline bound to commandlist. Bind pipeline first.");
			return;
		}

		VKContext& vkContext = VKContext::GetInstance();
		const VKVertexObject& vkVertexObject = static_cast<const VKVertexObject&>(vertexObject);
		
		std::vector<VkBuffer> vertexBuffers = { static_cast<VKDataBuffer<MeshVertexData>*>(vkVertexObject.m_LinkedBuffers[0])->m_VkBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(m_VkCommandBuffer[vkContext.m_FiFIndex], 0, vertexBuffers.size(), vertexBuffers.data(), offsets);
		vkCmdBindIndexBuffer(m_VkCommandBuffer[vkContext.m_FiFIndex], static_cast<VKDataBuffer<UInt32VertexData>*>(vkVertexObject.m_IndexBuffer.get())->m_VkBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdPushConstants(m_VkCommandBuffer[vkContext.m_FiFIndex], m_BoundPipeline->GetVkPipelineLayout(), VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(PCData), &pcData);

		vkCmdDrawIndexed(m_VkCommandBuffer[vkContext.m_FiFIndex], static_cast<uint32_t>(vkVertexObject.m_IndexBuffer.get()->m_BufferSize), 1, 0, 0, 0);
	}

	void VKCommandList::DrawFullscreenQuad(PCData pcData)
	{
		if (m_BoundPipeline == nullptr)
		{
			GM_CORE_ASSERT(false, "No pipeline bound to commandlist. Bind pipeline first.");
			return;
		}
		
		VKContext& vkContext = VKContext::GetInstance();
		vkCmdPushConstants(m_VkCommandBuffer[vkContext.m_FiFIndex], m_BoundPipeline->GetVkPipelineLayout(), VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(PCData), &pcData);
		vkCmdDraw(m_VkCommandBuffer[vkContext.m_FiFIndex], 3, 1, 0, 0);
	}
}