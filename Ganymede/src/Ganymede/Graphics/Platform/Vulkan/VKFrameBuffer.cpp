#include "VKFrameBuffer.h"

#include "Ganymede/Log/Log.h"
#include "VKRenderTarget.h"
#include "VKContext.h"

namespace Ganymede
{
	namespace
	{
		std::optional<VkImageLayout> ToNativeAttachmentLayout(FrameBufferAttachmentTypee attachmentType)
		{
			switch (attachmentType)
			{
			case FrameBufferAttachmentTypee::Color: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case FrameBufferAttachmentTypee::Depth: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			default: break;
			}

			GM_CORE_ASSERT(false, "Unsupported framebuffer attachment.");
			return std::nullopt;
		}
	}

	VKFrameBuffer::VKFrameBuffer(const FrameBufferAttachmentStorage& attachments, glm::u32vec2 renderDimension) :
		FrameBuffer(attachments, renderDimension)
	{
		VKContext& vkContext = VKContext::GetInstance();

		// ---------------------------
		// 6) Render Pass definieren
		// ---------------------------
		// Color Attachment definieren
		std::vector<VkAttachmentDescription> attachmentDescriptions;
		std::vector<VkAttachmentReference> colorAttachmentReferences;
		std::vector<VkImageView> imageViews;
		attachmentDescriptions.reserve(attachments.size());
		colorAttachmentReferences.reserve(attachments.size());
		std::optional<VkAttachmentReference> depthAttachmentReferenceOpt = std::nullopt;
		for (const uint32_t bindingLocation : attachments)
		{
			GM_CORE_INFO(bindingLocation);
			const FrameBufferAttachment& attachment = *attachments.TryGetByBindingLocation(bindingLocation);
			const FrameBufferAttachmentTypee attachmentType = attachment.m_AttachmentType;
			const uint32_t attachmentLocation = attachment.m_AttachmentLocation;
			const VKRenderTarget& renderTarget = *static_cast<const VKRenderTarget*>(attachment.m_RenderTarget);

			const std::optional<VkImageLayout> nativeAttachmentLayout = ToNativeAttachmentLayout(attachment.m_AttachmentType);
			if (nativeAttachmentLayout.has_value() == false)
			{
				GM_CORE_ASSERT(false, "Unsupported attachment type");
				return;
			}

			VkAttachmentDescription& vkAttachmentDescription = attachmentDescriptions.emplace_back();
			vkAttachmentDescription.format = renderTarget.m_VkFormat;
			vkAttachmentDescription.samples = renderTarget.GetSampleCountFlagBits();
			vkAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // Inhalt zu Beginn loeschen
			vkAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Inhalt am Ende speichern (fuer Present)
			vkAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			vkAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			switch (attachmentType)
			{
			case FrameBufferAttachmentTypee::Color:
			{
				vkAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				vkAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				
				VkAttachmentReference& colorAttachmentRef = colorAttachmentReferences.emplace_back();
				colorAttachmentRef.attachment = static_cast<uint32_t>(attachmentDescriptions.size() - 1);
				colorAttachmentRef.layout = nativeAttachmentLayout.value();

				VkPipelineColorBlendAttachmentState& colorBlendAttachment = m_ColorBlendAttachments.emplace_back();
				colorBlendAttachment.colorWriteMask =
					VK_COLOR_COMPONENT_R_BIT |
					VK_COLOR_COMPONENT_G_BIT |
					VK_COLOR_COMPONENT_B_BIT |
					VK_COLOR_COMPONENT_A_BIT;
				break;
			}
			case FrameBufferAttachmentTypee::Depth:
			{
				vkAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				vkAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

				depthAttachmentReferenceOpt = VkAttachmentReference();
				depthAttachmentReferenceOpt.value().attachment = static_cast<uint32_t>(attachmentDescriptions.size() - 1);
				depthAttachmentReferenceOpt.value().layout = nativeAttachmentLayout.value();
				break;
			}
			default:
				GM_CORE_ASSERT(false, "Unsupported attachment type");
				return;
			}
		}

		m_HasDepthAttachment = depthAttachmentReferenceOpt.has_value(); 
		// Subpass definieren
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = colorAttachmentReferences.size();
		subpass.pColorAttachments = colorAttachmentReferences.data();
		subpass.pDepthStencilAttachment = m_HasDepthAttachment ? &depthAttachmentReferenceOpt.value() : nullptr;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments = attachmentDescriptions.data();

		m_VkRenderPass = {};
		if (vkCreateRenderPass(vkContext.device.device(), &renderPassInfo, nullptr, &m_VkRenderPass) != VK_SUCCESS)
		{
			GM_CORE_ASSERT(false, "Cannot create renderpass");
			return;
		}

		uint32_t& scImgCount = vkContext.m_SCImgCount;
		m_VkFramebuffers.resize(scImgCount);
		for (size_t i = 0; i < scImgCount; i++)
		{
			std::vector<VkImageView> views;
			for (const uint32_t bindingLocation : m_Attachments)
			{
				// TODO dd in correct order in respect to attachmentDescriptions. This is wrong
				const FrameBufferAttachment& attach = *m_Attachments.TryGetByBindingLocation(bindingLocation);
				views.push_back(static_cast<const VKRenderTarget*>(attach.m_RenderTarget)->m_VkImageView[i]);
			}

			VkFramebufferCreateInfo fbInfo{};
			fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbInfo.renderPass = m_VkRenderPass;
			fbInfo.attachmentCount = static_cast<uint32_t>(views.size());
			fbInfo.pAttachments = views.data();
			fbInfo.width = vkContext.extent.width;
			fbInfo.height = vkContext.extent.height;
			fbInfo.layers = 1;

			if (vkCreateFramebuffer(vkContext.device.device(), &fbInfo, nullptr, &m_VkFramebuffers[i]) != VK_SUCCESS)
			{
				GM_CORE_ASSERT(false, "Failed to create framebuffer!");
				return;
			}
		}
	}

	VKFrameBuffer::~VKFrameBuffer()
	{
		VKContext& vkContext = VKContext::GetInstance();

		if (m_VkRenderPass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(vkContext.device.device(), m_VkRenderPass, nullptr);
			m_VkRenderPass = VK_NULL_HANDLE;
		}

		for (auto framebuffer : m_VkFramebuffers)
		{
			if (framebuffer != VK_NULL_HANDLE)
			{
				vkDestroyFramebuffer(vkContext.device.device(), framebuffer, nullptr);
			}
		}
	}

	VKFrameBuffer::VKFrameBuffer(VKFrameBuffer&& other) noexcept :
		FrameBuffer(std::move(other)),
		m_VkRenderPass(other.m_VkRenderPass),
		m_VkFramebuffers(other.m_VkFramebuffers)
	{
		other.m_VkRenderPass = VK_NULL_HANDLE;
		other.m_VkFramebuffers.clear();
	}

	VKFrameBuffer& VKFrameBuffer::operator=(VKFrameBuffer&& other) noexcept
	{
		if (this != &other)
		{
			FrameBuffer::operator=(std::move(other));
			m_VkRenderPass = other.m_VkRenderPass;
			m_VkFramebuffers = other.m_VkFramebuffers;

			other.m_VkRenderPass = VK_NULL_HANDLE;
			other.m_VkFramebuffers.clear();
		}

		return *this;
	}

	VkAccessFlags VKFrameBuffer::AccessMaskForLayout(VkImageLayout layout)
	{
		switch (layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED: return 0;
		case VK_IMAGE_LAYOUT_PREINITIALIZED: return VK_ACCESS_HOST_WRITE_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_ACCESS_SHADER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return VK_ACCESS_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return VK_ACCESS_TRANSFER_WRITE_BIT;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return 0;
		default: return 0;
		}
	}

	VkPipelineStageFlags VKFrameBuffer::StageForLayout(VkImageLayout layout)
	{
		switch (layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED: return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		case VK_IMAGE_LAYOUT_PREINITIALIZED: return VK_PIPELINE_STAGE_HOST_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return VK_PIPELINE_STAGE_TRANSFER_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return VK_PIPELINE_STAGE_TRANSFER_BIT;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		default: return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		}
	}

	void BlitImage(
		VkCommandBuffer cmd,
		VkImage srcImage, VkImage dstImage,
		VkExtent3D srcExtent, VkExtent3D dstExtent,
		VkImageLayout srcOldLayout, VkImageLayout dstOldLayout,
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		VkFilter filter = VK_FILTER_LINEAR,
		VkImageLayout srcFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VkImageLayout dstFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		uint32_t srcMip = 0, uint32_t dstMip = 0,
		uint32_t srcBaseLayer = 0, uint32_t dstBaseLayer = 0,
		uint32_t layerCount = 1)
	{
		// Wenn Depth/Stencil -> Filter != LINEAR
		if (aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
			filter = VK_FILTER_NEAREST;

		// 1) �berg�nge in TRANSFER_SRC / TRANSFER_DST
		VkImageMemoryBarrier barriers[2] = {};

		// src -> TRANSFER_SRC_OPTIMAL
		barriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barriers[0].oldLayout = srcOldLayout;
		barriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].image = srcImage;
		barriers[0].subresourceRange.aspectMask = aspectMask;
		barriers[0].subresourceRange.baseMipLevel = srcMip;
		barriers[0].subresourceRange.levelCount = 1;
		barriers[0].subresourceRange.baseArrayLayer = srcBaseLayer;
		barriers[0].subresourceRange.layerCount = layerCount;
		barriers[0].srcAccessMask = VKFrameBuffer::AccessMaskForLayout(srcOldLayout);
		barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		// dst -> TRANSFER_DST_OPTIMAL
		barriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barriers[1].oldLayout = dstOldLayout;
		barriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[1].image = dstImage;
		barriers[1].subresourceRange.aspectMask = aspectMask;
		barriers[1].subresourceRange.baseMipLevel = dstMip;
		barriers[1].subresourceRange.levelCount = 1;
		barriers[1].subresourceRange.baseArrayLayer = dstBaseLayer;
		barriers[1].subresourceRange.layerCount = layerCount;
		barriers[1].srcAccessMask = VKFrameBuffer::AccessMaskForLayout(dstOldLayout);
		barriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(
			cmd,
			VKFrameBuffer::StageForLayout(srcOldLayout),
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			2, barriers);

		// 2) Blit region
		VkImageBlit blit{};
		blit.srcSubresource.aspectMask = aspectMask;
		blit.srcSubresource.mipLevel = srcMip;
		blit.srcSubresource.baseArrayLayer = srcBaseLayer;
		blit.srcSubresource.layerCount = layerCount;
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = {
			static_cast<int32_t>(srcExtent.width),
			static_cast<int32_t>(srcExtent.height),
			static_cast<int32_t>(srcExtent.depth) };

		blit.dstSubresource.aspectMask = aspectMask;
		blit.dstSubresource.mipLevel = dstMip;
		blit.dstSubresource.baseArrayLayer = dstBaseLayer;
		blit.dstSubresource.layerCount = layerCount;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = {
			static_cast<int32_t>(dstExtent.width),
			static_cast<int32_t>(dstExtent.height),
			static_cast<int32_t>(dstExtent.depth) };

		vkCmdBlitImage(
			cmd,
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			filter);

		// 3) �berg�nge zu final layouts
		VkImageMemoryBarrier post[2] = {};

		// src
		post[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		post[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		post[0].newLayout = srcFinalLayout;
		post[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		post[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		post[0].image = srcImage;
		post[0].subresourceRange.aspectMask = aspectMask;
		post[0].subresourceRange.baseMipLevel = srcMip;
		post[0].subresourceRange.levelCount = 1;
		post[0].subresourceRange.baseArrayLayer = srcBaseLayer;
		post[0].subresourceRange.layerCount = layerCount;
		post[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		post[0].dstAccessMask = VKFrameBuffer::AccessMaskForLayout(srcFinalLayout);

		// dst
		post[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		post[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		post[1].newLayout = dstFinalLayout;
		post[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		post[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		post[1].image = dstImage;
		post[1].subresourceRange.aspectMask = aspectMask;
		post[1].subresourceRange.baseMipLevel = dstMip;
		post[1].subresourceRange.levelCount = 1;
		post[1].subresourceRange.baseArrayLayer = dstBaseLayer;
		post[1].subresourceRange.layerCount = layerCount;
		post[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		post[1].dstAccessMask = VKFrameBuffer::AccessMaskForLayout(dstFinalLayout);

		vkCmdPipelineBarrier(
			cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VKFrameBuffer::StageForLayout(srcFinalLayout) | VKFrameBuffer::StageForLayout(dstFinalLayout),
			0,
			0, nullptr,
			0, nullptr,
			2, post);
	}

	void VKFrameBuffer::Blit(
		FrameBuffer& m_SourceFrameBuffer,
		FrameBufferAttachmentTypee m_SourceAttachment,
		uint32_t m_SourceAttachmentLocation,
		FrameBufferAttachmentTypee m_DestAttachment,
		uint32_t m_DestAttachmentLocation,
		const glm::u32vec4& m_SourcePixelBounds,
		const glm::u32vec4& m_DestPixelBounds,
		FrameBufferBlitFilterType m_FilterType)
	{
		
	}
}