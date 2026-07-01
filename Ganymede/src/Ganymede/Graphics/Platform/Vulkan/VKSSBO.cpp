#include "VKSSBO.h"
#include "VKDescriptorSetPoolManager.h"
#include "VKContext.h"

namespace Ganymede
{
	VKSSBO::VKSSBO(unsigned int bindingPointID, unsigned int bufferSize, bool autoResize) :
		SSBO(bindingPointID, bufferSize, autoResize)
	{
		VKContext& vkContext = VKContext::GetInstance();

		uboSize = bufferSize;
		vkContext.createBuffer(
			uboSize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffer,
			uniformMemory
		);

		vkMapMemory(vkContext.device.device(), uniformMemory, 0, uboSize, 0, &m_DirectAccessBuffer);
		vkContext.RegisterBindlessVKSSBO(*this);
	}

	VKSSBO::~VKSSBO()
	{
		VKContext& vkContext = VKContext::GetInstance();

		vkUnmapMemory(vkContext.device.device(), uniformMemory);
		vkDestroyBuffer(vkContext.device.device(), uniformBuffer, nullptr);
		vkFreeMemory(vkContext.device.device(), uniformMemory, nullptr);
	}

	VKSSBO::VKSSBO(VKSSBO&& other) noexcept :
		SSBO(std::move(other)),
		m_DirectAccessBuffer(other.m_DirectAccessBuffer),
		uniformBuffer(other.uniformBuffer),
		uniformMemory(other.uniformMemory),
		uboSize(other.uboSize)
	{
		other.m_DirectAccessBuffer = nullptr;
		uniformBuffer = VK_NULL_HANDLE;
		uniformMemory = VK_NULL_HANDLE;
		uboSize = 0;
	}

	VKSSBO& VKSSBO::operator=(VKSSBO&& other) noexcept
	{
		if (this != &other)
		{
			SSBO::operator=(std::move(other));
			m_DirectAccessBuffer = other.m_DirectAccessBuffer;
			uniformBuffer = other.uniformBuffer;
			uniformMemory = other.uniformMemory;
			uboSize = other.uboSize;

			other.m_DirectAccessBuffer = 0;
			uniformBuffer = VK_NULL_HANDLE;
			uniformMemory = VK_NULL_HANDLE;
			uboSize = 0;
		}

		return *this;
	}

	void VKSSBO::Barrier()
	{
		GM_CORE_ASSERT(false, "Not Implemented");
	}

	void VKSSBO::Write(unsigned int offset, unsigned int byteCount, void* data)
	{
		auto dst = static_cast<char*>(m_DirectAccessBuffer) + offset;
		memcpy(dst, data, byteCount);
	}

	void VKSSBO::Read(unsigned int offset, unsigned int byteCount, void* dataOut)
	{
		memcpy(dataOut, m_DirectAccessBuffer, (size_t)byteCount);
	}
}