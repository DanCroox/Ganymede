#include "CollectGeometryPass.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/ECS/Components/GCDynamicMobility.h"
#include "Ganymede/ECS/Components/GCEntityID.h"
#include "Ganymede/ECS/Components/GCMesh.h"
#include "Ganymede/ECS/Components/GCPointlight.h"
#include "Ganymede/ECS/Components/GCRigidBody.h"
#include "Ganymede/ECS/Components/GCSkeletal.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/ECS/Components/GCRenderObject.h"
#include "Ganymede/Graphics/SSBO.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/World/World.h"
#include "Ganymede/Graphics/RenderContext.h"

namespace Ganymede
{
	bool CollectGeometryPass::Initialize(RenderContext& renderContext)
	{
		m_CubemapShadowMappingInstanceDataSSBO = renderContext.CreateSSBO("CubemapShadowMappingInstanceData", 5, 1000000 * sizeof(InstanceDataGBuffer), true);

		m_AnimationDataSSBO = renderContext.CreateSSBO("AnimationData", 2, 1 * sizeof(glm::mat4), true);

		m_PointLightProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.f, 0.01f, 1000.0f);

		renderContext.m_VertexObjectStorage.resize(100000);
		renderContext.m_InstanceIDToGBufferInstanceDataIndexLookup.resize(1000000);
		renderContext.m_InstanceIDToCubemapShadowMappingInstanceDataIndexLookup.resize(1000000);
		for (int i = 0; i < 1000000; ++i)
		{
			renderContext.m_InstanceIDToGBufferInstanceDataIndexLookup[i] = -1;
			renderContext.m_InstanceIDToCubemapShadowMappingInstanceDataIndexLookup[i] = -1;
		}

		return true;
	}

	void CollectGeometryPass::Execute(RenderContext& renderContext)
	{
		SCOPED_TIMER("CollectGeometryPass");

		std::vector<std::optional<VertexObject>>& voStorage = renderContext.m_VertexObjectStorage;
		//const FPSCamera& camera = renderContext.GetCamera();
		RenderView rv;
		const FPSCamera camera(rv);
		RenderCommandQueue& cubemapCommandQueue = renderContext.m_CubemapShadowMappingCommandQueue;
		cubemapCommandQueue.clear();

		const auto pointlightsView = renderContext.GetWorld().GetEntities(Include<GCPointlight, GCTransform>{});
		const unsigned int numPointlights = std::distance(pointlightsView.begin(), pointlightsView.end());

		std::vector<glm::mat4> pointlightVPTransforms;
		pointlightVPTransforms.resize(numPointlights * 6);
		unsigned int currentLightID = 0;
		for (auto [entity, pl, plTransform] : pointlightsView.each())
		{
			const glm::vec3 plWorldPosition = plTransform.GetPosition();
			const unsigned int vpIndex = currentLightID * 6;
			pointlightVPTransforms[vpIndex] = m_PointLightProjectionMatrix * glm::lookAt(plWorldPosition, plWorldPosition + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
			pointlightVPTransforms[vpIndex + 1] = m_PointLightProjectionMatrix * glm::lookAt(plWorldPosition, plWorldPosition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
			pointlightVPTransforms[vpIndex + 2] = m_PointLightProjectionMatrix * glm::lookAt(plWorldPosition, plWorldPosition + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
			pointlightVPTransforms[vpIndex + 3] = m_PointLightProjectionMatrix * glm::lookAt(plWorldPosition, plWorldPosition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
			pointlightVPTransforms[vpIndex + 4] = m_PointLightProjectionMatrix * glm::lookAt(plWorldPosition, plWorldPosition + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
			pointlightVPTransforms[vpIndex + 5] = m_PointLightProjectionMatrix * glm::lookAt(plWorldPosition, plWorldPosition + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
			++currentLightID;
		}

		unsigned int animationDataOffset = 0;

		const auto entityView = renderContext.GetWorld().GetEntities(Include<GCMesh, GCTransform, GCEntityID, GCRenderObject>{});
		const unsigned int numMeshes = std::distance(entityView.begin(), entityView.end());
		NUMBERED_NAMED_COUNTER("Instances Total", numMeshes);
		for (auto [entity, gcMesh, gcTransform, entityID, gcRenderObject] : entityView.each())
		{
			unsigned int currentAnimationDataOffset = 0;
			
			if (std::optional<GCSkeletal> gcSkeletal = renderContext.GetWorld().GetComponentFromEntity<GCSkeletal>(entity))
			{
				//const std::vector<glm::mat4>& animationFrame = gcSkeletal.value().m_AnimationBoneData;
				//m_AnimationDataSSBO->Write(animationDataOffset * sizeof(glm::mat4), animationFrame.size() * sizeof(glm::mat4), (void*)&animationFrame[0]);
				//currentAnimationDataOffset = animationDataOffset;
				//animationDataOffset += animationFrame.size();
			}

			glm::mat4 transform;

			if (renderContext.GetWorld().HasComponents<GCRigidBody>(entity) && renderContext.GetWorld().HasComponents<GCDynamicMobility>(entity))
			{
				std::optional<GCRigidBody> gcRigidBody = renderContext.GetWorld().GetComponentFromEntity<GCRigidBody>(entity);
				transform = gcRigidBody.value().m_RigidBody.GetWorldTransform();
			}
			else
			{
				transform = gcTransform.GetMatrix();
			}

			//// BEGIN upload instancedata for CubemapShadowmapping pass
			unsigned int pointlightIndex = 0;

			cubemapInstanceDataIndices.clear();
			{
				for (auto [entity, pl, plTransform] : pointlightsView.each())
				{
					for (unsigned int layerIndex = 0; layerIndex < 6; layerIndex++)
					{
						unsigned int instanceID = (entityID.m_ID * numPointlights * 6);
						instanceID += ((pointlightIndex * 6) + layerIndex);

						InstanceDataCubemapShadowMapping iCubemapData;
						iCubemapData.m_M = transform;
						iCubemapData.m_Attribs = { pointlightIndex, layerIndex, currentAnimationDataOffset, 0 };
						iCubemapData.m_MVP = pointlightVPTransforms[(pointlightIndex * 6) + layerIndex] * iCubemapData.m_M;

						std::int32_t cubemapSSBOIndex = renderContext.m_InstanceIDToCubemapShadowMappingInstanceDataIndexLookup[instanceID];
						if (cubemapSSBOIndex == -1)
						{
							cubemapSSBOIndex = renderContext.m_NextFreeCubemapSSBOInstanceDataIndex++;
							renderContext.m_InstanceIDToCubemapShadowMappingInstanceDataIndexLookup[instanceID] = cubemapSSBOIndex;
						}
						m_CubemapShadowMappingInstanceDataSSBO->Write(cubemapSSBOIndex * sizeof(InstanceDataCubemapShadowMapping), sizeof(InstanceDataCubemapShadowMapping), &iCubemapData);
						cubemapInstanceDataIndices.push_back({ cubemapSSBOIndex, iCubemapData.m_MVP });
					}

					++pointlightIndex;
				}
			}
			//// END upload instancedata for GBuffer pass

			unsigned int meshIdx = 0;
			//for (auto& vo : gcRenderObject.m_MeshData)
			//{
			//	for (const Visibility& vis : cubemapInstanceDataIndices)
			//	{
			//		RenderCommand& command = cubemapCommandQueue.emplace_back();
			//		command.m_VO = vo.m_VO.get();
			//		command.m_SSBOInstanceID = vis.m_InstanceID;
			//	}
			//}
		}

		{
			SCOPED_TIMER("Sorting");
			std::sort(cubemapCommandQueue.begin(), cubemapCommandQueue.end(), [](const RenderCommand& a, const RenderCommand& b)
				{
					return a.m_VO < b.m_VO;
				}
			);
		}
	}
}


