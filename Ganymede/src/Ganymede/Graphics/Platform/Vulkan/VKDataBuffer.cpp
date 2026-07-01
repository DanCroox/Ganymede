#include "VKDataBuffer.h"
#include "VKContext.h";

namespace Ganymede
{
	namespace VKDataBuffer_Private
	{
		VkDevice GetDevice()
		{
			return  VKContext::GetInstance().device.device();
		}

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
		{
			VKContext::GetInstance().createBuffer(size, usage, properties, buffer, bufferMemory);
		}

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
		{
			VKContext::GetInstance().copyBuffer(srcBuffer, dstBuffer, size);
		}

		void DeleteBuffer(VkBuffer buffer, VkDeviceMemory memory)
		{
			VKContext& vkContext = VKContext::GetInstance();
			vkDestroyBuffer(vkContext.device.device(), buffer, nullptr);
			vkFreeMemory(vkContext.device.device(), memory, nullptr);
		}
	}
}