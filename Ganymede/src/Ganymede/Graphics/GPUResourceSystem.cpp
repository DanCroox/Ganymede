#include "GPUResourceSystem.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/ECS/Components/GCDynamicMobility.h"
#include "Ganymede/ECS/Components/GCGPUMeshData.h"
#include "Ganymede/ECS/Components/GCLoadRenderObject.h"
#include "Ganymede/ECS/Components/GCMesh.h"
#include "Ganymede/ECS/Components/GCRenderObject.h"
#include "Ganymede/ECS/Components/GCSkeletal.h"
#include "Ganymede/ECS/Components/GCStaticMobility.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/ECS/Components/GCUnloadRenderObject.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/World.h"
#include "RenderContext.h"
#include "SSBO.h"
#include "VertexObject.h"

#include "glm/glm.hpp"

namespace Ganymede
{
	struct InstanceDataGBuffer
	{
		glm::mat4 m_M;
		glm::uvec4 m_AnimationDataOffset;
	};

	GPUResourceSystem::GPUResourceSystem(World& world) :
		m_InstanceDataIndexBuffer(nullptr, 600, DataBufferType::Dynamic),
		m_GBufferInstanceDataSSBO(std::make_unique<SSBO>(3, 10000 * sizeof(InstanceDataGBuffer), true)),
		m_AnimationDataSSBO(std::make_unique<SSBO>(2, 100000 * sizeof(glm::mat4), true)),
		m_World(world)
	{
		m_VertexObjects.resize(10000);
	}

	void GPUResourceSystem::UpdateGPUResources()
	{
		RemoveMeshDataFromGPU();
		LoadPendingMeshDataToGPU();

		UpdateInstanceDataOnGPU();
		UpdateAnimationDataOnGPU();
	}

	std::shared_ptr<VertexObject> GPUResourceSystem::GetVO(MeshWorldObject::Mesh& mesh)
	{
		if (auto ptr = m_VertexObjects[mesh.m_MeshID].lock())
		{
			return ptr;
		}
		else
		{
			std::shared_ptr<VertexObject> sPtr = std::make_shared<VertexObject>(&mesh.m_VertexIndicies[0], mesh.m_VertexIndicies.size());
			std::unique_ptr<DataBuffer<MeshVertexData>> bufferptr = std::make_unique<DataBuffer<MeshVertexData>>(&mesh.m_Vertices[0], mesh.m_Vertices.size(), DataBufferType::Static);
			sPtr->LinkAndOwnBuffer(std::move(bufferptr));
			sPtr->LinkBuffer(m_InstanceDataIndexBuffer, true);
			m_VertexObjects[mesh.m_MeshID] = sPtr;
			return sPtr;
		}
	}

	void GPUResourceSystem::RemoveMeshDataFromGPU()
	{
		auto entitesToUnload = m_World.GetEntities(Include<GCRenderObject, GCUnloadRenderObject>{});
		entitesToUnload.each([&](Entity entity, GCRenderObject& gcRenderObject)
			{
				for (Entity renderEntity : gcRenderObject.m_RenderEntities)
				{
					// Every mesh we would like to render exists as an own entity with only a GCGPUMeshData component.
					// Destroying the entity destroys the component which destroys the inherent VAO on the GPU. See "LoadPendingMeshDataToGPU" for more details.
					m_World.DestroyEntity(renderEntity);
				}

				m_GBufferInstanceDataFreeList.Free(gcRenderObject.m_InstanceDataIndex);
				m_World.RemoveComponents<GCRenderObject>(entity);
				m_World.RemoveComponents<GCUnloadRenderObject>(entity);
			});
	}

	void GPUResourceSystem::LoadPendingMeshDataToGPU()
	{
		const auto entitesToLoad = m_World.GetEntities(Include<GCMesh, GCTransform, GCLoadRenderObject>{});
		entitesToLoad.each([&](Entity entity, GCMesh& gcMesh, GCTransform& gcTransform)
			{
				GCRenderObject& gcRenderObject = m_World.AddComponent<GCRenderObject>(entity);
				m_World.RemoveComponents<GCLoadRenderObject>(entity);

				const std::int32_t ssboIndex = m_GBufferInstanceDataFreeList.Append();
				gcRenderObject.m_InstanceDataIndex = ssboIndex;

				InstanceDataGBuffer iData;
				iData.m_M = gcTransform.GetMatrix();

				m_GBufferInstanceDataSSBO->Write(ssboIndex * sizeof(InstanceDataGBuffer), sizeof(InstanceDataGBuffer::m_M), &iData);

				for (MeshWorldObject::Mesh* mesh : gcMesh.m_Meshes)
				{
					// We create separate entities for each mesh. Since world-entites can contain multiples meshes but ECS can only hold one component of same type
					// we have to enlist all meshes within the GCMesh component. That fact does not allow us to sort all meshes by type in order to gain instance rendering.
					// Therefore we create a new entity per mesh we would like to render and decouple it from the relation to its world-entity. Later we can easily sort and group
					// the GCGPUMeshData components based on related mesh and submit many instance draws with one call.
					Entity renderEntity = m_World.CreateEntity();
					GCGPUMeshData& gcGpuMeshData = m_World.AddComponent<GCGPUMeshData>(renderEntity);
					gcRenderObject.m_RenderEntities.push_back(renderEntity);
					gcGpuMeshData.m_VO = std::move(GetVO(*mesh));
					gcGpuMeshData.m_Material = &mesh->m_Material;
					gcGpuMeshData.m_InstanceDataIndex = ssboIndex;
				}
			});
	}

	void GPUResourceSystem::UpdateInstanceDataOnGPU()
	{
		const auto entitiesToUpdate = m_World.GetEntities(Include<GCTransform, GCRenderObject, GCDynamicMobility>{});
		for (auto [entiy, gcTransform, gcRenderObject] : entitiesToUpdate.each())
		{
			InstanceDataGBuffer iData;
			iData.m_M = gcTransform.GetMatrix();

			const std::int32_t ssboIndex = gcRenderObject.m_InstanceDataIndex;
			m_GBufferInstanceDataSSBO->Write(ssboIndex * sizeof(InstanceDataGBuffer), sizeof(InstanceDataGBuffer::m_M), &iData);
		}
	}

	void GPUResourceSystem::UpdateAnimationDataOnGPU()
	{
		unsigned int animationDataOffset = 0;
		const auto skeltalEntitiesToUpdate = m_World.GetEntities(Include<GCTransform, GCRenderObject, GCSkeletal>{});
		for (auto [entiy, gcTransform, gcRenderObject, gcSkeletal] : skeltalEntitiesToUpdate.each())
		{
			const std::int32_t ssboIndex = gcRenderObject.m_InstanceDataIndex;
			m_GBufferInstanceDataSSBO->Write((ssboIndex * sizeof(InstanceDataGBuffer)) + offsetof(InstanceDataGBuffer, m_AnimationDataOffset), sizeof(InstanceDataGBuffer::m_AnimationDataOffset), (void*)&animationDataOffset);

			const std::vector<glm::mat4>& animationFrame = gcSkeletal.m_AnimationBoneData;
			m_AnimationDataSSBO->Write(animationDataOffset * sizeof(glm::mat4), animationFrame.size() * sizeof(glm::mat4), (void*)&animationFrame[0]);
			animationDataOffset += animationFrame.size() * sizeof(glm::mat4);
		}
	}
}