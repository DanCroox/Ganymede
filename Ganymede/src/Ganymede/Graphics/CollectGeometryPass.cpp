#include "CollectGeometryPass.h"

#include "RenderContext.h"
#include "Ganymede/World/MeshWorldObjectInstance.h"
#include "Ganymede/World/PointlightWorldObjectInstance.h"
#include "Ganymede/World/World.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/Common/Helpers.h"


namespace Ganymede
{
	namespace CollectGeometryPass_Private
	{
		bool IsInFrustum(const glm::mat4& mvp, const MeshWorldObject::Mesh& mesh)
		{
			int clipSides[6] = {0};
			for (const MeshWorldObject::Mesh::BoundingBoxVertex& bbVert : mesh.m_BoundingBoxVertices)
			{
				glm::vec4 clipPoint = mvp * glm::vec4(bbVert.m_Position, 1);

				clipSides[0] += clipPoint.x < -clipPoint.w; //left of Left plane
				clipSides[1] += clipPoint.x > clipPoint.w;  //right of Right plane
				clipSides[2] += clipPoint.y < -clipPoint.w; //below Bottom plane
				clipSides[3] += clipPoint.y > clipPoint.w;  //above Top plane
				clipSides[4] += clipPoint.z < -clipPoint.w; //in front of Near plane
				clipSides[5] += clipPoint.z > clipPoint.w;  //behind Far plane
			}

			const bool isOutSideFrustum = clipSides[0] == 8 || clipSides[1] == 8 || clipSides[2] == 8 ||
				clipSides[3] == 8 || clipSides[4] == 8 || clipSides[5] == 8;

			return !isOutSideFrustum;
		}
	}

	bool CollectGeometryPass::Initialize(RenderContext& renderContext)
	{
		m_InstanceDataIndexBuffer = renderContext.CreateDataBuffer<UInt32VertexData>("InstanceDataIndexBuffer", nullptr, 1, DataBufferType::Dynamic);

		m_GBufferInstanceDataSSBO = renderContext.CreateSSBO("GBufferInstanceData", 3, 1 * sizeof(InstanceDataGBuffer), true);
		m_CubemapShadowMappingInstanceDataSSBO = renderContext.CreateSSBO("CubemapShadowMappingInstanceData", 5, 1 * sizeof(InstanceDataGBuffer), true);

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

		auto instances = renderContext.GetWorld().GetWorldObjectInstances<MeshWorldObjectInstance>();
		std::vector<std::optional<VertexObject>>& voStorage = renderContext.m_VertexObjectStorage;
		const FPSCamera& camera = renderContext.GetCamera();
		RenderCommandQueue& commandQueue = renderContext.m_GBufferCommandQueue;
		RenderCommandQueue& cubemapCommandQueue = renderContext.m_CubemapShadowMappingCommandQueue;
		commandQueue.clear();
		cubemapCommandQueue.clear();

		auto pointlights = renderContext.GetWorld().GetWorldObjectInstances<PointlightWorldObjectInstance>();
		std::vector<glm::mat4> pointlightVPTransforms;
		pointlightVPTransforms.resize(pointlights.size() * 6);
		unsigned int currentLightID = 0;
		for (auto pl : pointlights)
		{
			const glm::vec3 plWorldPosition = pl->GetPosition();
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
		for (auto instance : instances)
		{
			unsigned int currentAnimationDataOffset = 0;
			
			if (SkeletalMeshWorldObjectInstance* skeletalMesh = dynamic_cast<SkeletalMeshWorldObjectInstance*>(instance))
			{
				const std::vector<glm::mat4>& animationFrame = skeletalMesh->GetAnimationBoneData();
				m_AnimationDataSSBO->Write(animationDataOffset * sizeof(glm::mat4), animationFrame.size() * sizeof(glm::mat4), (void*)&animationFrame[0]);
				currentAnimationDataOffset = animationDataOffset;
				animationDataOffset += animationFrame.size();
			}

			//// BEGIN upload instancedata for GBuffer pass				
			InstanceDataGBuffer iData;
			iData.m_M = instance->GetTransform();
			iData.m_MV = camera.GetTransform() * instance->GetTransform();
			iData.m_AnimationDataOffset.x = currentAnimationDataOffset;

			std::int32_t ssboIndex = renderContext.m_InstanceIDToGBufferInstanceDataIndexLookup[instance->GetInstanceID()];
			if (ssboIndex == -1)
			{
				ssboIndex = renderContext.m_NextFreeGBufferSSBOInstanceDataIndex++;
				renderContext.m_InstanceIDToGBufferInstanceDataIndexLookup[instance->GetInstanceID()] = ssboIndex;
			}
			m_GBufferInstanceDataSSBO->Write(ssboIndex * sizeof(InstanceDataGBuffer), sizeof(InstanceDataGBuffer), &iData);
			//// END upload instancedata for GBuffer pass

			//// BEGIN upload instancedata for CubemapShadowmapping pass
			unsigned int pointlightIndex = 0;

			cubemapInstanceDataIndices.clear();
			{
				for (const PointlightWorldObjectInstance* pl : pointlights)
				{
					for (unsigned int layerIndex = 0; layerIndex < 6; layerIndex++)
					{
						unsigned int instanceID = (instance->GetInstanceID() * pointlights.size() * 6);
						instanceID += ((pointlightIndex * 6) + layerIndex);

						InstanceDataCubemapShadowMapping iCubemapData;
						iCubemapData.m_M = instance->GetTransform();
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

			const MeshWorldObject* mwo = instance->GetMeshWorldObject();
			for (MeshWorldObject::Mesh* mesh : mwo->m_Meshes)
			{
				// Upload mesh data if not already happened
				VertexObject* vo;
				if (!voStorage[mesh->m_MeshID].has_value())
				{
					std::unique_ptr<DataBuffer<MeshVertexData>> bufferptr = std::make_unique<DataBuffer<MeshVertexData>>(&mesh->m_Vertices[0], mesh->m_Vertices.size(), DataBufferType::Static);
					vo = &voStorage[mesh->m_MeshID].emplace(&mesh->m_VertexIndicies[0], mesh->m_VertexIndicies.size());
					vo->LinkAndOwnBuffer(std::move(bufferptr));
					vo->LinkBuffer(*m_InstanceDataIndexBuffer, true);
				}
				else
				{
					vo = &voStorage[mesh->m_MeshID].value();
				}
				
				if (CollectGeometryPass_Private::IsInFrustum(camera.GetProjection() * iData.m_MV, *mesh))
				{
					RenderCommand& command = commandQueue.emplace_back();
					command.m_VO = vo;
					command.m_Material = &mesh->m_Material;
					command.m_SSBOInstanceID = ssboIndex;
				}

				for (const Visibility& vis : cubemapInstanceDataIndices)
				{
					if (!CollectGeometryPass_Private::IsInFrustum(vis.m_MVP, *mesh))
					{
						continue;
					}
					RenderCommand& command = cubemapCommandQueue.emplace_back();
					command.m_VO = vo;
					command.m_SSBOInstanceID = vis.m_InstanceID;
				}
			}
		}

		{
			SCOPED_TIMER("Sorting");
			std::sort(commandQueue.begin(), commandQueue.end(), [](const RenderCommand& a, const RenderCommand& b)
				{
					if (a.m_VO != b.m_VO)
					{
						return a.m_VO < b.m_VO;
					}
					return a.m_Material->GetShader() < b.m_Material->GetShader();
				}
			);

			std::sort(cubemapCommandQueue.begin(), cubemapCommandQueue.end(), [](const RenderCommand& a, const RenderCommand& b)
				{
					return a.m_VO < b.m_VO;
				}
			);
		}
	}
}


