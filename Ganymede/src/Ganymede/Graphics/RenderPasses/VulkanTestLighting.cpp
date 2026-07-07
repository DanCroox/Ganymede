#include "VulkanTestLighting.h"

#include "Ganymede/Graphics/CommandList.h"
#include "Ganymede/Graphics/Platform/GraphicsFactory.h"
#include "Ganymede/Graphics/Pipeline.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/ShaderBinary.h"
#include "Ganymede/Data/StaticData.h"

// TODO Propreitary vullkan includes - remove later
#include "Ganymede/Graphics/Platform/Vulkan/VKContext.h"
#include "Ganymede/Graphics/Platform/Vulkan/VKRenderTarget.h"
#include "Ganymede/Graphics/Platform/Vulkan/VKSSBO.h"
#include "Ganymede/Graphics/Platform/Vulkan/VKGPUTexture.h"

namespace Ganymede
{
	namespace
	{
		struct alignas(16) CompositeMaterialData
		{
			struct Data
			{
				glm::uint m_PositionIdx;
				glm::uint m_NormalIdx;
				glm::uint m_AlbedoIdx;
				glm::uint m_RoughnessMetalIdx;

				glm::uint m_EmissionIdx;
				glm::uint m_ComplexFrgIdx;
				glm::uint m_DepthIdx;
				glm::uint _PadA;
			};

			union
			{
				Data data;
				std::byte padding[256];
			};
		};

		std::unique_ptr<Pipeline> compoPipeline;
		std::vector<PCData> compPCData;
		FreeList matDataIndexFreelist;
		VKSSBO* ssboMaterialData;
	}

	bool VulkanTestLighting::Initialize(RenderContext& renderContext)
	{
		VKContext& vkContext = VKContext::GetInstance();
		const glm::uvec2 extent = { vkContext.extent.width, vkContext.extent.height };

		FrameBufferAttachmentStorage compositefbAttachments{ { FrameBufferAttachmentTypee::Color, 0, vkContext.m_SCImageRenderTargets.get() } };
		const FrameBuffer* compositeFB = renderContext.CreateFrameBuffer("VKCompoPassFB", compositefbAttachments, { extent.x, extent.y });

		std::optional<ShaderBinary> shaderBinary = GraphicsFactory::LoadShader("res/shaders/VulkanComposite.shader");
		const uint32_t shaderBinIdx = StaticData::Instance->m_ShaderBinaries.size();

		StaticData::Instance->m_ShaderBinaries.push_back(std::move(shaderBinary.value()));
		Material& compoMat = StaticData::Instance->m_Materials.emplace_back(shaderBinIdx);

		std::vector<uint32_t> compssbos;
		compoPipeline = GraphicsFactory::CreatePipeline(compoMat.GetShaderBinary().GetData(), *compositeFB, compssbos);


		ssboMaterialData = static_cast<VKSSBO*>(renderContext.CreateSSBO("LightingMaterialData", 0, sizeof(char) * 1000000, false));

		VKRenderTarget* positionRT = static_cast<VKRenderTarget*>(renderContext.GetSingleSampleRenderTarget("positionRT"));
		VKRenderTarget* normalsRT = static_cast<VKRenderTarget*>(renderContext.GetSingleSampleRenderTarget("normalsRT"));
		VKRenderTarget* albedoRT = static_cast<VKRenderTarget*>(renderContext.GetSingleSampleRenderTarget("albedoRT"));
		VKRenderTarget* roughnessMetalRT = static_cast<VKRenderTarget*>(renderContext.GetSingleSampleRenderTarget("roughnessMetalRT"));
		VKRenderTarget* emissionRT = static_cast<VKRenderTarget*>(renderContext.GetSingleSampleRenderTarget("emissionRT"));
		VKRenderTarget* complexFragRT = static_cast<VKRenderTarget*>(renderContext.GetSingleSampleRenderTarget("complexFragRT"));
		VKRenderTarget* depthRT = static_cast<VKRenderTarget*>(renderContext.GetSingleSampleRenderTarget("depthRT"));

		for (uint32_t scImg = 0; scImg < vkContext.m_SCImgCount; ++scImg)
		{
			CompositeMaterialData mdata;
			mdata.data.m_PositionIdx = positionRT->GetBindlessDSIndex()[scImg];
			mdata.data.m_NormalIdx = normalsRT->GetBindlessDSIndex()[scImg];
			mdata.data.m_AlbedoIdx = albedoRT->GetBindlessDSIndex()[scImg];
			mdata.data.m_RoughnessMetalIdx = roughnessMetalRT->GetBindlessDSIndex()[scImg];
			mdata.data.m_EmissionIdx = emissionRT->GetBindlessDSIndex()[scImg];
			mdata.data.m_ComplexFrgIdx = complexFragRT->GetBindlessDSIndex()[scImg];
			mdata.data.m_DepthIdx = depthRT->GetBindlessDSIndex()[scImg];

			const uint32_t matDataIdx = matDataIndexFreelist.Append();
			ssboMaterialData->Write(matDataIdx * sizeof(CompositeMaterialData), sizeof(CompositeMaterialData::Data), &mdata);

			PCData& pcData = compPCData.emplace_back();
			pcData.m_DataIndex = matDataIdx;
			pcData.m_BufferIndex = ssboMaterialData->GetBindlessDSIndex();
		}

		return true;
	}

	void VulkanTestLighting::Execute(RenderContext& renderContext)
	{
		VKContext& vkContext = VKContext::GetInstance();
		CommandList& commandList = *GraphicsFactory::GetCommandList();

		commandList.BindFrameBuffer(compoPipeline->GetFrameBuffer());
		commandList.BindPipeline(*compoPipeline);
		commandList.DrawFullscreenQuad(compPCData[vkContext.m_SCIndex]);

		commandList.End();
	}
}