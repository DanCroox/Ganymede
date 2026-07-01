#pragma once

#include "Ganymede/Graphics/Pipeline.h"
#include "Ganymede/Graphics/VertexDataTypes.h"
#include <optional>
#include <vector>
#include <volk.h>

namespace Ganymede
{
	class VKFrameBuffer;

	class VKPipeline : public Pipeline
	{
	public:
		VKPipeline(const ShaderBinary& shaderBinary,
			uint32_t vertexInputDataStride,
			const std::vector<VertexDataPrimitiveTypeInfo>& vertexDataPrimitiveTypeInfos,
			const VKFrameBuffer& frameBuffer,
			const std::vector<uint32_t>& ssboBindingPoints);

		VKPipeline(const ShaderBinary& shaderBinary,
			const VKFrameBuffer& frameBuffer,
			const std::vector<uint32_t>& ssboBindingPoints);

		VkPipeline GetVkPipeline() const { return m_VkPipeline; }
		VkPipelineLayout GetVkPipelineLayout() const { return m_VkPipelineLayout; }

	private:
		VkPipeline m_VkPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_VkPipelineLayout = VK_NULL_HANDLE;
	};
}