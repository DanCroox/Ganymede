#include "ComputePass.h"

#include "Ganymede/Common/Helpers.h"
#include "Ganymede/ECS/Components/GCDynamicMobility.h"
#include "Ganymede/ECS/Components/GCMesh.h"
#include "Ganymede/ECS/Components/GCTransform.h"
#include "Ganymede/Graphics/DataBuffer.h"
#include "Ganymede/Graphics/OGLBindingHelper.h"
#include "Ganymede/Graphics/RenderContext.h"
#include "Ganymede/Graphics/Shader.h"
#include "Ganymede/Graphics/SSBO.h"
#include "Ganymede/Graphics/VertexDataTypes.h"
#include "Ganymede/Graphics/VertexObject.h"
#include "Ganymede/Player/FPSCamera.h"
#include "Ganymede/World/MeshWorldObject.h"
#include "Ganymede/World/World.h"
#include "GL/glew.h"
#include <chrono>
#include <thread>

namespace Ganymede
{
	ComputePass::~ComputePass()
	{
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	}

	bool ComputePass::Initialize(RenderContext& renderContext)
	{
		ssbo_EntityDataCounter = renderContext.CreateSSBO("EntityDataCounter", 18, sizeof(glm::uint32), false);
		ssbo_EntityVisiblityMasksCounter = renderContext.CreateSSBO("EntityVisiblityMasksCounter", 19, sizeof(glm::uint32), false);
		ssbo_AppendCounter = renderContext.CreateSSBO("AppendCounter", 20, sizeof(glm::uint32), false);
		ssbo_CommandCounter = renderContext.CreateSSBO("CommandCounter", 21, sizeof(glm::uint32), false);
		ssbo_NumRenderViews = renderContext.CreateSSBO("NumRenderViews", 22, sizeof(glm::uint32), false);

		ssbo_EntityData = renderContext.CreateSSBO("EntityData", 23, sizeof(EntityData) * 1000000, false);
		ssbo_RenderViews = renderContext.CreateSSBO("GPURenderViews", 24, sizeof(GPURenderView) * 1000000, false);
		ssbo_EntityVisiblityMasks = renderContext.CreateSSBO("VisibilityMasks", 25, sizeof(VisibilityMask) * 1000000, false);
		ssbo_AppendBuffer = renderContext.CreateSSBO("InstanceData", 26, sizeof(InstanceData) * 1000000, false);
		ssbo_IndirectDrawCmds = renderContext.CreateSSBO("IndirectDrawCommands", 27, sizeof(DrawElementsIndirectCommand) * 1000000, false);
		ssbo_RenderInfos = renderContext.CreateSSBO("RenderInfos", 28, sizeof(RenderMeshInstanceCommand) * 1000000, false);

		m_FindVisibleEntitiesCompute = renderContext.LoadShader("FindVisibleEntitiesComputeShader", "res/shaders/Compute/FindVisibleEntitiesComputeShader.shader");
		m_UnfoldVisibleEntitiesCompute = renderContext.LoadShader("UnfoldVisibleEntitiesCompute", "res/shaders/Compute/UnfoldVisibleEntitiesCompute.shader");
		m_GenerateIndirectDrawCommands = renderContext.LoadShader("GenerateIndirectDrawCommands", "res/shaders/Compute/GenerateIndirectDrawCommands.shader");

		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssbo_IndirectDrawCmds->m_RenderID);

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
			camView.m_FarClip = view.m_FarClip;
			camView.m_NearClip = view.m_NearClip;
			camView.m_Perspective = view.m_Perspective;
			camView.m_Transform = view.ToTransform();
			camView.m_FaceIndex = view.m_FaceIndex;
			camView.m_WorldPosition = glm::vec4(view.m_Position, 1);

			ssbo_RenderViews->Write(i * sizeof(GPURenderView), sizeof(GPURenderView), &camView);
			glm::uint32 ii = i + 1;
			ssbo_NumRenderViews->Write(0, sizeof(glm::uint32), &ii);
		}

		World& world = renderContext.GetWorld();

		auto entitiesToAdd = world.GetEntities(Include<GCMesh, GCTransform>{}, Exclude<GCUploaded>{});
		for (auto [entity, gcMesh, gcTransform] : entitiesToAdd.each())
		{
			for (MeshWorldObject::Mesh* mesh : gcMesh.m_Meshes)
			{
				EntityData data;
				data.m_Transform = gcTransform.GetMatrix();
				data.m_MeshID = mesh->m_MeshID;
				data.m_NumMeshIndices = mesh->m_VertexIndicies.size();
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
				ssbo_EntityDataCounter->Write(0, sizeof(glm::uint32), (void*)&m_NumEntities);
				ssbo_EntityData->Write(idx * sizeof(EntityData), sizeof(EntityData), &data);

				world.AddComponent<GCUploaded>(entity, idx);
			}
		}

		auto entitiesToUpdate = world.GetEntities(Include<GCTransform, GCUploaded, GCDynamicMobility>{});
		for (auto [entity, gcTransform, gcUploaded] : entitiesToUpdate.each())
		{
			ssbo_EntityData->Write(gcUploaded.m_SSBOIndex * sizeof(EntityData), sizeof(glm::mat4), (void*) &gcTransform.GetMatrix());
		}

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// Reset all compute counters
		const glm::uint32 zero = 0;
		ssbo_EntityVisiblityMasksCounter->Write(0, sizeof(glm::uint32), (void*)&zero);
		ssbo_AppendCounter->Write(0, sizeof(glm::uint32), (void*)&zero);
		ssbo_CommandCounter->Write(0, sizeof(glm::uint32), (void*)&zero);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		const unsigned int numComputeThreads = 256;

		OGLBindingHelper::BindShader(m_FindVisibleEntitiesCompute->GetRendererID());
		glDispatchCompute((m_NumEntities + numComputeThreads - 1) / numComputeThreads, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// Find visible entities for given RenderViews
		glm::uint numWorkgroupsUnfold;
		ssbo_EntityVisiblityMasksCounter->Read(0, sizeof(glm::uint), (void*)&numWorkgroupsUnfold);
		OGLBindingHelper::BindShader(m_UnfoldVisibleEntitiesCompute->GetRendererID());
		glDispatchCompute((numWorkgroupsUnfold + numComputeThreads -1) / numComputeThreads, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// Sort visible entities by RenderView and mesh. For simplicity we readback the instancedata buffer to sort. Later we do it directly in a compute pass
		glm::uint numAppends;
		ssbo_AppendCounter->Read(0, sizeof(glm::uint), (void*)&numAppends);

		std::vector<InstanceData> appends;
		appends.resize(numAppends);
		ssbo_AppendBuffer->Read(0, appends.size() * sizeof(InstanceData), appends.data());
		std::sort(appends.begin(), appends.end(), [](const InstanceData& a, const InstanceData& b)
			{
				if (a.m_ViewID < b.m_ViewID)
					return true;
				if (a.m_ViewID > b.m_ViewID)
					return false;
				return a.m_MeshID < b.m_MeshID;
			}
		);
		ssbo_AppendBuffer->Write(0, numAppends * sizeof(InstanceData), appends.data());

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// Generate draw commands based on sorted instances
		OGLBindingHelper::BindShader(m_GenerateIndirectDrawCommands->GetRendererID());
		glDispatchCompute((numAppends + numComputeThreads - 1) / numComputeThreads, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// Receive render command list
		glm::uint numCommands;
		ssbo_CommandCounter->Read(0, sizeof(glm::uint), (void*)&numCommands);
		renderContext.m_RenderInfo.resize(numCommands);
		ssbo_RenderInfos->Read(0, numCommands * sizeof(RenderMeshInstanceCommand), renderContext.m_RenderInfo.data());

		NUMBERED_NAMED_COUNTER("Commands", numCommands);

		renderContext.m_RenderInfoOffsets.resize(numCommands);

		std::sort(renderContext.m_RenderInfo.begin(), renderContext.m_RenderInfo.end(), [](const RenderMeshInstanceCommand& a, const RenderMeshInstanceCommand& b)
			{
				if (a.m_ViewID < b.m_ViewID)
					return true;
				if (a.m_ViewID > b.m_ViewID)
					return false;
				return a.m_MeshID < b.m_MeshID;
			}
		);

		unsigned int count = 0;
		for (unsigned int i = 0; i < renderContext.m_RenderInfo.size(); ++i)
		{
			const RenderMeshInstanceCommand& cmd = renderContext.m_RenderInfo[i];
			const unsigned int viewID = cmd.m_ViewID;

			const bool isNew = (i == 0) || renderContext.m_RenderInfo[i - 1].m_ViewID != viewID;
			const bool isLast = (i == renderContext.m_RenderInfo.size() - 1) || renderContext.m_RenderInfo[i + 1].m_ViewID != viewID;

			if (isNew)
			{
				renderContext.m_RenderInfoOffsets[viewID].m_StartIndex = i;
			}

			if (isLast)
			{
				++count;
				renderContext.m_RenderInfoOffsets[viewID].m_LastIndex = count;
				count = 0;
				continue;
			}

			++count;
		}

		std::vector< DrawElementsIndirectCommand> cmds;
		cmds.resize(numCommands);
		ssbo_IndirectDrawCmds->Read(0, numCommands * sizeof(DrawElementsIndirectCommand), cmds.data());

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
}