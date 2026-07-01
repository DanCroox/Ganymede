#pragma once

#include "Ganymede/Graphics/VertexObject.h"
#include "VKDataBuffer.h"
#include <volk.h>

namespace Ganymede
{
	class GANYMEDE_API VKVertexObject : public VertexObject
	{
	public:
		VKVertexObject(const unsigned int* indicesData, unsigned int numIndices);
		~VKVertexObject() override = default;

		VKVertexObject(VKVertexObject&& other) noexcept = default;
		VKVertexObject& operator=(VKVertexObject&& other) noexcept = default;

		bool IsValid() const override { return true; }

		void LinkBuffer(DataBufferBase& dataBuffer, bool isMultiInstanceDataBuffer = false) override;
		void LinkAndOwnBuffer(std::unique_ptr<DataBufferBase> dataBufferPtr, bool isMultiInstanceDataBuffer) override;

		std::unique_ptr<VKDataBuffer<UInt32VertexData>> m_IndexBuffer;
		VkPipelineVertexInputStateCreateInfo m_VertexInput{};
		VkPipelineInputAssemblyStateCreateInfo m_InputAssembly{};
		std::vector<DataBufferBase*> m_LinkedBuffers;

		static VkFormat ToNativeFormat(VertexDataPrimitiveType primitiveType, unsigned int numComponents);

	private:
		std::vector<VkBuffer> m_VKVertexBuffers;
		std::vector<VkVertexInputBindingDescription> m_BindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> m_AttributeDescriptions;

		uint32_t m_NextFreeBindingLocation = 0;
	};
}