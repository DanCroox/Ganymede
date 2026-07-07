#include "VulkanTestRenderPass.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/ShaderBinary.h"

#include "Ganymede/ECS/Components/GCDynamicMobility.h"
#include "Ganymede/ECS/Components/GCGPUEntityData.h"
#include "Ganymede/ECS/Components/GCMesh.h"
#include "Ganymede/ECS/Components/GCName.h"
#include "Ganymede/ECS/Components/GCSkeletal.h"
#include "Ganymede/ECS/Components/GCRigidBody.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/World.h"
#include "Ganymede/Log/Log.h"
#include "Ganymede/Data/StaticData.h"
#include "Ganymede/Graphics/CommandList.h"

#include "Ganymede/Core/Application.h"
#include "Ganymede/Graphics/Platform/GraphicsFactory.h"
#include "Ganymede/Graphics/Pipeline.h"

#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

//TODO: Remove propreitary include later 
#include "Ganymede/Graphics/Platform/Vulkan/VKContext.h"
#include "Ganymede/Graphics/Platform/Vulkan/VKRenderTarget.h"
#include "Ganymede/Graphics/Platform/Vulkan/VKSSBO.h"
#include "Ganymede/Graphics/Platform/Vulkan/VKGPUTexture.h"

namespace Ganymede
{
	namespace
	{
		std::vector<std::unique_ptr<Pipeline>> m_MaterialPipelinePtrs;
		std::unique_ptr<Pipeline> compoPipeline;

		struct alignas(16) GBufferMaterialData
		{
			struct Data
			{
				glm::uint m_AlbedoIdx;
				glm::uint m_NormalIdx;
				glm::uint m_RoughnessIdx;
				glm::uint m_MetallicIdx;

				glm::uint m_EmissionIdx;
				glm::uint _PadA;
				glm::uint _PadB;
				glm::uint _PadC;

				glm::vec4 m_BaseColor;

				glm::vec3 m_EmissiveColor;
				float _PadD;

				float m_Roughness;
				float m_Metalness;
				float _PadE;
				float _PadF;
			};

			union
			{
				Data data;
				std::byte padding[256];
			};
		};
		static_assert(sizeof(GBufferMaterialData) == 256);

		struct GBufferMeshInstanceData
		{
			glm::mat4 m_M;
			glm::mat4 m_V;
			glm::mat4 m_P;

			glm::uint m_MaterialBufferIndex;
			glm::uint m_MaterialDataIndex;
			glm::uint m_AnimDataIndex;
			glm::uint _PadC;
		};

		VKSSBO* ssboMeshInstanzData;
		VKSSBO* ssboMaterialData;
		VKSSBO* ssboAnimationData;

		FreeList matDataIndexFreelist;
	}

	bool VulkanTestRenderPass::Initialize(RenderContext& renderContext)
	{
		VKContext& vkContext = VKContext::GetInstance();
		const glm::uvec2 extent = { vkContext.extent.width, vkContext.extent.height };

		// Init GBUFFER pass
		// Create rendertargets for gbuffer
		VKRenderTarget* positionRT = static_cast<VKRenderTarget*>(renderContext.CreateSingleSampleRenderTarget("positionRT", RenderTargetTypes::ComponentType::RGB, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { extent.x, extent.y }));
		VKRenderTarget* normalsRT = static_cast<VKRenderTarget*>(renderContext.CreateSingleSampleRenderTarget("normalsRT", RenderTargetTypes::ComponentType::RGB, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { extent.x, extent.y }));
		VKRenderTarget* albedoRT = static_cast<VKRenderTarget*>(renderContext.CreateSingleSampleRenderTarget("albedoRT", RenderTargetTypes::ComponentType::RGBA, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { extent.x, extent.y }));
		VKRenderTarget* roughnessMetalRT = static_cast<VKRenderTarget*>(renderContext.CreateSingleSampleRenderTarget("roughnessMetalRT", RenderTargetTypes::ComponentType::RG, RenderTargetTypes::ChannelDataType::UNorm, RenderTargetTypes::ChannelPrecision::B8, { extent.x, extent.y }));
		VKRenderTarget* emissionRT = static_cast<VKRenderTarget*>(renderContext.CreateSingleSampleRenderTarget("emissionRT", RenderTargetTypes::ComponentType::RGB, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { extent.x, extent.y }));
		VKRenderTarget* complexFragRT = static_cast<VKRenderTarget*>(renderContext.CreateSingleSampleRenderTarget("complexFragRT", RenderTargetTypes::ComponentType::R, RenderTargetTypes::ChannelDataType::UInt, RenderTargetTypes::ChannelPrecision::B8, { extent.x, extent.y }));
		VKRenderTarget* depthRT = static_cast<VKRenderTarget*>(renderContext.CreateSingleSampleRenderTarget("depthRT", RenderTargetTypes::ComponentType::Depth, RenderTargetTypes::ChannelDataType::Float, RenderTargetTypes::ChannelPrecision::B32, { extent.x, extent.y }));

		// Create framebuffer with given rendertargets on given attachment slots
		FrameBufferAttachmentStorage fbAttachments{
			{ FrameBufferAttachmentTypee::Color, 0, positionRT},
			{ FrameBufferAttachmentTypee::Color, 1, normalsRT},
			{ FrameBufferAttachmentTypee::Color, 2, albedoRT},
			{ FrameBufferAttachmentTypee::Color, 3, roughnessMetalRT},
			{ FrameBufferAttachmentTypee::Color, 4, emissionRT},
			{ FrameBufferAttachmentTypee::Color, 5, complexFragRT},
			{ FrameBufferAttachmentTypee::Depth, 6, depthRT }};
		const FrameBuffer* geometryFB = renderContext.CreateFrameBuffer("VKGeoPassFB", fbAttachments, { extent.x, extent.y });

		// Create ssbo with per-mesh instance data
		ssboMaterialData = static_cast<VKSSBO*>(renderContext.CreateSSBO("MaterialData", 0, sizeof(char) * 1000000, false));
		ssboMeshInstanzData = static_cast<VKSSBO*>(renderContext.CreateSSBO("MeshInstanzData", 0, sizeof(GBufferMeshInstanceData) * 1000000, false));
		ssboAnimationData = static_cast<VKSSBO*>(renderContext.CreateSSBO("AnimationData", 0, sizeof(glm::mat4) * 1000000, false));

		// Create pipelines for all material - in case we need them, we dont need to lazy-create them
		const std::vector<Material>& staticMaterials = StaticData::Instance->m_Materials;
		m_MaterialPipelinePtrs.resize(staticMaterials.size());
		uint32_t matID = 0; //Binding ID  is the index within the staticMaterials array
		for (const Material& staticMaterial : staticMaterials)
		{
			m_MaterialPipelinePtrs[matID++] = GraphicsFactory::CreatePipeline(
				staticMaterial.GetShaderBinary().GetData(),
				sizeof(MeshVertexData::VertexDataDescriptor::VertexDataType),
				MeshVertexData::GetVertexDataPrimitiveTypeInfo(),
				*geometryFB,
				std::vector{0u}); //We gonna have exactly 1 SSBO on binding location 0
		}

		return true;
	}

	void VulkanTestRenderPass::Execute(RenderContext& renderContext)
	{
		SCOPED_TIMER("Vulkan Test Pass");

		uint32_t meshInstanceDataOffset = 0;
		uint32_t animDataIndex = 0;

		Renderer& renderer = renderContext.GetRenderer();
		RenderView renderView = renderContext.GetRenderView(0);
		VKContext& vkContext = VKContext::GetInstance();
		CommandList& commandList = *GraphicsFactory::GetCommandList();

		commandList.Reset();
		commandList.Begin();

		Pipeline* currentPipeline = nullptr;
		const FrameBuffer* currentFrameBuffer = nullptr;

		const World& world = renderContext.GetWorld();
		auto entitiesToAdd = world.GetEntities(Include<GCMesh, GCTransform, GCName>{});
		for (auto [entity, gcMesh, gcTransform, gcName] : entitiesToAdd.each())
		{
			MeshWorldObject::Mesh* mesh = gcMesh.m_Meshes[0];

			Pipeline& graphicsPipelineInfo = *m_MaterialPipelinePtrs[mesh->m_MaterialHandle.GetID()];
			if (currentPipeline != &graphicsPipelineInfo)
			{
				if (currentFrameBuffer != &graphicsPipelineInfo.GetFrameBuffer())
				{
					commandList.BindFrameBuffer(graphicsPipelineInfo.GetFrameBuffer());
					currentFrameBuffer = &graphicsPipelineInfo.GetFrameBuffer();
				}

				commandList.BindPipeline(graphicsPipelineInfo);
				currentPipeline = &graphicsPipelineInfo;
			}

			static std::unordered_map<uint32_t, uint32_t> matHandleToMatDataOffset;
			if (matHandleToMatDataOffset.find(mesh->m_MaterialHandle.GetID()) == matHandleToMatDataOffset.end())
			{
				GBufferMaterialData m_Data;
				const auto& propertiesMap = mesh->m_MaterialHandle.GetData().GetMaterialProperties();
				bool hasData = false;
				for (const auto& [bindingPoint, property] : propertiesMap)
				{
					const Material::MaterialPropertyData& propertyValue = property.m_Data;
					if (std::holds_alternative<Handle<Texture>>(propertyValue))
					{
						const Handle<Texture>& handle = std::get<Handle<Texture>>(propertyValue);
						const VKGPUTexture* tex = static_cast<const VKGPUTexture*>(&renderContext.GetGPUTexture(handle));
						if (bindingPoint == 0) m_Data.data.m_AlbedoIdx = tex->GetBindlessDSIndex();
						if (bindingPoint == 1) m_Data.data.m_NormalIdx = tex->GetBindlessDSIndex();
						if (bindingPoint == 2) m_Data.data.m_RoughnessIdx = tex->GetBindlessDSIndex();
						if (bindingPoint == 3) m_Data.data.m_MetallicIdx = tex->GetBindlessDSIndex();
						if (bindingPoint == 4) m_Data.data.m_EmissionIdx = tex->GetBindlessDSIndex();
					}
					else if (!hasData && (std::holds_alternative<glm::vec3>(propertyValue) || std::holds_alternative<float>(propertyValue)))
					{
						hasData = true;
						if (auto it = propertiesMap.find(5); it != propertiesMap.end()) m_Data.data.m_BaseColor = glm::vec4(std::get<glm::vec3>(propertiesMap.find(5)->second.m_Data), 1.0f);
						else if (auto it = propertiesMap.find(6); it != propertiesMap.end()) m_Data.data.m_EmissiveColor = std::get<glm::vec3>(propertiesMap.find(6)->second.m_Data);
						else if (auto it = propertiesMap.find(7); it != propertiesMap.end()) m_Data.data.m_Roughness = std::get<float>(propertiesMap.find(7)->second.m_Data);
						else if (auto it = propertiesMap.find(8); it != propertiesMap.end()) m_Data.data.m_Metalness = std::get<float>(propertiesMap.find(8)->second.m_Data);

					}
				}
				const uint32_t matDataIdx = matDataIndexFreelist.Append();
				ssboMaterialData->Write(matDataIdx * sizeof(GBufferMaterialData), sizeof(GBufferMaterialData::Data), &m_Data);
				matHandleToMatDataOffset[mesh->m_MaterialHandle.GetID()] = matDataIdx * sizeof(GBufferMaterialData);
			}

			static std::unordered_map<entt::entity, std::vector<uint32_t>> meshInstanceOffsets;
			auto[it, inserted] = meshInstanceOffsets.try_emplace(entity);
			if (inserted)
			{
				for (uint32_t fifIdx = 0; fifIdx < vkContext.m_NumFIF; ++fifIdx)
				{
					uint32_t& dataOffset = it->second.emplace_back();
					dataOffset = meshInstanceDataOffset;
					meshInstanceDataOffset += sizeof(GBufferMeshInstanceData);
				}
			}
			uint32_t dataOffset = it->second[vkContext.m_FiFIndex]; //Big buffer holding per-mesh instance data

			glm::vec3 p = renderView.GetPosition();
			glm::mat4 view = glm::lookAt(p, p + renderView.GetFrontVector(), renderView.GetUpVector());
			glm::mat4 myMVPMatrix = renderView.GetProjectionMatrix() * view * gcTransform.GetMatrix();

			uint32_t currentAnimDataIndex = 0;
			if (std::optional<GCSkeletal> gcSkeletal = world.GetComponentFromEntity<GCSkeletal>(entity))
			{
				const std::vector<glm::mat4>& animFrameData = gcSkeletal.value().m_AnimationBoneData;
				ssboAnimationData->Write(animDataIndex * sizeof(glm::mat4), animFrameData.size() * sizeof(glm::mat4), (void*)&animFrameData[0]);
				currentAnimDataIndex = animDataIndex;
				animDataIndex += animFrameData.size();
			}

			GBufferMeshInstanceData mid
			{
				gcTransform.GetMatrix(),
				view,
				renderView.GetProjectionMatrix(),
				ssboMaterialData->GetBindlessDSIndex(),
				matHandleToMatDataOffset[mesh->m_MaterialHandle.GetID()] / sizeof(GBufferMaterialData),
				ssboAnimationData->GetBindlessDSIndex(),
				currentAnimDataIndex
			};

			ssboMeshInstanzData->Write(dataOffset, sizeof(GBufferMeshInstanceData), &mid);
			const VertexObject& vertexObject = renderContext.GetVO(*mesh);

			PCData pc;
			pc.m_DataIndex = dataOffset / sizeof(GBufferMeshInstanceData);
			pc.m_BufferIndex = ssboMeshInstanzData->GetBindlessDSIndex();
			commandList.DrawGeometry(vertexObject, pc);
		}

		//commandList.End();
	}
}