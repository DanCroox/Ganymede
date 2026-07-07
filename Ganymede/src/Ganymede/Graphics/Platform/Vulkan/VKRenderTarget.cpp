#include "VKRenderTarget.h"
#include "VKContext.h"

namespace Ganymede
{
	namespace
	{
		VkImageAspectFlags ToNativeImageAspectFlag(RenderTargetTypes::ComponentType componentType)
		{
			switch (componentType)
			{
			case RenderTargetTypes::ComponentType::R:
			case RenderTargetTypes::ComponentType::RG:
			case RenderTargetTypes::ComponentType::RGB:
			case RenderTargetTypes::ComponentType::RGBA: return VK_IMAGE_ASPECT_COLOR_BIT;
			case RenderTargetTypes::ComponentType::Depth: return VK_IMAGE_ASPECT_DEPTH_BIT;
			}

			GM_CORE_ASSERT(false, "Unsupported component type for image aspect");
			return 0;
		}

		VkImageUsageFlags ToNativeImageUsageFlags(RenderTargetTypes::ComponentType componentType)
		{
			VkImageUsageFlags flags = 0;

			switch (componentType)
			{
			case RenderTargetTypes::ComponentType::R:
			case RenderTargetTypes::ComponentType::RG:
			case RenderTargetTypes::ComponentType::RGB:
			case RenderTargetTypes::ComponentType::RGBA: flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; break;
			case RenderTargetTypes::ComponentType::Depth: flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; break; 
			}

			GM_CORE_ASSERT(flags != 0, "Unsupported component type for usage type");

			// We always want to be able to sample and blit from/to our RT
			flags |= VK_IMAGE_USAGE_SAMPLED_BIT |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
				VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			return flags;
		}

		VkFormat ToNativeFormat(
			RenderTargetTypes::ComponentType componentType,
			RenderTargetTypes::ChannelDataType dataType,
			RenderTargetTypes::ChannelPrecision precision)
		{
			if (dataType == RenderTargetTypes::ChannelDataType::Float)
			{
				if (precision == RenderTargetTypes::ChannelPrecision::B8)
				{
					GM_CORE_ASSERT(false, "Unsupported channel precision for given channel type (float)");
				}

				if (precision == RenderTargetTypes::ChannelPrecision::B16)
				{
					switch (componentType)
					{
					case RenderTargetTypes::ComponentType::R:    return VK_FORMAT_R16_SFLOAT;
					case RenderTargetTypes::ComponentType::RG:   return VK_FORMAT_R16G16_SFLOAT;
					case RenderTargetTypes::ComponentType::RGB:  return VK_FORMAT_R16G16B16A16_SFLOAT;
					case RenderTargetTypes::ComponentType::RGBA: return VK_FORMAT_R16G16B16A16_SFLOAT;
					case RenderTargetTypes::ComponentType::Depth: GM_CORE_ASSERT(false, "There is no 16 bit depth float. Try UNorm.");
						;
					}
				}

				if (precision == RenderTargetTypes::ChannelPrecision::B32)
				{
					switch (componentType)
					{
					case RenderTargetTypes::ComponentType::R:		return VK_FORMAT_R32_SFLOAT;
					case RenderTargetTypes::ComponentType::RG:		return VK_FORMAT_R32G32_SFLOAT;
					case RenderTargetTypes::ComponentType::RGB:		return VK_FORMAT_R32G32B32A32_SFLOAT;
					case RenderTargetTypes::ComponentType::RGBA:	return VK_FORMAT_R32G32B32A32_SFLOAT;
					case RenderTargetTypes::ComponentType::Depth:	return VK_FORMAT_D32_SFLOAT;
					}
				}

				GM_CORE_ASSERT(false, "Unsupported channel precision for given channel type (Float)");
			}

			if (dataType == RenderTargetTypes::ChannelDataType::UInt)
			{
				if (precision == RenderTargetTypes::ChannelPrecision::B8)
				{
					switch (componentType)
					{
					case RenderTargetTypes::ComponentType::R:		return VK_FORMAT_R8_UINT;
					case RenderTargetTypes::ComponentType::RG:		return VK_FORMAT_R8G8_UINT;
					case RenderTargetTypes::ComponentType::RGB:		return VK_FORMAT_R8G8B8A8_UINT;
					case RenderTargetTypes::ComponentType::RGBA:	return VK_FORMAT_R8G8B8A8_UINT;
					case RenderTargetTypes::ComponentType::Depth:	GM_CORE_ASSERT(false, "Unsupported component type (Depth) for given channel precision and type (8Bit UInt)"); break;
					}
				}

				if (precision == RenderTargetTypes::ChannelPrecision::B16)
				{
					switch (componentType)
					{
					case RenderTargetTypes::ComponentType::R:		return VK_FORMAT_R16_UINT;
					case RenderTargetTypes::ComponentType::RG:		return VK_FORMAT_R16G16_UINT;
					case RenderTargetTypes::ComponentType::RGB:		return VK_FORMAT_R16G16B16A16_UINT;
					case RenderTargetTypes::ComponentType::RGBA:	return VK_FORMAT_R16G16B16A16_UINT;
					case RenderTargetTypes::ComponentType::Depth:	GM_CORE_ASSERT(false, "Unsupported component type (Depth) for given channel precision and type (16Bit UInt)"); break;
					}
				}

				if (precision == RenderTargetTypes::ChannelPrecision::B32)
				{
					switch (componentType)
					{
					case RenderTargetTypes::ComponentType::R:		return VK_FORMAT_R32_UINT;
					case RenderTargetTypes::ComponentType::RG:		return VK_FORMAT_R32G32_UINT;
					case RenderTargetTypes::ComponentType::RGB:		return VK_FORMAT_R32G32B32A32_UINT;
					case RenderTargetTypes::ComponentType::RGBA:	return VK_FORMAT_R32G32B32A32_UINT;
					case RenderTargetTypes::ComponentType::Depth:	GM_CORE_ASSERT(false, "Unsupported component type (Depth) for given channel precision and type (32Bit UInt)"); break;
					}
				}

				GM_CORE_ASSERT(false, "Unsupported channel precision for given channel type (UInt)");
			}

			if (dataType == RenderTargetTypes::ChannelDataType::UNorm)
			{
				if (precision == RenderTargetTypes::ChannelPrecision::B8)
				{
					switch (componentType)
					{
					case RenderTargetTypes::ComponentType::R:		return VK_FORMAT_R8_UNORM;
					case RenderTargetTypes::ComponentType::RG:		return VK_FORMAT_R8G8_UNORM;
					case RenderTargetTypes::ComponentType::RGB:		return VK_FORMAT_R8G8B8A8_UNORM;
					case RenderTargetTypes::ComponentType::RGBA:	return VK_FORMAT_R8G8B8A8_UNORM;
					case RenderTargetTypes::ComponentType::Depth:	GM_CORE_ASSERT(false, "Unsupported component type (Depth) for given channel precision and type (8Bit UInt)"); break;
					}
				}

				if (precision == RenderTargetTypes::ChannelPrecision::B16)
				{
					switch (componentType)
					{
					case RenderTargetTypes::ComponentType::R:		return VK_FORMAT_R16_UNORM;
					case RenderTargetTypes::ComponentType::RG:		return VK_FORMAT_R16G16_UNORM;
					case RenderTargetTypes::ComponentType::RGB:		return VK_FORMAT_R16G16B16A16_UNORM;
					case RenderTargetTypes::ComponentType::RGBA:	return VK_FORMAT_R16G16B16A16_UNORM;
					case RenderTargetTypes::ComponentType::Depth:	GM_CORE_ASSERT(false, "Unsupported component type (Depth) for given channel precision and type (16Bit UInt)"); break;
					}
				}

				if (precision == RenderTargetTypes::ChannelPrecision::B32)
				{
					GM_CORE_ASSERT(false, "Unsupported channel data type for given channel precision (32Bit). There is no 32bit UNorm. Try 32bit Float.");
				}

				GM_CORE_ASSERT(false, "Unsupported channel precision for given channel type (UNorm)");
			}

			GM_CORE_ASSERT(false, "Unsupported type requested");
			return VK_FORMAT_UNDEFINED;
		}

	}

	VKRenderTarget::VKRenderTarget(
		RenderTargetTypes::ComponentType componentType,
		RenderTargetTypes::ChannelDataType dataType,
		RenderTargetTypes::ChannelPrecision precision,
		glm::uvec2 size) :
		RenderTarget(componentType, dataType, precision, size),
		m_CurrentLayout{}
	{
		for (auto& layout : m_CurrentLayout)
		{
			layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		ResetBindlessDSIndex();
	};

	VKRenderTarget::~VKRenderTarget()
	{
		VkDevice device = VKContext::GetInstance().device.device();
		if (g_VKGetSCIndex(m_VkImageView) != VK_NULL_HANDLE)
		{
			vkDestroyImageView(device, g_VKGetSCIndex(m_VkImageView), nullptr);
		}

		if (g_VKGetSCIndex(m_VkImage) != VK_NULL_HANDLE)
		{
			vkDestroyImage(device, g_VKGetSCIndex(m_VkImage), nullptr);
		}
	}

	VKRenderTarget::VKRenderTarget(VKRenderTarget&& other) noexcept
		: RenderTarget(std::move(other)),
		m_VkFormat(other.m_VkFormat),
		m_VkImage(other.m_VkImage),
		m_VkImageView(other.m_VkImageView),
		m_VkImageAspectFlags(other.m_VkImageAspectFlags),
		m_CurrentLayout(other.m_CurrentLayout)
	{
		g_VKClearBackBuffer(other.m_VkImage);
		g_VKClearBackBuffer(other.m_VkImageView);
		for (auto& layout : other.m_CurrentLayout)
		{
			layout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}

	VKRenderTarget& VKRenderTarget::operator=(VKRenderTarget&& other) noexcept
	{
		if (this != &other)
		{
			RenderTarget::operator=(std::move(other));
			m_VkFormat = other.m_VkFormat;
			m_VkImage = other.m_VkImage;
			m_VkImageView = other.m_VkImageView;
			m_VkImageAspectFlags = other.m_VkImageAspectFlags;
			m_CurrentLayout = other.m_CurrentLayout;

			g_VKClearBackBuffer(other.m_VkImage);
			g_VKClearBackBuffer(other.m_VkImageView);
			for (auto& layout : other.m_CurrentLayout)
			{
				layout = VK_IMAGE_LAYOUT_UNDEFINED;
			}
		}

		return *this;
	}

	void VKRenderTarget::ResetBindlessDSIndex()
	{
		VKContext& vkContext = VKContext::GetInstance();

		const uint32_t numSCImages = vkContext.m_SCImgCount;
		for (uint32_t i = 0; i < numSCImages; ++i)
		{
			m_BindlessDescriptorSetIndex[i] = Numbers::MAX_UINT32;
		}
	}

	VKRenderTargetEmpty::VKRenderTargetEmpty(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		VKRenderTarget(componentType, dataType, precision, size)
	{
		GM_CORE_ASSERT(size.x > 0 && size.y > 0, "Size needs to be at least 1x1 pixels.");

		VKContext& vkContext = VKContext::GetInstance();
		VkDevice device = vkContext.device.device();
		const uint32_t numSCImages = vkContext.m_SCImgCount;

		m_VkFormat = ToNativeFormat(componentType, dataType, precision);

		VkImageCreateInfo vkImageInfo{};
		vkImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		vkImageInfo.imageType = VK_IMAGE_TYPE_2D;
		vkImageInfo.extent.width = size.x;
		vkImageInfo.extent.height = size.y;
		vkImageInfo.extent.depth = 1;
		vkImageInfo.mipLevels = 1;
		vkImageInfo.arrayLayers = 1;
		vkImageInfo.format = m_VkFormat;
		vkImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		vkImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		vkImageInfo.usage = ToNativeImageUsageFlags(componentType); // Want to be able to sample from all RT in case we need it
		vkImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		vkImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		for (uint32_t i = 0; i < numSCImages; ++i)
		{
			VkImage& currentVkImage = m_VkImage[i];

			GM_CORE_ASSERT(vkCreateImage(VKContext::GetInstance().device.device(), &vkImageInfo, nullptr, &currentVkImage) == VK_SUCCESS, "Failed to create Vulkan image");

			VkMemoryRequirements vkImageMemReq;
			vkGetImageMemoryRequirements(device, currentVkImage, &vkImageMemReq);

			VkMemoryAllocateInfo vkImageMemyAllocInfo{};
			vkImageMemyAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			vkImageMemyAllocInfo.allocationSize = vkImageMemReq.size;
			vkImageMemyAllocInfo.memoryTypeIndex = VKContext::GetInstance().findMemoryType(vkImageMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			GM_CORE_ASSERT(vkAllocateMemory(device, &vkImageMemyAllocInfo, nullptr, &m_VkImageMemory[i]) == VK_SUCCESS, "Failed to allocate Vulkan image memory on device");

			vkBindImageMemory(device, currentVkImage, m_VkImageMemory[i], 0);
		
			// Depth Image View
			m_VkImageAspectFlags = ToNativeImageAspectFlag(componentType);
			VkImageViewCreateInfo vkImageViewInfo{};
			vkImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			vkImageViewInfo.image = currentVkImage;
			vkImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			vkImageViewInfo.format = m_VkFormat;
			vkImageViewInfo.subresourceRange.aspectMask = m_VkImageAspectFlags;
			vkImageViewInfo.subresourceRange.baseMipLevel = 0;
			vkImageViewInfo.subresourceRange.levelCount = 1;
			vkImageViewInfo.subresourceRange.baseArrayLayer = 0;
			vkImageViewInfo.subresourceRange.layerCount = 1;

			GM_CORE_ASSERT(vkCreateImageView(device, &vkImageViewInfo, nullptr, &m_VkImageView[i]) == VK_SUCCESS, "Failed to create Vulkan image view");

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

			GM_CORE_ASSERT(vkCreateSampler(device, &s, nullptr, &m_VkSampler[i]) == VK_SUCCESS, "Failed to create Vulkan sampler");

		}
		
		vkContext.RegisterBindlessVKRenderTarget(*this);
	}

	VKRenderTargetEmpty::~VKRenderTargetEmpty()
	{
		VKContext& vkContext = VKContext::GetInstance();
		VkDevice device = vkContext.device.device();
		const uint32_t numSCImages = numSCImages;

		for (uint32_t i = 0; i < numSCImages; ++i)
		{
			if (m_VkImageMemory[i] != VK_NULL_HANDLE)
			{
				vkFreeMemory(device, m_VkImageMemory[i], nullptr);
			}
		}
	}

	VKRenderTargetEmpty::VKRenderTargetEmpty(VKRenderTargetEmpty&& other) noexcept
		: VKRenderTarget(std::move(other)),
		m_VkImageMemory(other.m_VkImageMemory),
		m_VkCurrentAccessMask(other.m_VkCurrentAccessMask),
		m_VkCurrentStageFlags(other.m_VkCurrentStageFlags)
	{
		g_VKClearBackBuffer(other.m_VkImageMemory);
	}

	VKRenderTargetEmpty& VKRenderTargetEmpty::operator=(VKRenderTargetEmpty&& other) noexcept
	{
		if (this != &other)
		{
			VKRenderTarget::operator=(std::move(other));
			m_VkImageMemory = other.m_VkImageMemory;
			m_VkCurrentAccessMask = other.m_VkCurrentAccessMask;
			m_VkCurrentStageFlags = other.m_VkCurrentStageFlags;

			g_VKClearBackBuffer(other.m_VkImageMemory);
		}

		return *this;
	}

	void VKRenderTargetEmpty::Clear(
		unsigned int mipLayer,
		unsigned int destX,
		unsigned int destY,
		unsigned int destDepth,
		unsigned int extendX,
		unsigned int extendY,
		unsigned int extendDepth,
		const float* normalizedClearColor)
	{
	}

	void VKRenderTargetEmpty::SetParameter(RenderTargetTypes::ParameterKey key, RenderTargetTypes::ParameterValue value)
	{
	}

	VKSinglesampleRenderTarget::VKSinglesampleRenderTarget(RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		VKRenderTargetEmpty(componentType, dataType, precision, size)
	{
	}

	VKMultisampleRenderTarget::VKMultisampleRenderTarget(unsigned int sampleCount, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		VKRenderTargetEmpty(componentType, dataType, precision, size)
	{
		GM_CORE_ASSERT(sampleCount >= 2 && sampleCount <= 16, "Number of samples needs to be between 2 and 16.");
	}

	VKCubeMapArrayRenderTarget::VKCubeMapArrayRenderTarget(unsigned int numTextures, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		VKRenderTargetEmpty(componentType, dataType, precision, size)
	{
	}

	VKRenderTargetImageWrapper::VKRenderTargetImageWrapper(VKBackBuffer<VkImage> image, VKBackBuffer<VkImageView> imageView, VkFormat format, VkImageAspectFlags imageAspectFlags, RenderTargetTypes::ComponentType componentType, RenderTargetTypes::ChannelDataType dataType, RenderTargetTypes::ChannelPrecision precision, glm::uvec2 size) :
		VKRenderTarget(componentType, dataType, precision, size)
	{
		m_VkImage = image;
		m_VkImageView = imageView;
		m_VkFormat = format;
		m_VkImageAspectFlags = imageAspectFlags;
	}
}