#include "Ganymede/Graphics/Platform/Vulkan/VKVertexObject.h"
#include "VKDataBuffer.h"
#include "VKContext.h"

namespace Ganymede
{
	namespace
	{
		static inline constexpr const unsigned int p_MaxNumberVertexAttributes = 16;
	}

	VkFormat VKVertexObject::ToNativeFormat(VertexDataPrimitiveType primitiveType, unsigned int numComponents)
	{
		switch (primitiveType)
		{
		case VertexDataPrimitiveType::Char:
			switch (numComponents)
			{
			case 1: return VK_FORMAT_R8_SNORM;
			case 2: return VK_FORMAT_R8G8_SNORM;
			case 3: return VK_FORMAT_R8G8B8_SNORM;
			case 4: return VK_FORMAT_R8G8B8A8_SNORM;
			}
			break;

		case VertexDataPrimitiveType::UChar:
			switch (numComponents)
			{
			case 1: return VK_FORMAT_R8_UNORM;
			case 2: return VK_FORMAT_R8G8_UNORM;
			case 3: return VK_FORMAT_R8G8B8_UNORM;
			case 4: return VK_FORMAT_R8G8B8A8_UNORM;
			}
			break;

		case VertexDataPrimitiveType::Short:
			switch (numComponents)
			{
			case 1: return VK_FORMAT_R16_SNORM;
			case 2: return VK_FORMAT_R16G16_SNORM;
			case 3: return VK_FORMAT_R16G16B16_SNORM;
			case 4: return VK_FORMAT_R16G16B16A16_SNORM;
			}
			break;

		case VertexDataPrimitiveType::UShort:
			switch (numComponents)
			{
			case 1: return VK_FORMAT_R16_UNORM;
			case 2: return VK_FORMAT_R16G16_UNORM;
			case 3: return VK_FORMAT_R16G16B16_UNORM;
			case 4: return VK_FORMAT_R16G16B16A16_UNORM;
			}
			break;

		case VertexDataPrimitiveType::Int:
			switch (numComponents)
			{
			case 1: return VK_FORMAT_R32_SINT;
			case 2: return VK_FORMAT_R32G32_SINT;
			case 3: return VK_FORMAT_R32G32B32_SINT;
			case 4: return VK_FORMAT_R32G32B32A32_SINT;
			}
			break;

		case VertexDataPrimitiveType::UInt:
			switch (numComponents)
			{
			case 1: return VK_FORMAT_R32_UINT;
			case 2: return VK_FORMAT_R32G32_UINT;
			case 3: return VK_FORMAT_R32G32B32_UINT;
			case 4: return VK_FORMAT_R32G32B32A32_UINT;
			}
			break;

		case VertexDataPrimitiveType::Float:
			switch (numComponents)
			{
			case 1: return VK_FORMAT_R32_SFLOAT;
			case 2: return VK_FORMAT_R32G32_SFLOAT;
			case 3: return VK_FORMAT_R32G32B32_SFLOAT;
			case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
			break;

		case VertexDataPrimitiveType::Double:
			switch (numComponents)
			{
			case 1: return VK_FORMAT_R64_SFLOAT;
			case 2: return VK_FORMAT_R64G64_SFLOAT;
			case 3: return VK_FORMAT_R64G64B64_SFLOAT;
			case 4: return VK_FORMAT_R64G64B64A64_SFLOAT;
			}
			break;
		}

		GM_CORE_ASSERT(false, "");
		return VK_FORMAT_UNDEFINED;
	}

	VKVertexObject::VKVertexObject(const unsigned int* indicesData, unsigned int numIndices) :
		VertexObject(indicesData, numIndices)
	{
		m_InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		m_InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Triangle-List

		m_IndexBuffer = std::make_unique<VKDataBuffer<UInt32VertexData>>((glm::u32vec1*)indicesData, numIndices, DataBufferType::Static);
	}

	void VKVertexObject::LinkBuffer(DataBufferBase& dataBuffer, bool isMultiInstanceDataBuffer)
	{
		GM_CORE_ASSERT(m_NextFreeBindingLocation < p_MaxNumberVertexAttributes, "Maximum number of vertex attribute bindings exceeded");
		uint32_t bindingPoint = m_LinkedBuffers.size();
		
		m_LinkedBuffers.push_back(&dataBuffer);
		VkVertexInputBindingDescription& bindingDescription = m_BindingDescriptions.emplace_back();

		bindingDescription.binding = bindingPoint;
		bindingDescription.stride = dataBuffer.GetElementSize();
		bindingDescription.inputRate = isMultiInstanceDataBuffer ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
		
		const std::vector<VertexDataPrimitiveTypeInfo>& typeInfos = dataBuffer.GetVertexDataPrimitiveTypeInfo();

		for (const VertexDataPrimitiveTypeInfo& typeInfo : typeInfos)
		{
			VkVertexInputAttributeDescription& attribDesc = m_AttributeDescriptions.emplace_back();
			attribDesc.binding = bindingPoint;
			attribDesc.location = m_NextFreeBindingLocation++;
			attribDesc.format = ToNativeFormat(typeInfo.m_PrimitiveType, typeInfo.m_NumComponents);
			attribDesc.offset = typeInfo.m_ByteOffset;
		}

		m_VertexInput = {};
		m_VertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		m_VertexInput.vertexBindingDescriptionCount = m_BindingDescriptions.size();
		m_VertexInput.pVertexBindingDescriptions = m_BindingDescriptions.data();
		m_VertexInput.vertexAttributeDescriptionCount = m_AttributeDescriptions.size();
		m_VertexInput.pVertexAttributeDescriptions = m_AttributeDescriptions.data();
	}

	void VKVertexObject::LinkAndOwnBuffer(std::unique_ptr<DataBufferBase> dataBufferPtr, bool isMultiInstanceDataBuffer)
	{
		LinkBuffer(*dataBufferPtr.release(), isMultiInstanceDataBuffer);
	}
}