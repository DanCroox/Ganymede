#pragma once

#include "Ganymede/Graphics/GPUTexture.h"
#include "Ganymede/System/Types.h"
#include <volk.h>

namespace Ganymede
{
	class GANYMEDE_API VKGPUTexture : public GPUTexture
	{
	public:
		explicit VKGPUTexture(const Texture& texHandle);
		~VKGPUTexture() override;

		VKGPUTexture(VKGPUTexture&&) noexcept;
		VKGPUTexture& operator=(VKGPUTexture&&) noexcept;

		void Bind(unsigned int slot = 0) const override;
		void Unbind() const override;

		VkSampler textureSampler = VK_NULL_HANDLE;
		VkImageView vkImageView = VK_NULL_HANDLE;

		uint32_t GetBindlessDSIndex() const { return m_BindlessDescriptorSetIndex; }
		bool IsBindlessDSIndexValid() const { return m_BindlessDescriptorSetIndex != Numbers::MAX_UINT32; }

	private:
		friend class VKContext;

		void ResetBindlessDSIndex() { m_BindlessDescriptorSetIndex = Numbers::MAX_UINT32; }

		VkImage vkImage = VK_NULL_HANDLE;
		VkDeviceMemory vkMemory = VK_NULL_HANDLE;
		uint32_t m_BindlessDescriptorSetIndex = Numbers::MAX_UINT32;
	};
}