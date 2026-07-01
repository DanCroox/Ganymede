#include "VKGPUTexture.h"

#include "Ganymede/Graphics/Texture.h"
#include "VKContext.h"
#include "VKDescriptorSetPoolManager.h"
#include <glm/glm.hpp>

namespace Ganymede
{
	namespace
	{
		VkCommandBuffer beginSingleTimeCommands()
		{
			VKContext& vkContext = VKContext::GetInstance();

			VkCommandBufferAllocateInfo alloc{};
			alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			alloc.commandPool = vkContext.device.commandPoolHandle();
			alloc.commandBufferCount = 1;

			VkCommandBuffer cmd;
			vkAllocateCommandBuffers(vkContext.device.device(), &alloc, &cmd);

			VkCommandBufferBeginInfo begin{};
			begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(cmd, &begin);

			return cmd;
		}

		void endSingleTimeCommands(VkCommandBuffer cmd)
		{
			VKContext& vkContext = VKContext::GetInstance();

			vkEndCommandBuffer(cmd);
			VkSubmitInfo submit{};
			submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit.commandBufferCount = 1;
			submit.pCommandBuffers = &cmd;

			VkQueue queue = vkContext.device.graphicsQueue();
			vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE);
			vkQueueWaitIdle(queue);

			vkFreeCommandBuffers(vkContext.device.device(), vkContext.device.commandPoolHandle(), 1, &cmd);
		}

		// ---------------------------
		// Helper: Layout transition
		// ---------------------------
		void transitionImageLayout(
			VkImage image,
			VkImageLayout oldLayout,
			VkImageLayout newLayout)
		{
			VKContext& vkContext = VKContext::GetInstance();
			VkCommandBuffer cmd = beginSingleTimeCommands();

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags srcStage, dstStage;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
				newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
				newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}

			vkCmdPipelineBarrier(cmd,
				srcStage, dstStage,
				0, 0, nullptr, 0, nullptr,
				1, &barrier);

			endSingleTimeCommands(cmd);
		}

		// ---------------------------
		// Helper: copy buffer image
		// ---------------------------
		void copyBufferToImage(
			VkBuffer buffer, VkImage image,
			uint32_t width, uint32_t height)
		{
			VkCommandBuffer cmd = beginSingleTimeCommands();

			VkBufferImageCopy region{};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageExtent = { width, height, 1 };

			vkCmdCopyBufferToImage(
				cmd,
				buffer,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region);

			endSingleTimeCommands(cmd);
		}

	}

	std::vector<uint8_t> ConvertToRGBA(const Texture& texture)
	{
		const unsigned int bitDepth = texture.GetBitDepth();
		const unsigned int numChannels = texture.GetNumChannels();
		const std::vector<unsigned char>& texBytes = texture.GetBytes();

		const size_t width = texture.GetWidth();
		const size_t height = texture.GetHeight();
		const size_t numPixels = width * height;

		const size_t bytesPerChannel = bitDepth / 8;
		const size_t bytesPerPixelIn = numChannels * bytesPerChannel;

		using ReadFunc = uint8_t(*)(const unsigned char*);

		static auto read8 = [](const unsigned char* p) -> uint8_t {
			return *p;
			};

		static auto read16 = [](const unsigned char* p) -> uint8_t {
			uint16_t v = uint16_t(p[1]) << 8 | p[0];
			return uint8_t((v * 255u) / 65535u);   // integer scaling (schnell!)
			};

		static auto read32 = [](const unsigned char* p) -> uint8_t {
			float f;
			std::memcpy(&f, p, 4);
			f = glm::clamp(f, 0.0f, 1.0f);
			return uint8_t(f * 255.0f);
			};

		ReadFunc read;
		switch (bitDepth) {
		case 8:  read = read8; break;
		case 16: read = read16; break;
		case 32: read = read32; break;
		default: read = read8; break; // sollte nicht passieren
		}

		std::vector<uint8_t> out;
		out.resize(numPixels * 4);   // Keine push_backs, keine Reallocs!

		uint8_t* dst = out.data();
		const unsigned char* src = texBytes.data();

		for (size_t i = 0; i < numPixels; ++i)
		{
			const unsigned char* px = src + i * bytesPerPixelIn;

			// R
			dst[0] = read(px);
			// G
			dst[1] = (numChannels > 1) ? read(px + 1 * bytesPerChannel) : 0;
			// B
			dst[2] = (numChannels > 2) ? read(px + 2 * bytesPerChannel) : 0;
			// A
			dst[3] = (numChannels > 3) ? read(px + 3 * bytesPerChannel) : 255;

			dst += 4;
		}

		return out;
	}

	VKGPUTexture::VKGPUTexture(const Texture& texture) : GPUTexture(texture)
	{
		VKContext& vkContext = VKContext::GetInstance();
		VkDevice device = vkContext.device.device();

		VkDeviceSize imageSize = texture.GetWidth() * texture.GetHeight() * 4; // RGBA8

		// -----------------------
		// 1) STAGING BUFFER
		// -----------------------
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;
		vkContext.createBuffer(imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingMemory);

		// Convert Data
		const bool convertToRGBA = texture.GetNumChannels() != 4;
		std::vector<uint8_t> textData = convertToRGBA ? ConvertToRGBA(texture) : texture.GetBytes();

		void* data;
		vkMapMemory(device, stagingMemory, 0, textData.size(), 0, &data);
		memcpy(data, textData.data(), textData.size());
		vkUnmapMemory(device, stagingMemory);

		// -----------------------
		// 2) IMAGE create
		// -----------------------
		VkImageCreateInfo imgInfo{};
		imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imgInfo.imageType = VK_IMAGE_TYPE_2D;
		imgInfo.extent = { (uint32_t)texture.GetWidth(), (uint32_t)texture.GetHeight(), 1};
		imgInfo.mipLevels = 1;
		imgInfo.arrayLayers = 1;
		imgInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imgInfo.usage =
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT;
		imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &imgInfo, nullptr, &vkImage) != VK_SUCCESS)
			throw std::runtime_error("create image failed");

		// Allocate image memory
		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(device, vkImage, &memReq);

		VkMemoryAllocateInfo alloc{};
		alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc.allocationSize = memReq.size;
		alloc.memoryTypeIndex = vkContext.findMemoryType(
			memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device, &alloc, nullptr, &vkMemory) != VK_SUCCESS)
			throw std::runtime_error("allocate image memory failed");

		// Bind image to memory
		vkBindImageMemory(device, vkImage, vkMemory, 0);

		// -----------------------
		// 3) Layout transitions + Upload
		// -----------------------
		transitionImageLayout(vkImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		copyBufferToImage(
			stagingBuffer, vkImage, texture.GetWidth(), texture.GetHeight());

		transitionImageLayout(vkImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingMemory, nullptr);

		// -----------------------
		// 4) Image View
		// -----------------------
		VkImageViewCreateInfo view{};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.image = vkImage;
		view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view.format = VK_FORMAT_R8G8B8A8_UNORM;
		view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view.subresourceRange.baseMipLevel = 0;
		view.subresourceRange.levelCount = 1;
		view.subresourceRange.baseArrayLayer = 0;
		view.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &view, nullptr, &vkImageView) != VK_SUCCESS)
			throw std::runtime_error("failed to create image view");

		VkSamplerCreateInfo s{};
		s.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		s.magFilter = VK_FILTER_LINEAR;
		s.minFilter = VK_FILTER_LINEAR;
		s.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		s.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		s.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		s.anisotropyEnable = VK_TRUE;
		s.maxAnisotropy = 16.0f;
		s.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		s.unnormalizedCoordinates = VK_FALSE;
		s.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		vkCreateSampler(device, &s, nullptr, &textureSampler);

		vkContext.RegisterBindlessVKGPUTexture(*this);
		// END BINDLESS TEXTURE
	}

	VKGPUTexture::~VKGPUTexture()
	{
		VKContext& vkContext = VKContext::GetInstance();

		vkDestroyImageView(vkContext.device.device(), vkImageView, nullptr);
		vkDestroyImage(vkContext.device.device(), vkImage, nullptr);
		vkFreeMemory(vkContext.device.device(), vkMemory, nullptr);
	}

	VKGPUTexture::VKGPUTexture(VKGPUTexture&& other) noexcept
		: GPUTexture(std::move(other)),
		vkImage(other.vkImage),
		vkMemory(other.vkMemory),
		vkImageView(other.vkImageView)
	{
		vkImage = VK_NULL_HANDLE;
		vkMemory = VK_NULL_HANDLE;
		vkImageView = VK_NULL_HANDLE;
	}

	VKGPUTexture& VKGPUTexture::operator=(VKGPUTexture&& other) noexcept
	{
		if (this != &other)
		{
			GPUTexture::operator=(std::move(other));
			vkImage = other.vkImage;
			vkMemory = other.vkMemory;
			vkImageView = other.vkImageView;

			vkImage = VK_NULL_HANDLE;
			vkMemory = VK_NULL_HANDLE;
			vkImageView = VK_NULL_HANDLE;
		}

		return *this;
	}

	void VKGPUTexture::Bind(unsigned int slot) const
	{
	}

	void VKGPUTexture::Unbind() const
	{
	}
}