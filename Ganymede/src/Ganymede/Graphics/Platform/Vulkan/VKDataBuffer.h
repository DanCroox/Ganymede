#pragma once

#include "Ganymede/Graphics/DataBuffer.h"
#include <volk.h>

namespace Ganymede
{
	namespace VKDataBuffer_Private
	{
		VkDevice GetDevice();
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void DeleteBuffer(VkBuffer buffer, VkDeviceMemory memory);
	}

	template <typename T>
	class GANYMEDE_API VKDataBuffer : public DataBuffer<T>
	{
	public:
		VKDataBuffer(typename T::VertexDataType* data, unsigned int numElements, DataBufferType bufferType) :
			DataBuffer<T>(data, numElements, bufferType)
		{
			InitializeBuffer(data, numElements, bufferType);
		}

		~VKDataBuffer() override
		{
			DeleteBuffer();
		}

		void InitializeBuffer(typename T::VertexDataType* data, unsigned int numElements, DataBufferType bufferType)
		{
			m_BufferSize = sizeof(typename T::VertexDataType)* numElements;
			m_BufferType = bufferType;

			VkDeviceSize vbSize = m_BufferSize;
			VKDataBuffer_Private::CreateBuffer(
				vbSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
				VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_VkBuffer,
				m_VkBufferMemory
			);

			VkDevice device = VKDataBuffer_Private::GetDevice();

			// Write data from CPU to HOST_VISIBLE GPU memory
			vkMapMemory(device, m_VkBufferMemory, 0, vbSize, 0, &m_GPUMappedHostVisiblePtr);
			memcpy(m_GPUMappedHostVisiblePtr, data, (size_t)vbSize);

			if (m_BufferType == DataBufferType::Static)
			{
				// Unmap and move data from HOST_VISIBLE GPU memory to fast DEVICE_LOCAL GPU memory
				vkUnmapMemory(device, m_VkBufferMemory);

				VkBuffer vkBufferHostVisible = m_VkBuffer;
				VkDeviceMemory vkBufferMemoryHostVisible = m_VkBufferMemory;
				m_VkBuffer = VK_NULL_HANDLE;
				m_VkBufferMemory = VK_NULL_HANDLE;

				VKDataBuffer_Private::CreateBuffer(
					vbSize,
					VK_BUFFER_USAGE_TRANSFER_DST_BIT |
					VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
					VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					m_VkBuffer,
					m_VkBufferMemory
				);

				VKDataBuffer_Private::CopyBuffer(vkBufferHostVisible, m_VkBuffer, vbSize);

				vkDestroyBuffer(device, vkBufferHostVisible, nullptr);
				vkFreeMemory(device, vkBufferMemoryHostVisible, nullptr);
			}
		}

		void DeleteBuffer()
		{
			VKDataBuffer_Private::DeleteBuffer(m_VkBuffer, m_VkBufferMemory);
		}

		void Write(typename T::VertexDataType* data, unsigned int numElements, unsigned int offset) override
		{
			GM_CORE_ASSERT(m_BufferType == DataBufferType::Dynamic, "Writing data to a statically initialized data buffer is not possible.");
			GM_CORE_ASSERT(offset == 0, "Offset not supported yet");

			const size_t numBytesRequested = (sizeof(typename T::VertexDataType) * numElements);
			GM_CORE_ASSERT(numBytesRequested <= m_BufferSize, "Buffer resizing not supported yet");

			memcpy(static_cast<char*>(m_GPUMappedHostVisiblePtr), data, numBytesRequested);
		}

		VkBuffer m_VkBuffer;
		size_t m_BufferSize;
	private:
		DataBufferType m_BufferType;
		VkDeviceMemory m_VkBufferMemory;
		void* m_GPUMappedHostVisiblePtr; // Only accessible if dynamic buffer
	};
}