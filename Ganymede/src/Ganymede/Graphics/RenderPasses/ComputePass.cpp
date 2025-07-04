#include "ComputePass.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/ECS/Components/GCDynamicMobility.h"
#include "Ganymede/ECS/Components/GCGPUEntityData.h"
#include "Ganymede/ECS/Components/GCMesh.h"
#include "Ganymede/ECS/Components/GCRigidBody.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/Graphics/DataBuffer.h"
#include "Ganymede/Graphics/GPUCommands.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/Shader.h"
#include "Ganymede/Graphics/ShaderBinary.h"
#include "Ganymede/Graphics/SSBO.h"
#include "Ganymede/Graphics/VertexDataTypes.h"
#include "Ganymede/Graphics/VertexObject.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/World.h"
#include <chrono>
#include <thread>

namespace Ganymede
{
	bool ComputePass::Initialize(RenderContext& renderContext)
	{
		ssbo_ComputePassCounters = renderContext.CreateSSBO("ComputePassCounters", 20, sizeof(ComputePassCounters), false);

		ssbo_EntityData = renderContext.CreateSSBO("EntityData", 21, sizeof(EntityData) * 1000000, false);
		ssbo_RenderViews = renderContext.CreateSSBO("GPURenderViews", 22, sizeof(GPURenderView) * 1000000, false);
		ssbo_AppendBuffer = renderContext.CreateSSBO("InstanceData", 23, sizeof(InstanceData) * 1000000, false);
		ssbo_IndirectDrawCmds = renderContext.CreateSSBO("IndirectDrawCommands", 24, sizeof(DrawElementsIndirectCommand) * 1000000, false);
		ssbo_RenderInfos = renderContext.CreateSSBO("RenderInfos", 25, sizeof(RenderMeshInstanceCommand) * 1000000, false);
		ssbo_VisibleEntities = renderContext.CreateSSBO("EntityIDGPUInstanceDataMapping", 26, sizeof(VisibleEntity) * 1000000, false);

		m_FindVisibleEntitiesCompute = renderContext.LoadShader("FindVisibleEntitiesComputeShader", { "res/shaders/Compute/FindVisibleEntitiesComputeShader.shader" });
		m_GenerateIndirectDrawCommands = renderContext.LoadShader("GenerateIndirectDrawCommands", { "res/shaders/Compute/GenerateIndirectDrawCommands.shader" });

		return true;
	}

	void ComputePass::Execute(RenderContext& renderContext)
	{
		SCOPED_TIMER("Compute Pass");
		for (unsigned int i = 0; i < renderContext.GetNumRenderViews(); ++i)
		{
			const RenderView& view = renderContext.GetRenderView(i);

			GPURenderView camView;
			camView.m_ViewID = view.m_ViewID;
			camView.m_FarClip = view.GetFarClip();
			camView.m_NearClip = view.GetNearClip();
			camView.m_Perspective = view.GetProjectionMatrix();
			camView.m_Transform = view.GetViewMatrix();
			camView.m_FaceIndex = view.m_FaceIndex;
			camView.m_RenderViewGroup = view.m_RenderViewGroup;
			camView.m_WorldPosition = glm::vec4(view.GetPosition(), 1);

			ssbo_RenderViews->Write(i * sizeof(GPURenderView), sizeof(GPURenderView), &camView);
		}
		const unsigned int numViews = renderContext.GetNumRenderViews();
		ssbo_ComputePassCounters->Write(offsetof(ComputePassCounters, m_NumRenderViews), sizeof(glm::uint32), (void*) &numViews);

		World& world = renderContext.GetWorld();

		auto entitiesToAdd = world.GetEntities(Include<GCMesh, GCTransform>{}, Exclude<GCGPUEntityData>{});
		for (auto [entity, gcMesh, gcTransform] : entitiesToAdd.each())
		{
			for (MeshWorldObject::Mesh* mesh : gcMesh.m_Meshes)
			{
				EntityData data;
				data.m_Transform = gcTransform.GetMatrix();
				data.m_MeshID = mesh->m_MeshID;
				data.m_NumMeshIndices = mesh->m_VertexIndicies.size();
				data.m_AnimationDataOffset = 0;
				data.m_EntityID = static_cast<uint32_t>(entity);
				data.m_AABB[0] = glm::vec4(mesh->m_BoundingBoxVertices[0].m_Position, 1);
				data.m_AABB[1] = glm::vec4(mesh->m_BoundingBoxVertices[1].m_Position, 1);
				data.m_AABB[2] = glm::vec4(mesh->m_BoundingBoxVertices[2].m_Position, 1);
				data.m_AABB[3] = glm::vec4(mesh->m_BoundingBoxVertices[3].m_Position, 1);
				data.m_AABB[4] = glm::vec4(mesh->m_BoundingBoxVertices[4].m_Position, 1);
				data.m_AABB[5] = glm::vec4(mesh->m_BoundingBoxVertices[5].m_Position, 1);
				data.m_AABB[6] = glm::vec4(mesh->m_BoundingBoxVertices[6].m_Position, 1);
				data.m_AABB[7] = glm::vec4(mesh->m_BoundingBoxVertices[7].m_Position, 1);

				if (renderContext.m_MeshIDMapping.size() <= mesh->m_MeshID)
				{
					renderContext.m_MeshIDMapping.resize(mesh->m_MeshID + 1);
				}
				renderContext.m_MeshIDMapping[mesh->m_MeshID] = mesh;

				const unsigned int idx = m_NumEntities++;

				ssbo_ComputePassCounters->Write(offsetof(ComputePassCounters, m_NumEntities), sizeof(glm::uint32), (void*)&m_NumEntities);
				ssbo_EntityData->Write(idx * sizeof(EntityData), sizeof(EntityData), &data);

				world.AddComponent<GCGPUEntityData>(entity, idx);
			}
		}

		ssbo_ComputePassCounters->Barrier();
		ssbo_EntityData->Barrier();

		// Reset all compute counters
		const glm::uint32 zero = 0;
		ssbo_ComputePassCounters->Write(offsetof(ComputePassCounters, m_NumAppends), sizeof(glm::uint32), (void*)&zero);
		ssbo_ComputePassCounters->Write(offsetof(ComputePassCounters, m_NumIndirectCommands), sizeof(glm::uint32), (void*)&zero);
		ssbo_ComputePassCounters->Write(offsetof(ComputePassCounters, m_NumVisibleEntities), sizeof(glm::uint32), (void*)&zero);

		ssbo_ComputePassCounters->Barrier();

		const unsigned int numComputeThreads = 256;
		// Find visible entities for all registered views
		GPUCommands::Compute::Dispatch(*m_FindVisibleEntitiesCompute, (m_NumEntities + numComputeThreads - 1) / numComputeThreads, 1, 1);

		ssbo_ComputePassCounters->Barrier();
		ssbo_VisibleEntities->Barrier();
		ssbo_AppendBuffer->Barrier();

		glm::uint numVisibleEntities;
		ssbo_ComputePassCounters->Read(offsetof(ComputePassCounters, m_NumVisibleEntities), sizeof(glm::uint32), (void*)&numVisibleEntities);
		std::vector<VisibleEntity>& visibleEntities = renderContext.GetVisibleEntities();
		visibleEntities.resize(numVisibleEntities);
		ssbo_VisibleEntities->Read(0, numVisibleEntities * sizeof(VisibleEntity), visibleEntities.data());

		// Sort visible entities by RenderView and mesh. For simplicity we readback the instancedata buffer to sort. Later we do it directly in a compute pass
		glm::uint numAppends;
		ssbo_ComputePassCounters->Read(offsetof(ComputePassCounters, m_NumAppends), sizeof(glm::uint32), (void*)&numAppends);
		std::vector<InstanceData> appends;
		appends.resize(numAppends);
		ssbo_AppendBuffer->Read(0, appends.size() * sizeof(InstanceData), appends.data());
		std::sort(appends.begin(), appends.end(), [](const InstanceData& a, const InstanceData& b)
			{
				if (a.m_RenderViewGroup < b.m_RenderViewGroup)
					return true;
				if (a.m_RenderViewGroup > b.m_RenderViewGroup)
					return false;
				return a.m_MeshID < b.m_MeshID;
			}
		);
		ssbo_AppendBuffer->Write(0, numAppends * sizeof(InstanceData), appends.data());

		ssbo_AppendBuffer->Barrier();

		// Generate draw commands based on sorted instances
		GPUCommands::Compute::Dispatch(*m_GenerateIndirectDrawCommands, (numAppends + numComputeThreads - 1) / numComputeThreads, 1, 1);

		ssbo_AppendBuffer->Barrier();
		ssbo_ComputePassCounters->Barrier();
		ssbo_RenderInfos->Barrier();
		ssbo_IndirectDrawCmds->Barrier();

		// Receive render command list
		glm::uint numCommands;
		ssbo_ComputePassCounters->Read(offsetof(ComputePassCounters, m_NumIndirectCommands), sizeof(glm::uint32), (void*)&numCommands);
		renderContext.m_RenderInfo.resize(numCommands);
		ssbo_RenderInfos->Read(0, numCommands * sizeof(RenderMeshInstanceCommand), renderContext.m_RenderInfo.data());

		NUMBERED_NAMED_COUNTER("Indirect Commands Total", numCommands);
		NUMBERED_NAMED_COUNTER("Rendered Instances Total", numAppends);

		renderContext.m_RenderInfoOffsets.resize(numCommands);

		std::sort(renderContext.m_RenderInfo.begin(), renderContext.m_RenderInfo.end(), [](const RenderMeshInstanceCommand& a, const RenderMeshInstanceCommand& b)
			{
				if (a.m_RenderViewGroup < b.m_RenderViewGroup)
					return true;
				if (a.m_RenderViewGroup > b.m_RenderViewGroup)
					return false;
				return a.m_MeshID < b.m_MeshID;
			}
		);

		// TODO Proper offset reset
		renderContext.m_RenderInfoOffsets[0] = {};
		renderContext.m_RenderInfoOffsets[1] = {};

		unsigned int count = 0;
		for (unsigned int i = 0; i < renderContext.m_RenderInfo.size(); ++i)
		{
			const RenderMeshInstanceCommand& cmd = renderContext.m_RenderInfo[i];
			const unsigned int groupID = cmd.m_RenderViewGroup;

			const bool isNew = (i == 0) || renderContext.m_RenderInfo[i - 1].m_RenderViewGroup != groupID;
			const bool isLast = (i == renderContext.m_RenderInfo.size() - 1) || renderContext.m_RenderInfo[i + 1].m_RenderViewGroup != groupID;

			if (isNew)
			{
				renderContext.m_RenderInfoOffsets[groupID].m_StartIndex = i;
			}

			if (isLast)
			{
				++count;
				renderContext.m_RenderInfoOffsets[groupID].m_LastIndex = count;
				count = 0;
				continue;
			}

			++count;
		}
	}
}