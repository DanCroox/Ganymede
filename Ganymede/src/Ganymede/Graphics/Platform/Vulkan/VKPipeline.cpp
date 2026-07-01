#include "VKPipeline.h"

#include "Ganymede/Graphics/Material.h"
#include "Ganymede/Graphics/ShaderBinary.h"
#include "VKFrameBuffer.h"
#include "VKRenderTarget.h"
#include "VKContext.h"
#include "VKVertexObject.h"
#include <optional>

namespace Ganymede
{
	namespace
	{
		void InitInternal(const ShaderBinary& shaderBinary,
			const VKFrameBuffer& frameBuffer,
			const std::vector<uint32_t>& ssboBindingPoints,
			VkPipelineVertexInputStateCreateInfo vertexInput,
			VkPipeline& vkPipelineOut,
			VkPipelineLayout& vkPipelineLayoutOut)
		{
			VKContext& vkContext = VKContext::GetInstance();

			auto createShaderModule = [&](const std::vector<uint8_t>& code)
				{
					VkShaderModuleCreateInfo info{};
					info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
					info.codeSize = code.size();
					info.pCode = reinterpret_cast<const uint32_t*>(code.data());
					VkShaderModule module;
					if (vkCreateShaderModule(vkContext.device.device(), &info, nullptr, &module) != VK_SUCCESS)
						throw std::runtime_error("failed to create shader module!");
					return module;
				};

			VkShaderModule vertModule = createShaderModule(shaderBinary.m_BinaryContainer[0].m_Data);
			VkShaderModule fragModule = createShaderModule(shaderBinary.m_BinaryContainer[1].m_Data);

			VkPipelineShaderStageCreateInfo vertStage{};
			vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertStage.module = vertModule;
			vertStage.pName = "main";

			VkPipelineShaderStageCreateInfo fragStage{};
			fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragStage.module = fragModule;
			fragStage.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertStage, fragStage };

			// flips the viewport on y
			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = (float)vkContext.extent.height; // Start unten
			viewport.width = (float)vkContext.extent.width;
			viewport.height = -(float)vkContext.extent.height; // NEGATIV, flip Y!
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor{};
			scissor.extent = vkContext.extent;

			VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			VkPipelineRasterizationStateCreateInfo raster{};
			raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			raster.polygonMode = VK_POLYGON_MODE_FILL;
			raster.cullMode = VK_CULL_MODE_BACK_BIT;
			raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			raster.lineWidth = 1.0f;

			VkPipelineMultisampleStateCreateInfo multisample{};
			multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.attachmentCount = frameBuffer.GetColorBlendAttachments().size();
			colorBlending.pAttachments = frameBuffer.GetColorBlendAttachments().data();

			VkPushConstantRange pushRange{};
			pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			pushRange.offset = 0;
			pushRange.size = sizeof(glm::uint) * 4;

			VkPipelineLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			std::array<VkDescriptorSetLayout, 1> pipelineDescSetLayouts;
			pipelineDescSetLayouts = { vkContext.m_BindlessDescriptorSetLayout };
			layoutInfo.setLayoutCount = 1;
			layoutInfo.pSetLayouts = pipelineDescSetLayouts.data();
			layoutInfo.pushConstantRangeCount = 1;
			layoutInfo.pPushConstantRanges = &pushRange;

			if (vkCreatePipelineLayout(vkContext.device.device(), &layoutInfo, nullptr, &vkPipelineLayoutOut) != VK_SUCCESS)
				throw std::runtime_error("failed to create pipeline layout!");

			// Our pipelines only work with triangle lists for now
			VkPipelineInputAssemblyStateCreateInfo m_InputAssembly{};
			m_InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			m_InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Triangle-List

			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;
			pipelineInfo.pVertexInputState = &vertexInput;
			pipelineInfo.pInputAssemblyState = &m_InputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &raster;
			pipelineInfo.pMultisampleState = &multisample;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.layout = vkPipelineLayoutOut;
			pipelineInfo.renderPass = frameBuffer.GetVkRenderPass();
			pipelineInfo.subpass = 0;

			// Depth-Stencil State (Depth Test aktivieren)
			VkPipelineDepthStencilStateCreateInfo depthStencil{};
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.stencilTestEnable = VK_FALSE;
			pipelineInfo.pDepthStencilState = &depthStencil;
			if (frameBuffer.HasDepthAttachment())
			{
				depthStencil.depthTestEnable = VK_TRUE;
				depthStencil.depthWriteEnable = VK_TRUE;
			}
			else
			{
				depthStencil.depthTestEnable = VK_FALSE;
				depthStencil.depthWriteEnable = VK_FALSE;
			}

			if (vkCreateGraphicsPipelines(vkContext.device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipelineOut) != VK_SUCCESS)
				throw std::runtime_error("failed to create graphics pipeline!");
		}

		void Init(const ShaderBinary& shaderBinary,
			const VKFrameBuffer& frameBuffer,
			const std::vector<uint32_t>& ssboBindingPoints,
			uint32_t vertexInputDataStride,
			const std::vector<VertexDataPrimitiveTypeInfo>& vertexDataPrimitiveTypeInfos,
			VkPipeline& vkPipelineOut,
			VkPipelineLayout& vkPipelineLayoutOut)
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = vertexInputDataStride;
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // isMultiInstanceDataBuffer ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;

			std::vector<VkVertexInputAttributeDescription> m_AttributeDescriptions;
			uint32_t m_NextFreeBindingLocation = 0;
			for (const VertexDataPrimitiveTypeInfo& typeInfo : vertexDataPrimitiveTypeInfos)
			{
				VkVertexInputAttributeDescription& attribDesc = m_AttributeDescriptions.emplace_back();
				attribDesc.binding = 0;
				attribDesc.location = m_NextFreeBindingLocation++;
				attribDesc.format = VKVertexObject::ToNativeFormat(typeInfo.m_PrimitiveType, typeInfo.m_NumComponents);
				attribDesc.offset = typeInfo.m_ByteOffset;
			}

			VkPipelineVertexInputStateCreateInfo vertexInput{};
			vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInput.vertexBindingDescriptionCount = 1;
			vertexInput.pVertexBindingDescriptions = &bindingDescription;
			vertexInput.vertexAttributeDescriptionCount = m_AttributeDescriptions.size();
			vertexInput.pVertexAttributeDescriptions = m_AttributeDescriptions.data();

			InitInternal(shaderBinary, frameBuffer, ssboBindingPoints, vertexInput, vkPipelineOut, vkPipelineLayoutOut);
		}

		void Init(const ShaderBinary& shaderBinary,
			const VKFrameBuffer& frameBuffer,
			const std::vector<uint32_t>& ssboBindingPoints,
			VkPipeline& vkPipelineOut,
			VkPipelineLayout& vkPipelineLayoutOut)
		{
			VkPipelineVertexInputStateCreateInfo vertexInput{};
			vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInput.vertexBindingDescriptionCount = 0;
			vertexInput.pVertexBindingDescriptions = nullptr;
			vertexInput.vertexAttributeDescriptionCount = 0;
			vertexInput.pVertexAttributeDescriptions = nullptr;

			InitInternal(shaderBinary, frameBuffer, ssboBindingPoints, vertexInput, vkPipelineOut, vkPipelineLayoutOut);
		}
	}

	VKPipeline::VKPipeline(const ShaderBinary& shaderBinary,
		uint32_t vertexInputDataStride,
		const std::vector<VertexDataPrimitiveTypeInfo>& vertexDataPrimitiveTypeInfos,
		const VKFrameBuffer& frameBuffer,
		const std::vector<uint32_t>& ssboBindingPoints) :
		Pipeline(shaderBinary,
			vertexInputDataStride,
			vertexDataPrimitiveTypeInfos,
			static_cast<const FrameBuffer&>(frameBuffer),
			ssboBindingPoints)
	{
		Init(shaderBinary,
			static_cast<const VKFrameBuffer&>(frameBuffer),
			ssboBindingPoints,
			vertexInputDataStride,
			vertexDataPrimitiveTypeInfos,
			m_VkPipeline, m_VkPipelineLayout);
	}

	VKPipeline::VKPipeline(const ShaderBinary& shaderBinary,
		const VKFrameBuffer& frameBuffer,
		const std::vector<uint32_t>& ssboBindingPoints) :
		Pipeline(shaderBinary,
			static_cast<const FrameBuffer&>(frameBuffer),
			ssboBindingPoints)
	{
		Init(shaderBinary,
			static_cast<const VKFrameBuffer&>(m_FrameBuffer),
			ssboBindingPoints,
			m_VkPipeline, m_VkPipelineLayout);
	}
}