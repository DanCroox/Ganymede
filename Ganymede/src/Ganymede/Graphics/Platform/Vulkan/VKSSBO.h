#pragma once

#include "Ganymede/Core/Core.h"
#include "Ganymede/Graphics/SSBO.h"
#include "Ganymede/System/Types.h"
#include <volk.h>

namespace Ganymede
{
	class GANYMEDE_API VKSSBO : public SSBO
	{
	public:
		VKSSBO(unsigned int bindingPointID, unsigned int bufferSize, bool autoResize);
		~VKSSBO() override;

		VKSSBO(VKSSBO&& other) noexcept;
		VKSSBO& operator=(VKSSBO&& other) noexcept;

		void Write(unsigned int offset, unsigned int byteCount, void* data) override;
		void Read(unsigned int offset, unsigned int byteCount, void* dataOut) override;
		bool IsValid() const override { return false; }
		void Barrier() override;

		VkBuffer uniformBuffer = VK_NULL_HANDLE;
		VkDeviceMemory uniformMemory = VK_NULL_HANDLE;
		VkDeviceSize uboSize = 0;

		uint32_t GetBindlessDSIndex() const { return m_BindlessDescriptorSetIndex; }
		bool IsBindlessDSIndexValid() const { return m_BindlessDescriptorSetIndex != Numbers::MAX_UINT32; }

	private:
		friend class VKContext;

		void ResetBindlessDSIndex() { m_BindlessDescriptorSetIndex = Numbers::MAX_UINT32; }

		void* m_DirectAccessBuffer = nullptr;
		uint32_t m_BindlessDescriptorSetIndex = Numbers::MAX_UINT32;
	};
}